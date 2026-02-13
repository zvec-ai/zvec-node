#pragma once


#include <napi.h>
#include "zvec/db/status.h"
#include "zvec/db/type.h"


namespace binding {


inline void ThrowIfNotOk(const Napi::Env &env, const zvec::Status &status) {
  if (status.ok()) return;

  Napi::Error error;
  switch (status.code()) {
    case zvec::StatusCode::NOT_FOUND:
      error = Napi::Error::New(env, status.message());
      error.Value()["name"] = "NotFoundError";
      error.Value()["code"] = "ZVEC_NOT_FOUND";
      break;
    case zvec::StatusCode::ALREADY_EXISTS:
      error = Napi::Error::New(env, status.message());
      error.Value()["name"] = "AlreadyExistsError";
      error.Value()["code"] = "ZVEC_ALREADY_EXISTS";
      break;
    case zvec::StatusCode::INVALID_ARGUMENT:
      error = Napi::TypeError::New(env, status.message());
      error.Value()["name"] = "InvalidArgumentError";
      error.Value()["code"] = "ZVEC_INVALID_ARGUMENT";
      break;
    case zvec::StatusCode::PERMISSION_DENIED:
      error = Napi::Error::New(env, status.message());
      error.Value()["name"] = "PermissionDeniedError";
      error.Value()["code"] = "ZVEC_PERMISSION_DENIED";
      break;
    case zvec::StatusCode::FAILED_PRECONDITION:
      error = Napi::Error::New(env, status.message());
      error.Value()["name"] = "FailedPreconditionError";
      error.Value()["code"] = "ZVEC_FAILED_PRECONDITION";
      break;
    case zvec::StatusCode::RESOURCE_EXHAUSTED:
      error = Napi::Error::New(env, status.message());
      error.Value()["name"] = "ResourceExhaustedError";
      error.Value()["code"] = "ZVEC_RESOURCE_EXHAUSTED";
      break;
    case zvec::StatusCode::UNAVAILABLE:
      error = Napi::Error::New(env, status.message());
      error.Value()["name"] = "UnavailableError";
      error.Value()["code"] = "ZVEC_UNAVAILABLE";
      break;
    case zvec::StatusCode::INTERNAL_ERROR:
      error = Napi::Error::New(env, status.message());
      error.Value()["name"] = "InternalError";
      error.Value()["code"] = "ZVEC_INTERNAL_ERROR";
      break;
    case zvec::StatusCode::NOT_SUPPORTED:
      error = Napi::Error::New(env, status.message());
      error.Value()["name"] = "NotSupportedError";
      error.Value()["code"] = "ZVEC_NOT_SUPPORTED";
      break;
    case zvec::StatusCode::UNKNOWN:
      error = Napi::Error::New(env, status.message());
      error.Value()["name"] = "UnknownError";
      error.Value()["code"] = "ZVEC_UNKNOWN";
      break;
    default:
      error = Napi::Error::New(env, status.message());
      error.Value()["name"] = "InvalidStatusCodeError";
      error.Value()["code"] = "ZVEC_INVALID_STATUS_CODE";
      break;
  }
  error.ThrowAsJavaScriptException();
}


Napi::Object CreateStatusObject(Napi::Env env, zvec::Status status);

Napi::Object CreateDataTypeObject(Napi::Env env);

zvec::Result<zvec::DataType> ParseDataType(const Napi::Value &value);

std::string DataTypeToString(zvec::DataType type);

Napi::Object CreateIndexTypeObject(Napi::Env env);

zvec::Result<zvec::IndexType> ParseIndexType(const Napi::Value &value);

Napi::Object CreateMetricTypeObject(Napi::Env env);

zvec::Result<zvec::MetricType> ParseMetricType(const Napi::Value &value);

Napi::Object CreateQuantizeTypeObject(Napi::Env env);

zvec::Result<zvec::QuantizeType> ParseQuantizeType(const Napi::Value &value);

Napi::Object InitTypes(Napi::Env env, Napi::Object exports);


}  // namespace binding