#include "schema.h"
#include "params.h"
#include "types.h"


namespace binding {


zvec::Result<zvec::FieldSchema::Ptr> ParseFieldSchema(
    const Napi::Value &value) {
  if (!value.IsObject()) {
    return tl::make_unexpected(
        zvec::Status::InvalidArgument("Expected an object for FieldSchema"));
  }

  auto obj = value.As<Napi::Object>();

  if (!obj.Has("name")) {
    return tl::make_unexpected(zvec::Status::InvalidArgument(
        "Missing required argument 'name' in FieldSchema"));
  }
  if (!obj.Get("name").IsString()) {
    return tl::make_unexpected(zvec::Status::InvalidArgument(
        "Expected argument 'name' to be a string"));
  }
  std::string name = obj.Get("name").As<Napi::String>().Utf8Value();

  if (!obj.Has("dataType")) {
    return tl::make_unexpected(zvec::Status::InvalidArgument(
        "Missing required argument 'dataType' in FieldSchema"));
  }
  auto parsed_type = ParseDataType(obj.Get("dataType"));
  if (!parsed_type) {
    return tl::make_unexpected(parsed_type.error());
  }

  bool nullable = false;
  if (obj.Has("nullable")) {
    if (obj.Get("nullable").IsBoolean()) {
      nullable = obj.Get("nullable").As<Napi::Boolean>();
    } else {
      return tl::make_unexpected(zvec::Status::InvalidArgument(
          "Expected argument 'nullable' to be a boolean"));
    }
  }

  zvec::IndexParams::Ptr index_params{nullptr};
  if (obj.Has("indexParams")) {
    auto parsed_index_params = ParseIndexParams(obj.Get("indexParams"));
    if (parsed_index_params) {
      index_params = parsed_index_params.value();
    } else {
      return tl::make_unexpected(parsed_index_params.error());
    }
  }

  auto field_schema = std::make_shared<zvec::FieldSchema>(
      name, parsed_type.value(), nullable, index_params);

  if (field_schema->is_vector_field()) {
    return tl::make_unexpected(zvec::Status::InvalidArgument(
        "FieldSchema does not support vector fields"));
  }
  if (auto s = field_schema->validate(); s.ok()) {
    return field_schema;
  } else {
    return tl::make_unexpected(s);
  }
}


Napi::Object CreateFieldSchema(Napi::Env env, zvec::FieldSchema *field) {
  auto obj = Napi::Object::New(env);
  obj.Set("name", field->name());
  obj.Set("dataType", static_cast<uint32_t>(field->data_type()));
  obj.Set("nullable", field->nullable());
  if (field->index_params()) {
    obj.Set("indexParams", CreateIndexParams(env, field->index_params()));
  }
  return obj;
}


zvec::Result<zvec::FieldSchema::Ptr> ParseVectorSchema(
    const Napi::Value &value) {
  if (!value.IsObject()) {
    return tl::make_unexpected(
        zvec::Status::InvalidArgument("Expected an object for VectorSchema"));
  }

  auto obj = value.As<Napi::Object>();

  if (!obj.Has("name")) {
    return tl::make_unexpected(zvec::Status::InvalidArgument(
        "Missing required argument 'name' in VectorSchema"));
  }
  if (!obj.Get("name").IsString()) {
    return tl::make_unexpected(zvec::Status::InvalidArgument(
        "Expected argument 'name' to be a string"));
  }
  std::string name = obj.Get("name").As<Napi::String>().Utf8Value();

  if (!obj.Has("dataType")) {
    return tl::make_unexpected(zvec::Status::InvalidArgument(
        "Missing required argument 'dataType' in VectorSchema"));
  }
  auto parsed_type = ParseDataType(obj.Get("dataType"));
  if (!parsed_type) {
    return tl::make_unexpected(parsed_type.error());
  }

  int dimension = 0;
  if (obj.Has("dimension")) {
    if (obj.Get("dimension").IsNumber()) {
      dimension = obj.Get("dimension").As<Napi::Number>();
    } else {
      return tl::make_unexpected(zvec::Status::InvalidArgument(
          "Expected argument 'dimension' to be a number"));
    }
  }

  zvec::IndexParams::Ptr index_params = std::make_shared<zvec::FlatIndexParams>(
      zvec::MetricType::IP, zvec::QuantizeType::UNDEFINED);
  if (obj.Has("indexParams")) {
    auto parsed_index_params = ParseIndexParams(obj.Get("indexParams"));
    if (parsed_index_params) {
      index_params = parsed_index_params.value();
    } else {
      return tl::make_unexpected(parsed_index_params.error());
    }
  }

  auto field_schema = std::make_shared<zvec::FieldSchema>(
      name, parsed_type.value(), dimension, false, index_params);

  if (!field_schema->is_vector_field()) {
    return tl::make_unexpected(zvec::Status::InvalidArgument(
        "VectorSchema does not support scalar fields"));
  }
  if (auto s = field_schema->validate(); s.ok()) {
    return field_schema;
  } else {
    return tl::make_unexpected(s);
  }
}


Napi::Object CreateVectorSchema(Napi::Env env, zvec::FieldSchema *field) {
  auto obj = Napi::Object::New(env);
  obj.Set("name", field->name());
  obj.Set("dataType", static_cast<uint32_t>(field->data_type()));
  obj.Set("dimension", field->dimension());
  obj.Set("indexParams", CreateIndexParams(env, field->index_params()));
  return obj;
}


Napi::Object CollectionSchema::Init(Napi::Env env, Napi::Object exports,
                                    AddonConstructors *ctors) {
  Napi::Function func = DefineClass(
      env, "CollectionSchema",
      {
          InstanceAccessor("name", &CollectionSchema::Name, nullptr),
          InstanceMethod("field", &CollectionSchema::Field),
          InstanceMethod("vector", &CollectionSchema::Vector),
          InstanceMethod("fields", &CollectionSchema::Fields),
          InstanceMethod("vectors", &CollectionSchema::Vectors),
          InstanceMethod("toString", &CollectionSchema::ToString),
      });

  ctors->collectionSchemaConstructor = Napi::Persistent(func);
  exports.Set("CollectionSchema", func);
  return exports;
}


CollectionSchema::CollectionSchema(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<CollectionSchema>(info) {
  Napi::Env env = info.Env();
  auto length = info.Length();

  if (info.Length() != 1) {
    ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                          "CollectionSchema constructor expects 1 argument "
                          "(schema definition object): { name: string, fields: "
                          "[...], vectors: [...] }"));
    return;
  }

