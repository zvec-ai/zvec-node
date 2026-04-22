#include "collection.h"
#include "doc.h"
#include "params.h"
#include "schema.h"
#include "types.h"


namespace binding {


Napi::Object CreateAndOpenCollection(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  auto constructors = get_constructors(env);
  if (!constructors) {
    return Napi::Object::New(env);
  }
  auto length = info.Length();

  if (length < 2 || length > 3) {
    ThrowIfNotOk(env,
                 zvec::Status::InvalidArgument(
                     "CreateAndOpen() expects 2 to 3 arguments: (path: string, "
                     "schema: CollectionSchema, options?: CollectionOptions)"));
    return Napi::Object::New(env);
  }

  if (!info[0].IsString()) {
    ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                          "CreateAndOpen(): argument 'path' must be a string"));
    return Napi::Object::New(env);
  }
  std::string path = info[0].As<Napi::String>().Utf8Value();

  if (!info[1].IsObject()) {
    ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                          "CreateAndOpen(): argument 'schema' must be a "
                          "CollectionSchema object"));
    return Napi::Object::New(env);
  }
  Napi::Object schemaObj = info[1].As<Napi::Object>();
  if (!schemaObj.InstanceOf(
          constructors->collectionSchemaConstructor.Value())) {
    ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                          "CreateAndOpen(): argument 'schema' must be a "
                          "CollectionSchema object"));
    return Napi::Object::New(env);
  }
  auto collectionSchema = CollectionSchema::Unwrap(schemaObj);
  if (!collectionSchema) {
    ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                          "CreateAndOpen(): argument 'schema' must be a "
                          "CollectionSchema object"));
    return Napi::Object::New(env);
  }

  auto options{zvec::CollectionOptions{}};
  if (length > 2) {
    if (!info[2].IsObject()) {
      ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                            "CreateAndOpen(): argument 'options' must be a "
                            "CollectionOptions object"));
      return Napi::Object::New(env);
    }
    auto parsed_options = ParseCollectionOptions(info[2].As<Napi::Object>());
    if (parsed_options) {
      options = parsed_options.value();
    } else {
      ThrowIfNotOk(env, parsed_options.error());
      return Napi::Object::New(env);
    }
  }

  auto collection = zvec::Collection::CreateAndOpen(
      path, *(collectionSchema->get_wrapped()), options);
  if (collection) {
    Napi::Object obj = constructors->collectionConstructor.New(
        {Napi::External<void>::New(env, kInternalConstructionMarker)});
    auto returned_collection = Collection::Unwrap(obj);
    returned_collection->set_wrapped(env, collection.value());
    return obj;
  } else {
    ThrowIfNotOk(env, collection.error());
    return Napi::Object::New(env);
  }
}


Napi::Object OpenCollection(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  auto constructors = get_constructors(env);
  if (!constructors) {
    return Napi::Object::New(env);
  }
  auto length = info.Length();

  if (length < 1 || length > 2) {
    ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                          "Open() expects 1 to 2 arguments: (path: string, "
                          "options?: CollectionOptions)"));
    return Napi::Object::New(env);
  }

  if (!info[0].IsString()) {
    ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                          "Open(): argument 'path' must be a string"));
    return Napi::Object::New(env);
  }
  std::string path = info[0].As<Napi::String>().Utf8Value();

  auto options{zvec::CollectionOptions{}};
  if (length > 1) {
    if (!info[1].IsObject()) {
      ThrowIfNotOk(
          env,
          zvec::Status::InvalidArgument(
              "Open(): argument 'options' must be a CollectionOptions object"));
      return Napi::Object::New(env);
    }
    auto parsed_options = ParseCollectionOptions(info[1].As<Napi::Object>());
    if (parsed_options) {
      options = parsed_options.value();
    } else {
      ThrowIfNotOk(env, parsed_options.error());
      return Napi::Object::New(env);
    }
  }

  auto collection = zvec::Collection::Open(path, options);
  if (collection) {
    Napi::Object obj = constructors->collectionConstructor.New(
        {Napi::External<void>::New(env, kInternalConstructionMarker)});
    auto returned_collection = Collection::Unwrap(obj);
    returned_collection->set_wrapped(env, collection.value());
    return obj;
  } else {
    ThrowIfNotOk(env, collection.error());
    return Napi::Object::New(env);
  }
}


