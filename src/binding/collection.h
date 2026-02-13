#pragma once


#include <napi.h>
#include <mutex>
#include <shared_mutex>
#include "zvec/db/collection.h"
#include "addon.h"


namespace binding {


Napi::Object CreateAndOpenCollection(const Napi::CallbackInfo &info);

Napi::Object OpenCollection(const Napi::CallbackInfo &info);


class Collection : public Napi::ObjectWrap<Collection> {
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports,
                           AddonConstructors *ctors);

  Collection(const Napi::CallbackInfo &info);

  zvec::Collection::Ptr get_wrapped_collection();

  zvec::CollectionSchema::Ptr get_wrapped_schema();

  void set_wrapped(Napi::Env &env, zvec::Collection::Ptr collection);

  void set_wrapped_collection(zvec::Collection::Ptr collection);

  void set_wrapped_schema(Napi::Env &env);


 private:
  Napi::Value Path(const Napi::CallbackInfo &info);

  Napi::Value Schema(const Napi::CallbackInfo &info);

  Napi::Value Options(const Napi::CallbackInfo &info);

  Napi::Value Stats(const Napi::CallbackInfo &info);

  Napi::Value Insert(const Napi::CallbackInfo &info);

  Napi::Value Upsert(const Napi::CallbackInfo &info);

  Napi::Value Update(const Napi::CallbackInfo &info);

  Napi::Value Delete(const Napi::CallbackInfo &info);

  Napi::Value DeleteByFilter(const Napi::CallbackInfo &info);

  Napi::Value Query(const Napi::CallbackInfo &info);

  Napi::Value Fetch(const Napi::CallbackInfo &info);

  Napi::Value Optimize(const Napi::CallbackInfo &info);

  Napi::Value Close(const Napi::CallbackInfo &info);

  Napi::Value Destroy(const Napi::CallbackInfo &info);

  Napi::Value AddColumn(const Napi::CallbackInfo &info);

  Napi::Value DropColumn(const Napi::CallbackInfo &info);

  Napi::Value AlterColumn(const Napi::CallbackInfo &info);

  Napi::Value CreateIndex(const Napi::CallbackInfo &info);

  Napi::Value DropIndex(const Napi::CallbackInfo &info);


  zvec::Collection::Ptr collection_{nullptr};
  zvec::CollectionSchema::Ptr schema_{nullptr};
  mutable std::shared_mutex schema_lock_;
  mutable std::mutex ddl_lock_{};
};


}  // namespace binding