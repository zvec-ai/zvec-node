#pragma once


#include <napi.h>
#include "zvec/db/doc.h"


namespace binding {


zvec::Result<zvec::Doc> ParseDoc(const Napi::Value &value,
                                 zvec::CollectionSchema::Ptr schema);

zvec::Status ParseVectors(const Napi::Value &value,
                          zvec::CollectionSchema::Ptr &schema, zvec::Doc *doc);

zvec::Status ParseVector(const Napi::Value &value,
                         const std::string &vectorName,
                         zvec::FieldSchema *schema, zvec::Doc *doc);

zvec::Status ParseVectorTypedArray(const Napi::TypedArray &array,
                                   const std::string &vectorName,
                                   zvec::FieldSchema *schema, zvec::Doc *doc);

zvec::Status ParseVectorArray(const Napi::Array &array,
                              const std::string &vectorName,
                              zvec::FieldSchema *schema, zvec::Doc *doc);

zvec::Status ParseVectorObject(const Napi::Object &obj,
                               const std::string &vectorName,
                               zvec::FieldSchema *schema, zvec::Doc *doc);

zvec::Status ParseFields(const Napi::Value &value,
                         zvec::CollectionSchema::Ptr &schema, zvec::Doc *doc);

zvec::Status ParseField(const Napi::Value &value, const std::string &fieldName,
                        zvec::FieldSchema *schema, zvec::Doc *doc);

zvec::Status ParseScalarTypedArray(const Napi::TypedArray &array,
                                   const std::string &fieldName,
                                   zvec::FieldSchema *schema, zvec::Doc *doc);

zvec::Status ParseScalarArray(const Napi::Array &array,
                              const std::string &fieldName,
                              zvec::FieldSchema *schema, zvec::Doc *doc);

Napi::Object CreateDoc(Napi::Env env, zvec::CollectionSchema::Ptr schema,
                       zvec::Doc::Ptr doc);

Napi::Object CreateVectors(Napi::Env &env, zvec::CollectionSchema::Ptr &schema,
                           zvec::Doc::Ptr &doc);

Napi::Array CreateVectorArray(Napi::Env &env, const std::string &vectorName,
                              zvec::FieldSchema *schema, zvec::Doc::Ptr &doc);

Napi::Object CreateVectorObject(Napi::Env &env, const std::string &vectorName,
                                zvec::FieldSchema *schema, zvec::Doc::Ptr &doc);

Napi::Object CreateFields(Napi::Env &env, zvec::CollectionSchema::Ptr &schema,
                          zvec::Doc::Ptr &doc);

Napi::Array CreateScalarArray(Napi::Env &env, const std::string &fieldName,
                              zvec::FieldSchema *schema, zvec::Doc::Ptr &doc);


}  // namespace binding