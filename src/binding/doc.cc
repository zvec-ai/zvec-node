#include "doc.h"


namespace binding {


zvec::Result<zvec::Doc> ParseDoc(const Napi::Value &value,
                                 zvec::CollectionSchema::Ptr schema) {
  if (!value.IsObject()) {
    return tl::make_unexpected(
        zvec::Status::InvalidArgument("Expected an object for Doc"));
  }

  auto doc = zvec::Doc{};
  auto obj = value.As<Napi::Object>();

  if (!obj.Has("id")) {
    return tl::make_unexpected(
        zvec::Status::InvalidArgument("Missing required argument 'id' in Doc"));
  }
  if (!obj.Get("id").IsString()) {
    return tl::make_unexpected(
        zvec::Status::InvalidArgument("Expected argument 'id' to be a string"));
  }
  std::string id = obj.Get("id").As<Napi::String>().Utf8Value();
  doc.set_pk(id);

  if (obj.Has("vectors")) {
    if (auto s = ParseVectors(obj.Get("vectors"), schema, &doc); !s.ok()) {
      return tl::make_unexpected(s);
    }
  }

  if (obj.Has("fields")) {
    if (auto s = ParseFields(obj.Get("fields"), schema, &doc); !s.ok()) {
      return tl::make_unexpected(s);
    }
  }

  return doc;
}


zvec::Status ParseVectors(const Napi::Value &value,
                          zvec::CollectionSchema::Ptr &schema, zvec::Doc *doc) {
  if (!value.IsObject()) {
    return zvec::Status::InvalidArgument(
        "Expected argument 'vectors' to be an object");
  }
  auto vectors = value.As<Napi::Object>();
  auto vectorNames = vectors.GetPropertyNames();
  for (uint32_t i = 0; i < vectorNames.Length(); ++i) {
    auto vectorName = vectorNames.Get(i).As<Napi::String>().Utf8Value();
    auto vector_schema = schema->get_vector_field(vectorName);
    if (!vector_schema) {
      return zvec::Status::InvalidArgument("Vector[", vectorName,
                                           "] not found in collection schema");
    }
    auto vectorValue = vectors.Get(vectorName);
    auto s = ParseVector(vectorValue, vectorName, vector_schema, doc);
    if (!s.ok()) {
      return s;
    }
  }
  return zvec::Status::OK();
}


zvec::Status ParseVector(const Napi::Value &value,
                         const std::string &vectorName,
                         zvec::FieldSchema *schema, zvec::Doc *doc) {
  if (schema->is_dense_vector()) {
    if (value.IsTypedArray()) {
      return ParseVectorTypedArray(value.As<Napi::TypedArray>(), vectorName,
                                   schema, doc);
    } else if (value.IsArray()) {
      return ParseVectorArray(value.As<Napi::Array>(), vectorName, schema, doc);
    } else {
      return zvec::Status::InvalidArgument("Expected dense vector[", vectorName,
                                           "] to be an array of numbers");
    }
  } else if (schema->is_sparse_vector()) {
    if (value.IsObject()) {
      return ParseVectorObject(value.As<Napi::Object>(), vectorName, schema,
                               doc);
    } else {
      return zvec::Status::InvalidArgument("Expected sparse vector[",
                                           vectorName, "] to be an object");
    }
  } else {
    return zvec::Status::InternalError("Field[", vectorName,
                                       "] is not a vector field");
  }
}


zvec::Status ParseVectorTypedArray(const Napi::TypedArray &array,
                                   const std::string &vectorName,
                                   zvec::FieldSchema *schema, zvec::Doc *doc) {
  auto ta_type = array.TypedArrayType();
  size_t ta_length = array.ElementLength();
  switch (schema->data_type()) {
    case zvec::DataType::VECTOR_FP16: {
      if (ta_type != napi_float32_array) {
        return zvec::Status::InvalidArgument(
            "Expected dense vector[", vectorName, "] to be a Float32Array");
      }
      Napi::Float32Array float32Array = array.As<Napi::Float32Array>();
      std::vector<zvec::ailego::Float16> vector;
      vector.reserve(ta_length);
      for (size_t i = 0; i < ta_length; ++i) {
        vector.emplace_back(float32Array[i]);
      }
      doc->set(vectorName, std::move(vector));
      break;
    }
    case zvec::DataType::VECTOR_FP32: {
      if (ta_type != napi_float32_array) {
        return zvec::Status::InvalidArgument(
            "Expected dense vector[", vectorName, "] to be a Float32Array");
      }
      Napi::Float32Array float32Array = array.As<Napi::Float32Array>();
      std::vector<float> vector;
      const float *src_begin = static_cast<const float *>(float32Array.Data());
      const float *src_end = src_begin + ta_length;
      vector.assign(src_begin, src_end);
      doc->set(vectorName, std::move(vector));
      break;
    }
    case zvec::DataType::VECTOR_INT8: {
      if (ta_type != napi_int8_array) {
        return zvec::Status::InvalidArgument("Expected dense vector[",
                                             vectorName, "] to be a Int8Array");
      }
      Napi::Int8Array int8Array = array.As<Napi::Int8Array>();
      std::vector<int8_t> vector;
      const int8_t *src_begin = static_cast<const int8_t *>(int8Array.Data());
      const int8_t *src_end = src_begin + ta_length;
      vector.assign(src_begin, src_end);
      doc->set(vectorName, std::move(vector));
      break;
    }
    default: {
      return zvec::Status::InternalError(
          "Unknown data type: ", static_cast<uint32_t>(schema->data_type()));
    }
  }
  return zvec::Status::OK();
}


zvec::Status ParseVectorArray(const Napi::Array &array,
                              const std::string &vectorName,
                              zvec::FieldSchema *schema, zvec::Doc *doc) {
  switch (schema->data_type()) {
    case zvec::DataType::VECTOR_FP16: {
      std::vector<zvec::ailego::Float16> vector;
      vector.reserve(array.Length());
      for (uint32_t i = 0; i < array.Length(); i++) {
        auto value = array.Get(i);
        if (value.IsNumber()) {
          vector.emplace_back(value.As<Napi::Number>().FloatValue());
        } else {
          return zvec::Status::InvalidArgument(
              "All elements in the vector array for '", vectorName,
              "' must be numbers");
        }
      }
      doc->set(vectorName, std::move(vector));
      break;
    }
    case zvec::DataType::VECTOR_FP32: {
      std::vector<float> vector;
      vector.reserve(array.Length());
      for (uint32_t i = 0; i < array.Length(); i++) {
        auto value = array.Get(i);
        if (value.IsNumber()) {
          vector.emplace_back(value.As<Napi::Number>().FloatValue());
        } else {
          return zvec::Status::InvalidArgument(
              "All elements in the vector array for '", vectorName,
              "' must be numbers");
        }
      }
      doc->set(vectorName, std::move(vector));
      break;
    }
    case zvec::DataType::VECTOR_INT8: {
      std::vector<int8_t> vector;
      vector.reserve(array.Length());
      for (uint32_t i = 0; i < array.Length(); i++) {
        auto value = array.Get(i);
        if (value.IsNumber()) {
          vector.emplace_back(value.As<Napi::Number>().Int32Value());
        } else {
          return zvec::Status::InvalidArgument(
              "All elements in the vector array for '", vectorName,
              "' must be numbers");
        }
      }
      doc->set(vectorName, std::move(vector));
      break;
    }
    default: {
      return zvec::Status::InternalError(
          "Unknown data type: ", static_cast<uint32_t>(schema->data_type()));
    }
  }
  return zvec::Status::OK();
}


zvec::Status ParseVectorObject(const Napi::Object &obj,
                               const std::string &vectorName,
                               zvec::FieldSchema *schema, zvec::Doc *doc) {
  switch (schema->data_type()) {
    case zvec::DataType::SPARSE_VECTOR_FP32: {
      std::vector<uint32_t> indices;
      std::vector<float> values;
      auto stringIndices = obj.GetPropertyNames();
      for (uint32_t i = 0; i < stringIndices.Length(); ++i) {
        auto stringIndex = stringIndices.Get(i).As<Napi::String>().Utf8Value();
        uint32_t index;
        try {
          unsigned long parsed_ulong = std::stoul(stringIndex);
          if (parsed_ulong > std::numeric_limits<uint32_t>::max()) {
            return zvec::Status::InvalidArgument(
                "Index '", stringIndex, "' for '", vectorName,
                "' is out of range for uint32_t.");
          }
          index = static_cast<uint32_t>(parsed_ulong);
        } catch (const std::exception &) {
          return zvec::Status::InvalidArgument(
              "Index '", stringIndex, "' for '", vectorName,
              "' is invalid. Only non-negative integers are allowed.");
        }
        auto vectorValue = obj.Get(stringIndex);
        if (!vectorValue.IsNumber()) {
          return zvec::Status::InvalidArgument(
              "All elements in the vector object for '", vectorName,
              "' must be numbers");
        }
        indices.push_back(index);
        values.push_back(vectorValue.As<Napi::Number>().FloatValue());
      }
      const std::pair<std::vector<uint32_t>, std::vector<float>> sparse_vector{
          std::move(indices), std::move(values)};
      doc->set(vectorName, std::move(sparse_vector));
      break;
    }
    case zvec::DataType::SPARSE_VECTOR_FP16: {
      std::vector<uint32_t> indices;
      std::vector<zvec::ailego::Float16> values;
      auto stringIndices = obj.GetPropertyNames();
      for (uint32_t i = 0; i < stringIndices.Length(); ++i) {
        auto stringIndex = stringIndices.Get(i).As<Napi::String>().Utf8Value();
        uint32_t index;
        try {
          unsigned long parsed_ulong = std::stoul(stringIndex);
          if (parsed_ulong > std::numeric_limits<uint32_t>::max()) {
            return zvec::Status::InvalidArgument(
                "Index '", stringIndex, "' for '", vectorName,
                "' is out of range for uint32_t.");
          }
          index = static_cast<uint32_t>(parsed_ulong);
        } catch (const std::exception &) {
          return zvec::Status::InvalidArgument(
              "Index '", stringIndex, "' for '", vectorName,
              "' is invalid. Only non-negative integers are allowed.");
        }
        auto vectorValue = obj.Get(stringIndex);
        if (!vectorValue.IsNumber()) {
          return zvec::Status::InvalidArgument(
              "All elements in the vector object for '", vectorName,
              "' must be numbers");
        }
        indices.push_back(index);
        values.push_back(vectorValue.As<Napi::Number>().FloatValue());
      }
      const std::pair<std::vector<uint32_t>, std::vector<zvec::ailego::Float16>>
          sparse_vector{std::move(indices), std::move(values)};
      doc->set(vectorName, std::move(sparse_vector));
      break;
    }
    default: {
      return zvec::Status::InternalError(
          "Unknown data type: ", static_cast<uint32_t>(schema->data_type()));
    }
  };
  return zvec::Status::OK();
}


zvec::Status ParseFields(const Napi::Value &value,
                         zvec::CollectionSchema::Ptr &schema, zvec::Doc *doc) {
  if (!value.IsObject()) {
    return zvec::Status::InvalidArgument(
        "Expected argument 'fields' to be an object");
  }
  auto fields = value.As<Napi::Object>();
  auto fieldNames = fields.GetPropertyNames();
  for (uint32_t i = 0; i < fieldNames.Length(); ++i) {
    auto fieldName = fieldNames.Get(i).As<Napi::String>().Utf8Value();
    auto field_schema = schema->get_forward_field(fieldName);
    if (!field_schema) {
      return zvec::Status::InvalidArgument("Field[", fieldName,
                                           "] not found in collection schema");
    }
    auto fieldValue = fields.Get(fieldName);
    auto s = ParseField(fieldValue, fieldName, field_schema, doc);
    if (!s.ok()) {
      return s;
    }
  }
  return zvec::Status::OK();
}


zvec::Status ParseField(const Napi::Value &value, const std::string &fieldName,
                        zvec::FieldSchema *schema, zvec::Doc *doc) {
  switch (schema->data_type()) {
    case zvec::DataType::STRING: {
      if (value.IsString()) {
        doc->set(fieldName, value.As<Napi::String>().Utf8Value());
        return zvec::Status::OK();
      } else {
        return zvec::Status::InvalidArgument("Expected scalar field[",
                                             fieldName, "] to be a string");
      }
    }
    case zvec::DataType::BOOL: {
      if (value.IsBoolean()) {
        doc->set(fieldName, value.As<Napi::Boolean>().Value());
        return zvec::Status::OK();
      } else {
        return zvec::Status::InvalidArgument("Expected scalar field[",
                                             fieldName, "] to be a boolean");
      }
    }
    case zvec::DataType::INT32: {
      if (value.IsNumber()) {
        doc->set(fieldName, value.As<Napi::Number>().Int32Value());
        return zvec::Status::OK();
      } else {
        return zvec::Status::InvalidArgument("Expected scalar field[",
                                             fieldName, "] to be a number");
      }
    }
    case zvec::DataType::INT64: {
      if (value.IsNumber()) {
        doc->set(fieldName, value.As<Napi::Number>().Int64Value());
        return zvec::Status::OK();
      } else {
        return zvec::Status::InvalidArgument("Expected scalar field[",
                                             fieldName, "] to be a number");
      }
    }
    case zvec::DataType::UINT32: {
      if (value.IsNumber()) {
        doc->set(fieldName, value.As<Napi::Number>().Uint32Value());
        return zvec::Status::OK();
      } else {
        return zvec::Status::InvalidArgument("Expected scalar field[",
                                             fieldName, "] to be a number");
      }
    }
    case zvec::DataType::FLOAT: {
      if (value.IsNumber()) {
        doc->set(fieldName, value.As<Napi::Number>().FloatValue());
        return zvec::Status::OK();
      } else {
        return zvec::Status::InvalidArgument("Expected scalar field[",
                                             fieldName, "] to be a number");
      }
    }
    case zvec::DataType::DOUBLE: {
      if (value.IsNumber()) {
        doc->set(fieldName, value.As<Napi::Number>().DoubleValue());
        return zvec::Status::OK();
      } else {
        return zvec::Status::InvalidArgument("Expected scalar field[",
                                             fieldName, "] to be a number");
      }
    }
    case zvec::DataType::ARRAY_STRING:
    case zvec::DataType::ARRAY_BOOL:
    case zvec::DataType::ARRAY_INT64: {
      if (value.IsArray()) {
        return ParseScalarArray(value.As<Napi::Array>(), fieldName, schema,
                                doc);
      } else {
        return zvec::Status::InvalidArgument("Expected scalar field[",
                                             fieldName, "] to be an array");
      }
    }
    case zvec::DataType::ARRAY_INT32:
    case zvec::DataType::ARRAY_UINT32:
    case zvec::DataType::ARRAY_FLOAT:
    case zvec::DataType::ARRAY_DOUBLE: {
      if (value.IsTypedArray()) {
        return ParseScalarTypedArray(value.As<Napi::TypedArray>(), fieldName,
                                     schema, doc);
      } else if (value.IsArray()) {
        return ParseScalarArray(value.As<Napi::Array>(), fieldName, schema,
                                doc);
      } else {
        return zvec::Status::InvalidArgument("Expected scalar field[",
                                             fieldName, "] to be an array");
      }
    }
    default: {
      return zvec::Status::InternalError(
          "Unknown data type: ", static_cast<uint32_t>(schema->data_type()));
    }
  }
}


zvec::Status ParseScalarTypedArray(const Napi::TypedArray &array,
                                   const std::string &fieldName,
                                   zvec::FieldSchema *schema, zvec::Doc *doc) {
  auto ta_type = array.TypedArrayType();
  size_t ta_length = array.ElementLength();
  switch (schema->data_type()) {
    case zvec::DataType::ARRAY_INT32: {
      if (ta_type != napi_int32_array) {
        return zvec::Status::InvalidArgument(
            "Expected scalar field[", fieldName, "] to be an Int32Array");
      }
      Napi::Int32Array int32Array = array.As<Napi::Int32Array>();
      std::vector<int32_t> value;
      const int32_t *src_begin =
          static_cast<const int32_t *>(int32Array.Data());
      const int32_t *src_end = src_begin + ta_length;
      value.assign(src_begin, src_end);
      doc->set(fieldName, std::move(value));
      break;
    }
    case zvec::DataType::ARRAY_UINT32: {
      if (ta_type != napi_uint32_array) {
        return zvec::Status::InvalidArgument(
            "Expected scalar field[", fieldName, "] to be an Uint32Array");
      }
      Napi::Uint32Array uint32Array = array.As<Napi::Uint32Array>();
      std::vector<uint32_t> value;
      const uint32_t *src_begin =
          static_cast<const uint32_t *>(uint32Array.Data());
      const uint32_t *src_end = src_begin + ta_length;
      value.assign(src_begin, src_end);
      doc->set(fieldName, std::move(value));
      break;
    }
    case zvec::DataType::ARRAY_FLOAT: {
      if (ta_type != napi_float32_array) {
        return zvec::Status::InvalidArgument(
            "Expected scalar field[", fieldName, "] to be an Float32Array");
      }
      Napi::Float32Array float32Array = array.As<Napi::Float32Array>();
      std::vector<float> value;
      const float *src_begin = static_cast<const float *>(float32Array.Data());
      const float *src_end = src_begin + ta_length;
      value.assign(src_begin, src_end);
      doc->set(fieldName, std::move(value));
      break;
    }
    case zvec::DataType::ARRAY_DOUBLE: {
      if (ta_type != napi_float64_array) {
        return zvec::Status::InvalidArgument(
            "Expected scalar field[", fieldName, "] to be an Float64Array");
      }
      Napi::Float64Array float64Array = array.As<Napi::Float64Array>();
      std::vector<double> value;
      const double *src_begin =
          static_cast<const double *>(float64Array.Data());
      const double *src_end = src_begin + ta_length;
      value.assign(src_begin, src_end);
      doc->set(fieldName, std::move(value));
      break;
    }
    default: {
      return zvec::Status::InternalError(
          "Unknown data type: ", static_cast<uint32_t>(schema->data_type()));
    }
  }
  return zvec::Status::OK();
}


zvec::Status ParseScalarArray(const Napi::Array &array,
                              const std::string &fieldName,
                              zvec::FieldSchema *schema, zvec::Doc *doc) {
  switch (schema->data_type()) {
    case zvec::DataType::ARRAY_STRING: {
      std::vector<std::string> field_value;
      field_value.reserve(array.Length());
      for (uint32_t i = 0; i < array.Length(); i++) {
        auto value = array.Get(i);
        if (value.IsString()) {
          field_value.emplace_back(value.As<Napi::String>().Utf8Value());
        } else {
          return zvec::Status::InvalidArgument(
              "All elements in the scalar array for '", fieldName,
              "' must be strings");
        }
      }
      doc->set(fieldName, std::move(field_value));
      break;
    }
    case zvec::DataType::ARRAY_BOOL: {
      std::vector<bool> field_value;
      field_value.reserve(array.Length());
      for (uint32_t i = 0; i < array.Length(); i++) {
        auto value = array.Get(i);
        if (value.IsBoolean()) {
          field_value.emplace_back(value.As<Napi::Boolean>());
        } else {
          return zvec::Status::InvalidArgument(
              "All elements in the scalar array for '", fieldName,
              "' must be booleans");
        }
      }
      doc->set(fieldName, std::move(field_value));
      break;
    }
    case zvec::DataType::ARRAY_INT32: {
      std::vector<int32_t> field_value;
      field_value.reserve(array.Length());
      for (uint32_t i = 0; i < array.Length(); i++) {
        auto value = array.Get(i);
        if (value.IsNumber()) {
          field_value.emplace_back(value.As<Napi::Number>().Int32Value());
        } else {
          return zvec::Status::InvalidArgument(
              "All elements in the scalar array for '", fieldName,
              "' must be numbers");
        }
      }
      doc->set(fieldName, std::move(field_value));
      break;
    }
    case zvec::DataType::ARRAY_UINT32: {
      std::vector<uint32_t> field_value;
      field_value.reserve(array.Length());
      for (uint32_t i = 0; i < array.Length(); i++) {
        auto value = array.Get(i);
        if (value.IsNumber()) {
          field_value.emplace_back(value.As<Napi::Number>().Uint32Value());
        } else {
          return zvec::Status::InvalidArgument(
              "All elements in the scalar array for '", fieldName,
              "' must be numbers");
        }
      }
      doc->set(fieldName, std::move(field_value));
      break;
    }
    case zvec::DataType::ARRAY_INT64: {
      std::vector<int64_t> field_value;
      field_value.reserve(array.Length());
      for (uint32_t i = 0; i < array.Length(); i++) {
        auto value = array.Get(i);
        if (value.IsNumber()) {
          field_value.emplace_back(value.As<Napi::Number>().Int64Value());
        } else {
          return zvec::Status::InvalidArgument(
              "All elements in the scalar array for '", fieldName,
              "' must be numbers");
        }
      }
      doc->set(fieldName, std::move(field_value));
      break;
    }
    case zvec::DataType::ARRAY_FLOAT: {
      std::vector<float> field_value;
      field_value.reserve(array.Length());
      for (uint32_t i = 0; i < array.Length(); i++) {
        auto value = array.Get(i);
        if (value.IsNumber()) {
          field_value.emplace_back(value.As<Napi::Number>().FloatValue());
        } else {
          return zvec::Status::InvalidArgument(
              "All elements in the scalar array for '", fieldName,
              "' must be numbers");
        }
      }
      doc->set(fieldName, std::move(field_value));
      break;
    }
    case zvec::DataType::ARRAY_DOUBLE: {
      std::vector<double> field_value;
      field_value.reserve(array.Length());
      for (uint32_t i = 0; i < array.Length(); i++) {
        auto value = array.Get(i);
        if (value.IsNumber()) {
          field_value.emplace_back(value.As<Napi::Number>().DoubleValue());
        } else {
          return zvec::Status::InvalidArgument(
              "All elements in the scalar array for '", fieldName,
              "' must be numbers");
        }
      }
      doc->set(fieldName, std::move(field_value));
      break;
    }
    default: {
      return zvec::Status::InternalError(
          "Unknown data type: ", static_cast<uint32_t>(schema->data_type()));
    }
  }
  return zvec::Status::OK();
}


Napi::Object CreateDoc(Napi::Env env, zvec::CollectionSchema::Ptr schema,
                       zvec::Doc::Ptr doc) {
  auto obj = Napi::Object::New(env);
  obj.Set("id", doc->pk());
  obj.Set("score", doc->score());
  if (doc->is_empty()) {
    obj.Set("vectors", Napi::Object::New(env));
    obj.Set("fields", Napi::Object::New(env));
  } else {
    obj.Set("vectors", CreateVectors(env, schema, doc));
    obj.Set("fields", CreateFields(env, schema, doc));
  }
  return obj;
}


Napi::Object CreateVectors(Napi::Env &env, zvec::CollectionSchema::Ptr &schema,
                           zvec::Doc::Ptr &doc) {
  auto obj = Napi::Object::New(env);
  for (const auto &vec_schema : schema->vector_fields()) {
    const std::string &vec = vec_schema->name();
    if (!doc->has_value(vec)) {
      continue;
    }
    if (vec_schema->is_dense_vector()) {
      obj.Set(vec, CreateVectorArray(env, vec, vec_schema.get(), doc));
    } else {
      obj.Set(vec, CreateVectorObject(env, vec, vec_schema.get(), doc));
    }
  }
  return obj;
}


Napi::Array CreateVectorArray(Napi::Env &env, const std::string &vectorName,
                              zvec::FieldSchema *schema, zvec::Doc::Ptr &doc) {
  auto array = Napi::Array::New(env);
  switch (schema->data_type()) {
    case zvec::DataType::VECTOR_FP16: {
      auto res = doc->get<std::vector<zvec::ailego::Float16>>(vectorName);
      if (res.has_value()) {
        const std::vector<zvec::ailego::Float16> &vector = res.value();
        for (size_t i = 0; i < vector.size(); i++) {
          array.Set(i, static_cast<float>(vector[i]));
        }
      }
      break;
    }
    case zvec::DataType::VECTOR_FP32: {
      auto res = doc->get<std::vector<float>>(vectorName);
      if (res.has_value()) {
        const std::vector<float> &vector = res.value();
        for (size_t i = 0; i < vector.size(); i++) {
          array.Set(i, vector[i]);
        }
      }
      break;
    }
    case zvec::DataType::VECTOR_INT8: {
      auto res = doc->get<std::vector<int8_t>>(vectorName);
      if (res.has_value()) {
        const std::vector<int8_t> &vector = res.value();
        for (size_t i = 0; i < vector.size(); i++) {
          array.Set(i, vector[i]);
        }
      }
      break;
    }
    default: {
      break;
    }
  }
  return array;
}


Napi::Object CreateVectorObject(Napi::Env &env, const std::string &vectorName,
                                zvec::FieldSchema *schema,
                                zvec::Doc::Ptr &doc) {
  auto obj = Napi::Object::New(env);
  switch (schema->data_type()) {
    case zvec::DataType::SPARSE_VECTOR_FP32: {
      auto res = doc->get<std::pair<std::vector<uint32_t>, std::vector<float>>>(
          vectorName);
      if (res.has_value()) {
        const auto &indices = res.value().first;
        const auto &values = res.value().second;
        for (size_t i = 0; i < indices.size(); i++) {
          obj.Set(indices[i], values[i]);
        }
      }
      break;
    }
    case zvec::DataType::SPARSE_VECTOR_FP16: {
      auto res = doc->get<
          std::pair<std::vector<uint32_t>, std::vector<zvec::ailego::Float16>>>(
          vectorName);
      if (res.has_value()) {
        const auto &indices = res.value().first;
        const auto &values = res.value().second;
        for (size_t i = 0; i < indices.size(); i++) {
          obj.Set(indices[i], static_cast<float>(values[i]));
        }
      }
      break;
    }
    default: {
      break;
    }
  }
  return obj;
}


Napi::Object CreateFields(Napi::Env &env, zvec::CollectionSchema::Ptr &schema,
                          zvec::Doc::Ptr &doc) {
  auto obj = Napi::Object::New(env);
  for (const auto &field_schema : schema->forward_fields()) {
    const std::string &field = field_schema->name();
    if (!doc->has_value(field)) {
      continue;
    }
    if (field_schema->is_array_type()) {
      obj.Set(field, CreateScalarArray(env, field, field_schema.get(), doc));
    } else {
      switch (field_schema->data_type()) {
        case zvec::DataType::STRING: {
          obj.Set(field, doc->get<std::string>(field).value());
          break;
        }
        case zvec::DataType::BOOL: {
          obj.Set(field, doc->get<bool>(field).value());
          break;
        }
        case zvec::DataType::INT32: {
          obj.Set(field, doc->get<int32_t>(field).value());
          break;
        }
        case zvec::DataType::INT64: {
          obj.Set(field, doc->get<int64_t>(field).value());
          break;
        }
        case zvec::DataType::UINT32: {
          obj.Set(field, doc->get<uint32_t>(field).value());
          break;
        }
        case zvec::DataType::FLOAT: {
          obj.Set(field, doc->get<float>(field).value());
          break;
        }
        case zvec::DataType::DOUBLE: {
          obj.Set(field, doc->get<double>(field).value());
          break;
        }
        default: {
          break;
        }
      }
    }
  }
  return obj;
}


Napi::Array CreateScalarArray(Napi::Env &env, const std::string &fieldName,
                              zvec::FieldSchema *schema, zvec::Doc::Ptr &doc) {
  auto array = Napi::Array::New(env);
  switch (schema->data_type()) {
    case zvec::DataType::ARRAY_STRING: {
      auto res = doc->get<std::vector<std::string>>(fieldName);
      if (res.has_value()) {
        const std::vector<std::string> &scalar = res.value();
        for (size_t i = 0; i < scalar.size(); i++) {
          array.Set(i, scalar[i]);
        }
      }
      return array;
    }
    case zvec::DataType::ARRAY_BOOL: {
      auto res = doc->get<std::vector<bool>>(fieldName);
      if (res.has_value()) {
        const std::vector<bool> &scalar = res.value();
        for (size_t i = 0; i < scalar.size(); i++) {
          array.Set(i, Napi::Boolean::New(env, scalar[i]));
        }
      }
      return array;
    }
    case zvec::DataType::ARRAY_INT32: {
      auto res = doc->get<std::vector<int32_t>>(fieldName);
      if (res.has_value()) {
        const std::vector<int32_t> &scalar = res.value();
        for (size_t i = 0; i < scalar.size(); i++) {
          array.Set(i, scalar[i]);
        }
      }
      return array;
    }
    case zvec::DataType::ARRAY_UINT32: {
      auto res = doc->get<std::vector<uint32_t>>(fieldName);
      if (res.has_value()) {
        const std::vector<uint32_t> &scalar = res.value();
        for (size_t i = 0; i < scalar.size(); i++) {
          array.Set(i, scalar[i]);
        }
      }
      return array;
    }
    case zvec::DataType::ARRAY_INT64: {
      auto res = doc->get<std::vector<int64_t>>(fieldName);
      if (res.has_value()) {
        const std::vector<int64_t> &scalar = res.value();
        for (size_t i = 0; i < scalar.size(); i++) {
          array.Set(i, scalar[i]);
        }
      }
      return array;
    }
    case zvec::DataType::ARRAY_FLOAT: {
      auto res = doc->get<std::vector<float>>(fieldName);
      if (res.has_value()) {
        const std::vector<float> &scalar = res.value();
        for (size_t i = 0; i < scalar.size(); i++) {
          array.Set(i, scalar[i]);
        }
      }
      return array;
    }
    case zvec::DataType::ARRAY_DOUBLE: {
      auto res = doc->get<std::vector<double>>(fieldName);
      if (res.has_value()) {
        const std::vector<double> &scalar = res.value();
        for (size_t i = 0; i < scalar.size(); i++) {
          array.Set(i, scalar[i]);
        }
      }
      return array;
    }
    default: {
      return array;
    }
  }
}


}  // namespace binding