  if (info[0].IsExternal() && info[0].As<Napi::External<void>>().Data() ==
                                  kInternalConstructionMarker) {
    return;
  }

  if (!info[0].IsObject()) {
    ThrowIfNotOk(
        env, zvec::Status::InvalidArgument(
                 "CollectionSchema constructor argument must be an object"));
    return;
  }

  auto obj = info[0].As<Napi::Object>();

  if (!obj.Has("name")) {
    ThrowIfNotOk(env,
                 zvec::Status::InvalidArgument(
                     "Missing required argument 'name' in CollectionSchema"));
    return;
  }
  Napi::Value nameValue = obj.Get("name");
  if (!nameValue.IsString()) {
    ThrowIfNotOk(
        env,
        zvec::Status::InvalidArgument(
            "CollectionSchema constructor: argument 'name' must be a string"));
    return;
  }
  std::string name = nameValue.As<Napi::String>().Utf8Value();

  std::vector<zvec::FieldSchema::Ptr> fields;

  if (!obj.Has("vectors")) {
    ThrowIfNotOk(
        env, zvec::Status::InvalidArgument(
                 "Missing required argument 'vectors' in CollectionSchema"));
    return;
  }
  Napi::Value vectorsValue = obj.Get("vectors");
  if (vectorsValue.IsArray()) {
    Napi::Array vectorsArray = vectorsValue.As<Napi::Array>();
    for (uint32_t i = 0; i < vectorsArray.Length(); i++) {
      auto parsed_vector_schema = ParseVectorSchema(vectorsArray.Get(i));
      if (!parsed_vector_schema) {
        ThrowIfNotOk(env, parsed_vector_schema.error());
        return;
      }
      fields.emplace_back(parsed_vector_schema.value());
    }
  } else if (vectorsValue.IsObject()) {
    auto parsed_vector_schema = ParseVectorSchema(vectorsValue);
    if (!parsed_vector_schema) {
      ThrowIfNotOk(env, parsed_vector_schema.error());
      return;
    }
    fields.emplace_back(parsed_vector_schema.value());
  } else {
    ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                          "CollectionSchema constructor: argument 'vectors' "
                          "must be an object or an array of objects"));
    return;
  }

  if (obj.Has("fields")) {
    Napi::Value fieldsValue = obj.Get("fields");
    if (fieldsValue.IsArray()) {
      Napi::Array fieldsArray = fieldsValue.As<Napi::Array>();
      for (uint32_t i = 0; i < fieldsArray.Length(); i++) {
        auto parsed_field_schema = ParseFieldSchema(fieldsArray.Get(i));
        if (!parsed_field_schema) {
          ThrowIfNotOk(env, parsed_field_schema.error());
          return;
        }
        fields.emplace_back(parsed_field_schema.value());
      }
    } else if (fieldsValue.IsObject()) {
      auto parsed_field_schema = ParseFieldSchema(fieldsValue);
      if (!parsed_field_schema) {
        ThrowIfNotOk(env, parsed_field_schema.error());
        return;
      }
      fields.emplace_back(parsed_field_schema.value());
    } else {
      ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                            "CollectionSchema constructor: argument 'fields' "
                            "must be an object or an array of objects"));
      return;
    }
  }

  collection_schema_ = std::make_shared<zvec::CollectionSchema>(name, fields);

  auto s = collection_schema_->validate();
  ThrowIfNotOk(env, s);
}