Napi::Object Collection::Init(Napi::Env env, Napi::Object exports,
                              AddonConstructors *ctors) {
  Napi::Function func = DefineClass(
      env, "Collection",
      {
          InstanceAccessor("path", &Collection::Path, nullptr),
          InstanceAccessor("schema", &Collection::Schema, nullptr),
          InstanceAccessor("options", &Collection::Options, nullptr),
          InstanceAccessor("stats", &Collection::Stats, nullptr),
          InstanceMethod("insertSync", &Collection::Insert),
          InstanceMethod("upsertSync", &Collection::Upsert),
          InstanceMethod("updateSync", &Collection::Update),
          InstanceMethod("deleteSync", &Collection::Delete),
          InstanceMethod("deleteByFilterSync", &Collection::DeleteByFilter),
          InstanceMethod("_internalQuery", &Collection::Query),
          InstanceMethod("fetchSync", &Collection::Fetch),
          InstanceMethod("optimizeSync", &Collection::Optimize),
          InstanceMethod("closeSync", &Collection::Close),
          InstanceMethod("destroySync", &Collection::Destroy),
          InstanceMethod("addColumnSync", &Collection::AddColumn),
          InstanceMethod("dropColumnSync", &Collection::DropColumn),
          InstanceMethod("alterColumnSync", &Collection::AlterColumn),
          InstanceMethod("createIndexSync", &Collection::CreateIndex),
          InstanceMethod("dropIndexSync", &Collection::DropIndex),
      });

  ctors->collectionConstructor = Napi::Persistent(func);
  exports.Set("Collection", func);
  exports.Set("createAndOpen",
              Napi::Function::New(env, CreateAndOpenCollection));
  exports.Set("open", Napi::Function::New(env, OpenCollection));
  return exports;
}


