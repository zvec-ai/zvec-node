#pragma once


#include <napi.h>
#include "zvec/db/doc.h"
#include "zvec/db/index_params.h"
#include "zvec/db/options.h"
#include "zvec/db/status.h"


namespace binding {


/*** Index Parameters  ***/
zvec::Result<zvec::IndexParams::Ptr> ParseIndexParams(const Napi::Value &value);

Napi::Object CreateIndexParams(Napi::Env env, zvec::IndexParams::Ptr params);

zvec::Result<zvec::FlatIndexParams::Ptr> ParseFlatIndexParams(
    const Napi::Object &obj);

Napi::Object CreateFlatIndexParams(Napi::Env env,
                                   zvec::IndexParams::Ptr params);

zvec::Result<zvec::HnswIndexParams::Ptr> ParseHnswIndexParams(
    const Napi::Object &obj);

Napi::Object CreateHnswIndexParams(Napi::Env env,
                                   zvec::IndexParams::Ptr params);

zvec::Result<zvec::IVFIndexParams::Ptr> ParseIVFIndexParams(
    const Napi::Object &obj);

Napi::Object CreateIVFIndexParams(Napi::Env env, zvec::IndexParams::Ptr params);

zvec::Result<zvec::InvertIndexParams::Ptr> ParseInvertIndexParams(
    const Napi::Object &obj);

Napi::Object CreateInvertIndexParams(Napi::Env env,
                                     zvec::IndexParams::Ptr params);
/*** Index Parameters  ***/


/*** Query Parameters  ***/
zvec::Result<zvec::VectorQuery> ParseVectorQuery(
    const Napi::Value &value, zvec::CollectionSchema::Ptr schema);

zvec::Status ParseVectorToString(const Napi::Value &value,
                                 zvec::FieldSchema *schema,
                                 zvec::VectorQuery *query);

zvec::Status ParseVectorToMap(const Napi::Value &value,
                              zvec::FieldSchema *schema,
                              zvec::VectorQuery *query);

zvec::Result<zvec::QueryParams::Ptr> ParseQueryParams(const Napi::Value &value);

zvec::Result<zvec::FlatQueryParams::Ptr> ParseFlatQueryParams(
    const Napi::Object &obj);

zvec::Result<zvec::HnswQueryParams::Ptr> ParseHnswQueryParams(
    const Napi::Object &obj);

zvec::Result<zvec::IVFQueryParams::Ptr> ParseIVFQueryParams(
    const Napi::Object &obj);
/*** Query Parameters  ***/


/*** Collection-Level Options  ***/
zvec::Result<zvec::CollectionOptions> ParseCollectionOptions(
    const Napi::Object &obj);

Napi::Object CreateCollectionOptions(Napi::Env env,
                                     zvec::CollectionOptions options);

zvec::Result<zvec::AddColumnOptions> ParseAddColumnOptions(
    const Napi::Object &obj);

zvec::Result<zvec::AlterColumnOptions> ParseAlterColumnOptions(
    const Napi::Object &obj);

zvec::Result<zvec::CreateIndexOptions> ParseCreateIndexOptions(
    const Napi::Object &obj);

zvec::Result<zvec::OptimizeOptions> ParseOptimizeOptions(
    const Napi::Object &obj);
/*** Collection-Level Options  ***/


}  // namespace binding