zvec::CollectionSchema::Ptr CollectionSchema::get_wrapped() {
  return collection_schema_;
}


void CollectionSchema::set_wrapped(zvec::CollectionSchema::Ptr schema) {
  collection_schema_ = schema;
}


Napi::Value CollectionSchema::Name(const Napi::CallbackInfo &info) {
  return Napi::String::New(info.Env(), collection_schema_->name());
}


Napi::Value CollectionSchema::Field(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (info.Length() != 1 || !info[0].IsString()) {
    ThrowIfNotOk(
        env,
        zvec::Status::InvalidArgument(
            "CollectionSchema.field() expects 1 argument: (name: string)"));
    return env.Undefined();
  }

  std::string field_name = info[0].As<Napi::String>().Utf8Value();
  auto field = collection_schema_->get_field(field_name);
  if (!field) {
    ThrowIfNotOk(env,
                 zvec::Status::NotFound("Field[", field_name,
                                        "] does not exist in the collection"));
    return env.Undefined();
  }
  if (field->is_vector_field()) {
    ThrowIfNotOk(env, zvec::Status::InvalidArgument("Field[", field_name,
                                                    "] is not a scalar field"));
    return env.Undefined();
  }
  return CreateFieldSchema(env, field);
}


Napi::Value CollectionSchema::Vector(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (info.Length() != 1 || !info[0].IsString()) {
    ThrowIfNotOk(
        env,
        zvec::Status::InvalidArgument(
            "CollectionSchema.vector() expects 1 argument: (name: string)"));
    return env.Undefined();
  }

  std::string vector_name = info[0].As<Napi::String>().Utf8Value();
  auto vector = collection_schema_->get_field(vector_name);
  if (!vector) {
    ThrowIfNotOk(env,
                 zvec::Status::NotFound("Vector[", vector_name,
                                        "] does not exist in the collection"));
    return env.Undefined();
  }
  if (!vector->is_vector_field()) {
    ThrowIfNotOk(env, zvec::Status::InvalidArgument("Field[", vector_name,
                                                    "] is not a vector field"));
    return env.Undefined();
  }
  return CreateVectorSchema(env, vector);
}


Napi::Value CollectionSchema::Fields(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  auto fields = collection_schema_->forward_fields();
  Napi::Array fieldsArray = Napi::Array::New(env, fields.size());
  for (size_t i = 0; i < fields.size(); ++i) {
    fieldsArray.Set(i, CreateFieldSchema(env, fields[i].get()));
  }
  return fieldsArray;
}


Napi::Value CollectionSchema::Vectors(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  auto vectors = collection_schema_->vector_fields();
  Napi::Array vectorsArray = Napi::Array::New(env, vectors.size());
  for (size_t i = 0; i < vectors.size(); ++i) {
    vectorsArray.Set(i, CreateVectorSchema(env, vectors[i].get()));
  }
  return vectorsArray;
}


Napi::Value CollectionSchema::ToString(const Napi::CallbackInfo &info) {
  if (collection_schema_) {
    return Napi::String::New(info.Env(),
                             collection_schema_->to_string_formatted());
  } else {
    return Napi::String::New(info.Env(), "CollectionSchema{uninitialized}");
  }
}


}  // namespace binding