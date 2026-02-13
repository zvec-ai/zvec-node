#pragma once


#include <napi.h>
#include "zvec/db/schema.h"
#include "addon.h"


namespace binding {


zvec::Result<zvec::FieldSchema::Ptr> ParseFieldSchema(const Napi::Value &value);

Napi::Object CreateFieldSchema(Napi::Env env, zvec::FieldSchema *field);

zvec::Result<zvec::FieldSchema::Ptr> ParseVectorSchema(
    const Napi::Value &value);

Napi::Object CreateVectorSchema(Napi::Env env, zvec::FieldSchema *field);


class CollectionSchema : public Napi::ObjectWrap<CollectionSchema> {
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports,
                           AddonConstructors *ctors);

  CollectionSchema(const Napi::CallbackInfo &info);

  zvec::CollectionSchema::Ptr get_wrapped();

  void set_wrapped(zvec::CollectionSchema::Ptr schema);


 private:
  Napi::Value Name(const Napi::CallbackInfo &info);

  Napi::Value Field(const Napi::CallbackInfo &info);

  Napi::Value Vector(const Napi::CallbackInfo &info);

  Napi::Value Fields(const Napi::CallbackInfo &info);

  Napi::Value Vectors(const Napi::CallbackInfo &info);

  Napi::Value ToString(const Napi::CallbackInfo &info);


  zvec::CollectionSchema::Ptr collection_schema_{nullptr};
};


}  // namespace binding