Collection::Collection(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<Collection>(info) {
  Napi::Env env = info.Env();
  bool called_from_internal = (info.Length() == 1 && info[0].IsExternal() &&
                               info[0].As<Napi::External<void>>().Data() ==
                                   kInternalConstructionMarker);
  if (!called_from_internal) {
    ThrowIfNotOk(env, zvec::Status::PermissionDenied(
                          "Direct instantiation of Collection is not allowed. "
                          "Please use appropriate factory functions like "
                          "'CreateAndOpen()' or 'Open()'"));
  }
}


zvec::Collection::Ptr Collection::get_wrapped_collection() {
  return collection_;
}


zvec::CollectionSchema::Ptr Collection::get_wrapped_schema() {
  std::shared_lock<std::shared_mutex> lock(schema_lock_);
  return schema_;
}


void Collection::set_wrapped(Napi::Env &env, zvec::Collection::Ptr collection) {
  set_wrapped_collection(collection);
  set_wrapped_schema(env);
}


void Collection::set_wrapped_collection(zvec::Collection::Ptr collection) {
  collection_ = collection;
}


void Collection::set_wrapped_schema(Napi::Env &env) {
  std::unique_lock<std::shared_mutex> lock(schema_lock_);
  if (auto schema = collection_->Schema(); schema) {
    schema_ = std::make_shared<zvec::CollectionSchema>(schema.value());
  } else {
    ThrowIfNotOk(env, schema.error());
  }
}


Napi::Value Collection::Path(const Napi::CallbackInfo &info) {
  if (auto path = collection_->Path(); path) {
    return Napi::String::New(info.Env(), path.value());
  } else {
    ThrowIfNotOk(info.Env(), path.error());
    return info.Env().Undefined();
  }
}


Napi::Value Collection::Schema(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  if (auto schema = collection_->Schema(); schema) {
    auto constructors = get_constructors(env);
    if (!constructors) {
      return env.Undefined();
    }
    Napi::Object obj = constructors->collectionSchemaConstructor.New(
        {Napi::External<void>::New(env, kInternalConstructionMarker)});
    auto returned_schema = CollectionSchema::Unwrap(obj);
    returned_schema->set_wrapped(
        std::make_shared<zvec::CollectionSchema>(schema.value()));
    return obj;
  } else {
    ThrowIfNotOk(env, schema.error());
    return env.Undefined();
  }
}


Napi::Value Collection::Options(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  if (auto options = collection_->Options(); options) {
    return CreateCollectionOptions(env, options.value());
  } else {
    ThrowIfNotOk(env, options.error());
    return env.Undefined();
  }
}


Napi::Value Collection::Stats(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  if (auto stats = collection_->Stats(); stats) {
    auto obj = Napi::Object::New(env);
    obj.Set("docCount", stats.value().doc_count);
    auto indexCompleteness = Napi::Object::New(env);
    for (const auto &pair : stats.value().index_completeness) {
      indexCompleteness.Set(pair.first, pair.second);
    }
    obj.Set("indexCompleteness", indexCompleteness);
    return obj;
  } else {
    ThrowIfNotOk(env, stats.error());
    return env.Undefined();
  }
}


Napi::Value Collection::Insert(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  if (info.Length() != 1) {
    ThrowIfNotOk(
        env, zvec::Status::InvalidArgument(
                 "Collection.insert(): Expected exactly 1 argument. "
                 "Argument must be a Doc object or an array of Doc objects."));
    return env.Undefined();
  }

  std::vector<zvec::Doc> docs{};

  if (info[0].IsArray()) {
    Napi::Array docArray = info[0].As<Napi::Array>();
    uint32_t length = docArray.Length();
    docs.reserve(length);
    for (uint32_t i = 0; i < length; i++) {
      auto parsed_doc = ParseDoc(docArray.Get(i), get_wrapped_schema());
      if (!parsed_doc) {
        ThrowIfNotOk(env, parsed_doc.error());
        return env.Undefined();
      }
      docs.emplace_back(std::move(parsed_doc.value()));
    }
    auto res = collection_->Insert(docs);
    if (res) {
      const zvec::WriteResults &statuses = res.value();
      Napi::Array statusArray = Napi::Array::New(env);
      for (size_t i = 0; i < statuses.size(); i++) {
        statusArray.Set(i, CreateStatusObject(env, statuses[i]));
      }
      return statusArray;
    } else {
      ThrowIfNotOk(env, res.error());
      return env.Undefined();
    }
  } else {
    auto parsed_doc = ParseDoc(info[0], get_wrapped_schema());
    if (!parsed_doc) {
      ThrowIfNotOk(env, parsed_doc.error());
      return env.Undefined();
    }
    docs.emplace_back(std::move(parsed_doc.value()));
    auto res = collection_->Insert(docs);
    if (res) {
      const zvec::WriteResults &statuses = res.value();
      return CreateStatusObject(env, statuses[0]);
    } else {
      ThrowIfNotOk(env, res.error());
      return env.Undefined();
    }
  }
}


Napi::Value Collection::Upsert(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  if (info.Length() != 1) {
    ThrowIfNotOk(
        env, zvec::Status::InvalidArgument(
                 "Collection.upsert(): Expected exactly 1 argument. "
                 "Argument must be a Doc object or an array of Doc objects."));
    return env.Undefined();
  }

  std::vector<zvec::Doc> docs{};

  if (info[0].IsArray()) {
    Napi::Array docArray = info[0].As<Napi::Array>();
    uint32_t length = docArray.Length();
    docs.reserve(length);
    for (uint32_t i = 0; i < length; i++) {
      auto parsed_doc = ParseDoc(docArray.Get(i), get_wrapped_schema());
      if (!parsed_doc) {
        ThrowIfNotOk(env, parsed_doc.error());
        return env.Undefined();
      }
      docs.emplace_back(std::move(parsed_doc.value()));
    }
    auto res = collection_->Upsert(docs);
    if (res) {
      const zvec::WriteResults &statuses = res.value();
      Napi::Array statusArray = Napi::Array::New(env);
      for (size_t i = 0; i < statuses.size(); i++) {
        statusArray.Set(i, CreateStatusObject(env, statuses[i]));
      }
      return statusArray;
    } else {
      ThrowIfNotOk(env, res.error());
      return env.Undefined();
    }
  } else {
    auto parsed_doc = ParseDoc(info[0], get_wrapped_schema());
    if (!parsed_doc) {
      ThrowIfNotOk(env, parsed_doc.error());
      return env.Undefined();
    }
    docs.emplace_back(std::move(parsed_doc.value()));
    auto res = collection_->Upsert(docs);
    if (res) {
      const zvec::WriteResults &statuses = res.value();
      return CreateStatusObject(env, statuses[0]);
    } else {
      ThrowIfNotOk(env, res.error());
      return env.Undefined();
    }
  }
}


Napi::Value Collection::Update(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  if (info.Length() != 1) {
    ThrowIfNotOk(
        env, zvec::Status::InvalidArgument(
                 "Collection.update(): Expected exactly 1 argument. "
                 "Argument must be a Doc object or an array of Doc objects."));
    return env.Undefined();
  }

  std::vector<zvec::Doc> docs{};

  if (info[0].IsArray()) {
    Napi::Array docArray = info[0].As<Napi::Array>();
    uint32_t length = docArray.Length();
    docs.reserve(length);
    for (uint32_t i = 0; i < length; i++) {
      auto parsed_doc = ParseDoc(docArray.Get(i), get_wrapped_schema());
      if (!parsed_doc) {
        ThrowIfNotOk(env, parsed_doc.error());
        return env.Undefined();
      }
      docs.emplace_back(std::move(parsed_doc.value()));
    }
    auto res = collection_->Update(docs);
    if (res) {
      const zvec::WriteResults &statuses = res.value();
      Napi::Array statusArray = Napi::Array::New(env);
      for (size_t i = 0; i < statuses.size(); i++) {
        statusArray.Set(i, CreateStatusObject(env, statuses[i]));
      }
      return statusArray;
    } else {
      ThrowIfNotOk(env, res.error());
      return env.Undefined();
    }
  } else {
    auto parsed_doc = ParseDoc(info[0], get_wrapped_schema());
    if (!parsed_doc) {
      ThrowIfNotOk(env, parsed_doc.error());
      return env.Undefined();
    }
    docs.emplace_back(std::move(parsed_doc.value()));
    auto res = collection_->Update(docs);
    if (res) {
      const zvec::WriteResults &statuses = res.value();
      return CreateStatusObject(env, statuses[0]);
    } else {
      ThrowIfNotOk(env, res.error());
      return env.Undefined();
    }
  }
}


Napi::Value Collection::Delete(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  if (info.Length() != 1) {
    ThrowIfNotOk(
        env, zvec::Status::InvalidArgument(
                 "Collection.delete(): Expected exactly 1 argument. "
                 "Argument must be an id string or an array of id strings."));
    return env.Undefined();
  }

  std::vector<std::string> pks{};

  if (info[0].IsArray()) {
    Napi::Array pkArray = info[0].As<Napi::Array>();
    uint32_t length = pkArray.Length();
    pks.reserve(length);
    for (uint32_t i = 0; i < length; i++) {
      if (pkArray.Get(i).IsString()) {
        pks.emplace_back(pkArray.Get(i).As<Napi::String>().Utf8Value());
      } else {
        ThrowIfNotOk(env,
                     zvec::Status::InvalidArgument("Expected a string for id"));
        return env.Undefined();
      }
    }
    auto res = collection_->Delete(pks);
    if (res) {
      const zvec::WriteResults &statuses = res.value();
      Napi::Array statusArray = Napi::Array::New(env);
      for (size_t i = 0; i < statuses.size(); i++) {
        statusArray.Set(i, CreateStatusObject(env, statuses[i]));
      }
      return statusArray;
    } else {
      ThrowIfNotOk(env, res.error());
      return env.Undefined();
    }
  } else {
    if (info[0].IsString()) {
      pks.emplace_back(info[0].As<Napi::String>().Utf8Value());
    } else {
      ThrowIfNotOk(env,
                   zvec::Status::InvalidArgument("Expected a string for id"));
      return env.Undefined();
    }
    auto res = collection_->Delete(pks);
    if (res) {
      const zvec::WriteResults &statuses = res.value();
      return CreateStatusObject(env, statuses[0]);
    } else {
      ThrowIfNotOk(env, res.error());
      return env.Undefined();
    }
  }
}


Napi::Value Collection::DeleteByFilter(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  if (info.Length() != 1 || !info[0].IsString()) {
    ThrowIfNotOk(
        env, zvec::Status::InvalidArgument(
                 "Collection.deleteByFilter(): Expected exactly 1 argument. "
                 "Argument must be a string"));
    return env.Undefined();
  }
  auto s = collection_->DeleteByFilter(info[0].As<Napi::String>().Utf8Value());
  return CreateStatusObject(env, s);
}


Napi::Value Collection::Query(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  if (info.Length() != 1) {
    ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                          "Collection.query(): Expected exactly 1 argument. "
                          "Argument must be an Query object"));
    return env.Undefined();
  }

  if (auto parsed_query = ParseVectorQuery(info[0], get_wrapped_schema());
      parsed_query) {
    auto res = collection_->Query(parsed_query.value());
    if (res) {
      Napi::Array array = Napi::Array::New(env);
      const zvec::DocPtrList &doc_list = res.value();
      for (size_t i = 0; i < doc_list.size(); i++) {
        array.Set(i, CreateDoc(env, get_wrapped_schema(), doc_list[i]));
      }
      return array;
    } else {
      ThrowIfNotOk(env, res.error());
      return env.Undefined();
    }
  } else {
    ThrowIfNotOk(env, parsed_query.error());
    return env.Undefined();
  }
}


