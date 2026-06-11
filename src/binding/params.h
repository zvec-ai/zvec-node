#pragma once


#include <napi.h>
#include <zvec/db/index_params.h>
#include <zvec/db/options.h>
#include <zvec/db/query.h>
#include <zvec/db/status.h>


namespace binding {


/*** Index Parameters  ***/
zvec::Result<zvec::IndexParams::Ptr> ParseIndexParams(const Napi::Value &value);

Napi::Object CreateIndexParams(Napi::Env env, zvec::IndexParams::Ptr params);

zvec::Result<zvec::FlatIndexParams::OPtr> ParseFlatIndexParams(
    const Napi::Object &obj);

Napi::Object CreateFlatIndexParams(Napi::Env env,
                                   zvec::IndexParams::Ptr params);

zvec::Result<zvec::HnswIndexParams::OPtr> ParseHnswIndexParams(
    const Napi::Object &obj);

Napi::Object CreateHnswIndexParams(Napi::Env env,
                                   zvec::IndexParams::Ptr params);

zvec::Result<zvec::HnswRabitqIndexParams::OPtr> ParseHnswRabitqIndexParams(
    const Napi::Object &obj);

Napi::Object CreateHnswRabitqIndexParams(Napi::Env env,
                                         zvec::IndexParams::Ptr params);

zvec::Result<zvec::IVFIndexParams::OPtr> ParseIVFIndexParams(
    const Napi::Object &obj);

Napi::Object CreateIVFIndexParams(Napi::Env env, zvec::IndexParams::Ptr params);

zvec::Result<zvec::DiskAnnIndexParams::OPtr> ParseDiskAnnIndexParams(
    const Napi::Object &obj);

Napi::Object CreateDiskAnnIndexParams(Napi::Env env,
                                      zvec::IndexParams::Ptr params);

zvec::Result<zvec::InvertIndexParams::OPtr> ParseInvertIndexParams(
    const Napi::Object &obj);

Napi::Object CreateInvertIndexParams(Napi::Env env,
                                     zvec::IndexParams::Ptr params);

zvec::Result<std::shared_ptr<zvec::FtsIndexParams>> ParseFtsIndexParams(
    const Napi::Object &obj);

Napi::Object CreateFtsIndexParams(Napi::Env env, zvec::IndexParams::Ptr params);
/*** Index Parameters  ***/


/*** Query Parameters  ***/
zvec::Result<zvec::SearchQuery> ParseSearchQuery(
    const Napi::Value &value, zvec::CollectionSchema::Ptr schema);

zvec::Result<zvec::MultiQuery> ParseMultiQuery(
    const Napi::Value &value, zvec::CollectionSchema::Ptr schema);

zvec::Result<zvec::QueryParams::Ptr> ParseQueryParams(const Napi::Value &value);

zvec::Result<zvec::FlatQueryParams::Ptr> ParseFlatQueryParams(
    const Napi::Object &obj);

zvec::Result<zvec::HnswQueryParams::Ptr> ParseHnswQueryParams(
    const Napi::Object &obj);

zvec::Result<zvec::HnswRabitqQueryParams::Ptr> ParseHnswRabitqQueryParams(
    const Napi::Object &obj);

zvec::Result<zvec::IVFQueryParams::Ptr> ParseIVFQueryParams(
    const Napi::Object &obj);

zvec::Result<zvec::DiskAnnQueryParams::Ptr> ParseDiskAnnQueryParams(
    const Napi::Object &obj);

zvec::Result<zvec::FtsQueryParams::Ptr> ParseFtsQueryParams(
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
