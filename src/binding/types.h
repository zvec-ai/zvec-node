#pragma once


#include <napi.h>
#include "zvec/db/status.h"
#include "zvec/db/type.h"


namespace binding {


inline Napi::Error CreateZVecError(const Napi::Env &env,
                                   const zvec::Status &status) {
  auto error = Napi::Error::New(env, status.message());
  auto errorValue = error.Value();
  switch (status.code()) {
    case zvec::StatusCode::NOT_FOUND:
      errorValue["name"] = "NotFoundError";
      errorValue["code"] = "ZVEC_NOT_FOUND";
      break;
    case zvec::StatusCode::ALREADY_EXISTS:
      errorValue["name"] = "AlreadyExistsError";
      errorValue["code"] = "ZVEC_ALREADY_EXISTS";
      break;
    case zvec::StatusCode::INVALID_ARGUMENT:
      errorValue["name"] = "InvalidArgumentError";
      errorValue["code"] = "ZVEC_INVALID_ARGUMENT";
      break;
    case zvec::StatusCode::PERMISSION_DENIED:
      errorValue["name"] = "PermissionDeniedError";
      errorValue["code"] = "ZVEC_PERMISSION_DENIED";
      break;
    case zvec::StatusCode::FAILED_PRECONDITION:
      errorValue["name"] = "FailedPreconditionError";
      errorValue["code"] = "ZVEC_FAILED_PRECONDITION";
      break;
    case zvec::StatusCode::RESOURCE_EXHAUSTED:
      errorValue["name"] = "ResourceExhaustedError";
      errorValue["code"] = "ZVEC_RESOURCE_EXHAUSTED";
      break;
    case zvec::StatusCode::UNAVAILABLE:
      errorValue["name"] = "UnavailableError";
      errorValue["code"] = "ZVEC_UNAVAILABLE";
      break;
    case zvec::StatusCode::INTERNAL_ERROR:
      errorValue["name"] = "InternalError";
      errorValue["code"] = "ZVEC_INTERNAL_ERROR";
      break;
    case zvec::StatusCode::NOT_SUPPORTED:
      errorValue["name"] = "NotSupportedError";
      errorValue["code"] = "ZVEC_NOT_SUPPORTED";
      break;
    case zvec::StatusCode::UNKNOWN:
      errorValue["name"] = "UnknownError";
      errorValue["code"] = "ZVEC_UNKNOWN";
      break;
    default:
      errorValue["name"] = "InvalidStatusCodeError";
      errorValue["code"] = "ZVEC_INVALID_STATUS_CODE";
      break;
  }
  return error;
}


inline void ThrowIfNotOk(const Napi::Env &env, const zvec::Status &status) {
  if (status.ok()) return;
  CreateZVecError(env, status).ThrowAsJavaScriptException();
}


inline void RejectIfNotOk(const Napi::Env &env, const zvec::Status &status,
                          Napi::Promise::Deferred &deferred) {
  if (status.ok()) return;
  deferred.Reject(CreateZVecError(env, status).Value());
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