Napi::Value Collection::Fetch(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  if (info.Length() != 1) {
    ThrowIfNotOk(
        env, zvec::Status::InvalidArgument(
                 "Collection.fetch(): Expected exactly 1 argument. "
                 "Argument must be an id string or an array of id strings."));
    return env.Undefined();
  }

  std::vector<std::string> pks{};
  zvec::Result<zvec::DocPtrMap> res;

  if (info[0].IsArray()) {
    Napi::Array pkArray = info[0].As<Napi::Array>();
    uint32_t length = pkArray.Length();
    pks.reserve(length);
    for (uint32_t i = 0; i < length; i++) {
      if (pkArray.Get(i).IsString()) {
        pks.emplace_back(pkArray.Get(i).As<Napi::String>().Utf8Value());
      } else {
        ThrowIfNotOk(env,
                     zvec::Status::InvalidArgument("Expected a string for id"));
        return env.Undefined();
      }
    }
    res = collection_->Fetch(pks);
  } else {
    if (info[0].IsString()) {
      pks.emplace_back(info[0].As<Napi::String>().Utf8Value());
    } else {
      ThrowIfNotOk(env,
                   zvec::Status::InvalidArgument("Expected a string for id"));
      return env.Undefined();
    }
    res = collection_->Fetch(pks);
  }

  if (res) {
    Napi::Object obj = Napi::Object::New(env);
    const zvec::DocPtrMap &map = res.value();
    for (const auto &pair : map) {
      if (!pair.second) {
        continue;
      }
      obj.Set(pair.first, CreateDoc(env, get_wrapped_schema(), pair.second));
    }
    return obj;
  } else {
    ThrowIfNotOk(env, res.error());
    return env.Undefined();
  }
}


Napi::Value Collection::Optimize(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  if (info.Length() > 1) {
    ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                          "Collection.optimize(): Expected 0 to 1 argument. "
                          "Argument must be an OptimizeOptions object."));
    return env.Undefined();
  }
  auto options{zvec::OptimizeOptions{}};
  if (info.Length() == 1) {
    if (!info[0].IsObject()) {
      ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                            "Collection.optimize(): Expected 0 to 1 argument. "
                            "Argument must be an OptimizeOptions object."));
      return env.Undefined();
    }
    auto parsed_options = ParseOptimizeOptions(info[0].As<Napi::Object>());
    if (parsed_options) {
      options = parsed_options.value();
    } else {
      ThrowIfNotOk(env, parsed_options.error());
      return Napi::Object::New(env);
    }
  }
  ThrowIfNotOk(env, collection_->Optimize(options));
  return env.Undefined();
}


Napi::Value Collection::Close(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  if (info.Length() != 0) {
    ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                          "Collection.close(): Expected no argument"));
    return env.Undefined();
  }
  std::unique_lock<std::mutex> lock(ddl_lock_);
  collection_ = nullptr;
  return env.Undefined();
}


Napi::Value Collection::Destroy(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  if (info.Length() != 0) {
    ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                          "Collection.destroy(): Expected no argument"));
    return env.Undefined();
  }
  ThrowIfNotOk(env, collection_->Destroy());
  return env.Undefined();
}


Napi::Value Collection::AddColumn(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  if (info.Length() != 1 || !info[0].IsObject()) {
    ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                          "Collection.addColumn(): Expected exactly 1 "
                          "argument. Argument must be an object."));
    return env.Undefined();
  }
  auto obj = info[0].As<Napi::Object>();

  if (!obj.Has("fieldSchema")) {
    ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                          "Missing required argument 'fieldSchema' in "
                          "Collection.addColumn()"));
    return env.Undefined();
  }
  auto parsed_field_schema = ParseFieldSchema(obj.Get("fieldSchema"));
  if (!parsed_field_schema) {
    ThrowIfNotOk(env, parsed_field_schema.error());
    return env.Undefined();
  }
  std::string expression{};
  if (obj.Has("expression")) {
    if (obj.Get("expression").IsString()) {
      expression = obj.Get("expression").As<Napi::String>().Utf8Value();
    } else {
      ThrowIfNotOk(
          env, zvec::Status::InvalidArgument("Collection.addColumn(): argument "
                                             "'expression' must be a string"));
      return env.Undefined();
    }
  }
  auto options{zvec::AddColumnOptions{0}};
  if (obj.Has("options")) {
    if (obj.Get("options").IsObject()) {
      auto parsed_options =
          ParseAddColumnOptions(obj.Get("options").As<Napi::Object>());
      if (parsed_options) {
        options = parsed_options.value();
      } else {
        ThrowIfNotOk(env, parsed_options.error());
        return env.Undefined();
      }
    } else {
      ThrowIfNotOk(
          env,
          zvec::Status::InvalidArgument(
              "Collection.addColumn(): argument 'options' must be an object"));
      return env.Undefined();
    }
  }
  std::unique_lock<std::mutex> lock(ddl_lock_);
  ThrowIfNotOk(env, collection_->AddColumn(parsed_field_schema.value(),
                                           expression, options));
  set_wrapped_schema(env);
  return env.Undefined();
}


Napi::Value Collection::DropColumn(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  if (info.Length() != 1 || !info[0].IsString()) {
    ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                          "Collection.dropColumn(): Expected exactly 1 "
                          "argument. Argument must be a string."));
    return env.Undefined();
  }
  auto field_name = info[0].As<Napi::String>().Utf8Value();
  std::unique_lock<std::mutex> lock(ddl_lock_);
  ThrowIfNotOk(env, collection_->DropColumn(field_name));
  set_wrapped_schema(env);
  return env.Undefined();
}


Napi::Value Collection::AlterColumn(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  if (info.Length() != 1 || !info[0].IsObject()) {
    ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                          "Collection.alterColumn(): Expected exactly 1 "
                          "argument. Argument must be an object."));
    return env.Undefined();
  }
  auto obj = info[0].As<Napi::Object>();

  if (!obj.Has("columnName")) {
    ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                          "Missing required argument 'columnName' in "
                          "Collection.alterColumn()"));
    return env.Undefined();
  }
  if (!obj.Get("columnName").IsString()) {
    ThrowIfNotOk(
        env, zvec::Status::InvalidArgument("Collection.alterColumn(): argument "
                                           "'columnName' must be a string"));
    return env.Undefined();
  }
  std::string column_name =
      obj.Get("columnName").As<Napi::String>().Utf8Value();

  std::string new_column_name{""};
  if (obj.Has("newColumnName")) {
    if (obj.Get("newColumnName").IsString()) {
      new_column_name = obj.Get("newColumnName").As<Napi::String>().Utf8Value();
    } else {
      ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                            "Collection.alterColumn(): argument "
                            "'newColumnName' must be a string"));
      return env.Undefined();
    }
  }

  zvec::FieldSchema::Ptr field_schema{nullptr};
  if (obj.Has("fieldSchema")) {
    auto parsed_field_schema = ParseFieldSchema(obj.Get("fieldSchema"));
    if (parsed_field_schema) {
      field_schema = parsed_field_schema.value();
    } else {
      ThrowIfNotOk(env, parsed_field_schema.error());
      return env.Undefined();
    }
  }

  auto options{zvec::AlterColumnOptions{0}};
  if (obj.Has("options")) {
    if (obj.Get("options").IsObject()) {
      auto parsed_options =
          ParseAlterColumnOptions(obj.Get("options").As<Napi::Object>());
      if (parsed_options) {
        options = parsed_options.value();
      } else {
        ThrowIfNotOk(env, parsed_options.error());
        return env.Undefined();
      }
    } else {
      ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                            "Collection.alterColumn(): argument 'options' must "
                            "be an object"));
      return env.Undefined();
    }
  }

  if (new_column_name.empty() && !field_schema) {
    ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                          "Collection.alterColumn(): either 'newColumnName' or "
                          "'fieldSchema' must be provided"));
  } else if (!new_column_name.empty() && field_schema) {
    ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                          "Collection.alterColumn(): 'newColumnName' and "
                          "'fieldSchema' are mutually exclusive"));
  } else {
    std::unique_lock<std::mutex> lock(ddl_lock_);
    ThrowIfNotOk(env, collection_->AlterColumn(column_name, new_column_name,
                                               field_schema, options));
    set_wrapped_schema(env);
  }
  return env.Undefined();
}


Napi::Value Collection::CreateIndex(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  if (info.Length() != 1 || !info[0].IsObject()) {
    ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                          "Collection.createIndex(): Expected exactly 1 "
                          "argument. Argument must be an object."));
    return env.Undefined();
  }
  auto obj = info[0].As<Napi::Object>();

  if (!obj.Has("fieldName")) {
    ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                          "Missing required argument 'fieldName' in "
                          "Collection.createIndex()"));
    return env.Undefined();
  }
  if (!obj.Get("fieldName").IsString()) {
    ThrowIfNotOk(
        env, zvec::Status::InvalidArgument("Collection.createIndex(): argument "
                                           "'fieldName' must be a string"));
    return env.Undefined();
  }
  auto field_name = obj.Get("fieldName").As<Napi::String>().Utf8Value();

  if (!obj.Has("indexParams")) {
    ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                          "Missing required argument 'indexParams' in "
                          "Collection.createIndex()"));
    return env.Undefined();
  }
  auto parsed_index_params = ParseIndexParams(obj.Get("indexParams"));
  if (!parsed_index_params) {
    ThrowIfNotOk(env, parsed_index_params.error());
    return env.Undefined();
  }

  auto options{zvec::CreateIndexOptions{}};
  if (obj.Has("indexOptions")) {
    if (obj.Get("indexOptions").IsObject()) {
      auto parsed_options =
          ParseCreateIndexOptions(obj.Get("indexOptions").As<Napi::Object>());
      if (parsed_options) {
        options = parsed_options.value();
      } else {
        ThrowIfNotOk(env, parsed_options.error());
        return env.Undefined();
      }
    } else {
      ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                            "Collection.createIndex(): argument 'indexOptions' "
                            "must be an object"));
      return env.Undefined();
    }
  }

  std::unique_lock<std::mutex> lock(ddl_lock_);
  ThrowIfNotOk(env, collection_->CreateIndex(
                        field_name, parsed_index_params.value(), options));
  set_wrapped_schema(env);
  return env.Undefined();
}


Napi::Value Collection::DropIndex(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  if (info.Length() != 1 || !info[0].IsString()) {
    ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                          "Collection.dropIndex(): Expected exactly 1 "
                          "argument. Argument must be a string."));
    return env.Undefined();
  }
  auto field_name = info[0].As<Napi::String>().Utf8Value();
  std::unique_lock<std::mutex> lock(ddl_lock_);
  ThrowIfNotOk(env, collection_->DropIndex(field_name));
  set_wrapped_schema(env);
  return env.Undefined();
}


}  // namespace binding