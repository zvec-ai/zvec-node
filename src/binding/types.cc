#include "types.h"


namespace binding {


Napi::Object CreateStatusObject(Napi::Env env, zvec::Status status) {
  Napi::Object obj = Napi::Object::New(env);
  if (status.ok()) {
    obj.Set("ok", Napi::Boolean::New(env, true));
    obj.Set("code", "ZVEC_OK");
    obj.Set("message", "");
  } else {
    obj.Set("ok", Napi::Boolean::New(env, false));
    switch (status.code()) {
      case zvec::StatusCode::NOT_FOUND:
        obj.Set("code", "ZVEC_NOT_FOUND");
        break;
      case zvec::StatusCode::ALREADY_EXISTS:
        obj.Set("code", "ZVEC_ALREADY_EXISTS");
        break;
      case zvec::StatusCode::INVALID_ARGUMENT:
        obj.Set("code", "ZVEC_INVALID_ARGUMENT");
        break;
      case zvec::StatusCode::PERMISSION_DENIED:
        obj.Set("code", "ZVEC_PERMISSION_DENIED");
        break;
      case zvec::StatusCode::FAILED_PRECONDITION:
        obj.Set("code", "ZVEC_FAILED_PRECONDITION");
        break;
      case zvec::StatusCode::RESOURCE_EXHAUSTED:
        obj.Set("code", "ZVEC_RESOURCE_EXHAUSTED");
        break;
      case zvec::StatusCode::UNAVAILABLE:
        obj.Set("code", "ZVEC_UNAVAILABLE");
        break;
      case zvec::StatusCode::INTERNAL_ERROR:
        obj.Set("code", "ZVEC_INTERNAL_ERROR");
        break;
      case zvec::StatusCode::NOT_SUPPORTED:
        obj.Set("code", "ZVEC_NOT_SUPPORTED");
        break;
      case zvec::StatusCode::UNKNOWN:
        obj.Set("code", "ZVEC_UNKNOWN");
        break;
      default:
        obj.Set("code", "ZVEC_INVALID_STATUS_CODE");
        break;
    }
    obj.Set("message", status.message());
  }
  obj.Freeze();
  return obj;
}


Napi::Object CreateDataTypeObject(Napi::Env env) {
  Napi::Object obj = Napi::Object::New(env);
  // obj.Set("UNDEFINED", static_cast<uint32_t>(zvec::DataType::UNDEFINED));

  // obj.Set("BINARY", static_cast<uint32_t>(zvec::DataType::BINARY));
  obj.Set("STRING", static_cast<uint32_t>(zvec::DataType::STRING));
  obj.Set("BOOL", static_cast<uint32_t>(zvec::DataType::BOOL));
  obj.Set("INT32", static_cast<uint32_t>(zvec::DataType::INT32));
  obj.Set("INT64", static_cast<uint32_t>(zvec::DataType::INT64));
  obj.Set("UINT32", static_cast<uint32_t>(zvec::DataType::UINT32));
  obj.Set("UINT64", static_cast<uint32_t>(zvec::DataType::UINT64));
  obj.Set("FLOAT", static_cast<uint32_t>(zvec::DataType::FLOAT));
  obj.Set("DOUBLE", static_cast<uint32_t>(zvec::DataType::DOUBLE));

  // obj.Set("VECTOR_BINARY32",
  //         static_cast<uint32_t>(zvec::DataType::VECTOR_BINARY32));
  // obj.Set("VECTOR_BINARY64",
  //         static_cast<uint32_t>(zvec::DataType::VECTOR_BINARY64));
  obj.Set("VECTOR_FP16", static_cast<uint32_t>(zvec::DataType::VECTOR_FP16));
  obj.Set("VECTOR_FP32", static_cast<uint32_t>(zvec::DataType::VECTOR_FP32));
  obj.Set("VECTOR_FP64", static_cast<uint32_t>(zvec::DataType::VECTOR_FP64));
  // obj.Set("VECTOR_INT4", static_cast<uint32_t>(zvec::DataType::VECTOR_INT4));
  obj.Set("VECTOR_INT8", static_cast<uint32_t>(zvec::DataType::VECTOR_INT8));
  // obj.Set("VECTOR_INT16",
  // static_cast<uint32_t>(zvec::DataType::VECTOR_INT16));
  obj.Set("SPARSE_VECTOR_FP16",
          static_cast<uint32_t>(zvec::DataType::SPARSE_VECTOR_FP16));
  obj.Set("SPARSE_VECTOR_FP32",
          static_cast<uint32_t>(zvec::DataType::SPARSE_VECTOR_FP32));

  // obj.Set("ARRAY_BINARY",
  // static_cast<uint32_t>(zvec::DataType::ARRAY_BINARY));
  obj.Set("ARRAY_STRING", static_cast<uint32_t>(zvec::DataType::ARRAY_STRING));
  obj.Set("ARRAY_BOOL", static_cast<uint32_t>(zvec::DataType::ARRAY_BOOL));
  obj.Set("ARRAY_INT32", static_cast<uint32_t>(zvec::DataType::ARRAY_INT32));
  obj.Set("ARRAY_INT64", static_cast<uint32_t>(zvec::DataType::ARRAY_INT64));
  obj.Set("ARRAY_UINT32", static_cast<uint32_t>(zvec::DataType::ARRAY_UINT32));
  obj.Set("ARRAY_UINT64", static_cast<uint32_t>(zvec::DataType::ARRAY_UINT64));
  obj.Set("ARRAY_FLOAT", static_cast<uint32_t>(zvec::DataType::ARRAY_FLOAT));
  obj.Set("ARRAY_DOUBLE", static_cast<uint32_t>(zvec::DataType::ARRAY_DOUBLE));

  obj.Freeze();
  return obj;
}


zvec::Result<zvec::DataType> ParseDataType(const Napi::Value &value) {
  if (!value.IsNumber()) {
    return tl::make_unexpected(
        zvec::Status::InvalidArgument("Expected a number for DataType"));
  }

  auto raw = value.As<Napi::Number>().Uint32Value();
  switch (static_cast<zvec::DataType>(raw)) {
    // case zvec::DataType::UNDEFINED:
    // case zvec::DataType::BINARY:
    case zvec::DataType::STRING:
    case zvec::DataType::BOOL:
    case zvec::DataType::INT32:
    case zvec::DataType::INT64:
    case zvec::DataType::UINT32:
    case zvec::DataType::UINT64:
    case zvec::DataType::FLOAT:
    case zvec::DataType::DOUBLE:
    // case zvec::DataType::VECTOR_BINARY32:
    // case zvec::DataType::VECTOR_BINARY64:
    case zvec::DataType::VECTOR_FP16:
    case zvec::DataType::VECTOR_FP32:
    case zvec::DataType::VECTOR_FP64:
    // case zvec::DataType::VECTOR_INT4:
    case zvec::DataType::VECTOR_INT8:
    // case zvec::DataType::VECTOR_INT16:
    case zvec::DataType::SPARSE_VECTOR_FP16:
    case zvec::DataType::SPARSE_VECTOR_FP32:
    // case zvec::DataType::ARRAY_BINARY:
    case zvec::DataType::ARRAY_STRING:
    case zvec::DataType::ARRAY_BOOL:
    case zvec::DataType::ARRAY_INT32:
    case zvec::DataType::ARRAY_INT64:
    case zvec::DataType::ARRAY_UINT32:
    case zvec::DataType::ARRAY_UINT64:
    case zvec::DataType::ARRAY_FLOAT:
    case zvec::DataType::ARRAY_DOUBLE:
      return static_cast<zvec::DataType>(raw);
    default:
      return tl::make_unexpected(zvec::Status::InvalidArgument(
          "Unknown DataType value: " + std::to_string(raw)));
  }
}


std::string DataTypeToString(zvec::DataType type) {
  switch (type) {
    case zvec::DataType::UNDEFINED:
      return "UNDEFINED";
    case zvec::DataType::BINARY:
      return "BINARY";
    case zvec::DataType::STRING:
      return "STRING";
    case zvec::DataType::BOOL:
      return "BOOL";
    case zvec::DataType::INT32:
      return "INT32";
    case zvec::DataType::INT64:
      return "INT64";
    case zvec::DataType::UINT32:
      return "UINT32";
    case zvec::DataType::UINT64:
      return "UINT64";
    case zvec::DataType::FLOAT:
      return "FLOAT";
    case zvec::DataType::DOUBLE:
      return "DOUBLE";
    case zvec::DataType::VECTOR_BINARY32:
      return "VECTOR_BINARY32";
    case zvec::DataType::VECTOR_BINARY64:
      return "VECTOR_BINARY64";
    case zvec::DataType::VECTOR_FP16:
      return "VECTOR_FP16";
    case zvec::DataType::VECTOR_FP32:
      return "VECTOR_FP32";
    case zvec::DataType::VECTOR_FP64:
      return "VECTOR_FP64";
    case zvec::DataType::VECTOR_INT4:
      return "VECTOR_INT4";
    case zvec::DataType::VECTOR_INT8:
      return "VECTOR_INT8";
    case zvec::DataType::VECTOR_INT16:
      return "VECTOR_INT16";
    case zvec::DataType::SPARSE_VECTOR_FP16:
      return "SPARSE_VECTOR_FP16";
    case zvec::DataType::SPARSE_VECTOR_FP32:
      return "SPARSE_VECTOR_FP32";
    case zvec::DataType::ARRAY_BINARY:
      return "ARRAY_BINARY";
    case zvec::DataType::ARRAY_STRING:
      return "ARRAY_STRING";
    case zvec::DataType::ARRAY_BOOL:
      return "ARRAY_BOOL";
    case zvec::DataType::ARRAY_INT32:
      return "ARRAY_INT32";
    case zvec::DataType::ARRAY_INT64:
      return "ARRAY_INT64";
    case zvec::DataType::ARRAY_UINT32:
      return "ARRAY_UINT32";
    case zvec::DataType::ARRAY_UINT64:
      return "ARRAY_UINT64";
    case zvec::DataType::ARRAY_FLOAT:
      return "ARRAY_FLOAT";
    case zvec::DataType::ARRAY_DOUBLE:
      return "ARRAY_DOUBLE";
    default:
      return "UNKNOWN";
  }
}


Napi::Object CreateIndexTypeObject(Napi::Env env) {
  Napi::Object obj = Napi::Object::New(env);
  obj.Set("UNDEFINED", static_cast<uint32_t>(zvec::IndexType::UNDEFINED));
  obj.Set("HNSW", static_cast<uint32_t>(zvec::IndexType::HNSW));
  obj.Set("IVF", static_cast<uint32_t>(zvec::IndexType::IVF));
  obj.Set("FLAT", static_cast<uint32_t>(zvec::IndexType::FLAT));
  obj.Set("INVERT", static_cast<uint32_t>(zvec::IndexType::INVERT));
  obj.Freeze();
  return obj;
}


zvec::Result<zvec::IndexType> ParseIndexType(const Napi::Value &value) {
  if (!value.IsNumber()) {
    return tl::make_unexpected(
        zvec::Status::InvalidArgument("Expected a number for IndexType"));
  }

  auto raw = value.As<Napi::Number>().Uint32Value();
  switch (static_cast<zvec::IndexType>(raw)) {
    case zvec::IndexType::UNDEFINED:
    case zvec::IndexType::HNSW:
    case zvec::IndexType::IVF:
    case zvec::IndexType::FLAT:
    case zvec::IndexType::INVERT:
      return static_cast<zvec::IndexType>(raw);
    default:
      return tl::make_unexpected(zvec::Status::InvalidArgument(
          "Unknown IndexType value: " + std::to_string(raw)));
  }
}


Napi::Object CreateMetricTypeObject(Napi::Env env) {
  Napi::Object obj = Napi::Object::New(env);
  obj.Set("UNDEFINED", static_cast<uint32_t>(zvec::MetricType::UNDEFINED));
  obj.Set("L2", static_cast<uint32_t>(zvec::MetricType::L2));
  obj.Set("IP", static_cast<uint32_t>(zvec::MetricType::IP));
  obj.Set("COSINE", static_cast<uint32_t>(zvec::MetricType::COSINE));
  obj.Freeze();
  return obj;
}


zvec::Result<zvec::MetricType> ParseMetricType(const Napi::Value &value) {
  if (!value.IsNumber()) {
    return tl::make_unexpected(
        zvec::Status::InvalidArgument("Expected a number for MetricType"));
  }

  auto raw = value.As<Napi::Number>().Uint32Value();
  switch (static_cast<zvec::MetricType>(raw)) {
    case zvec::MetricType::UNDEFINED:
    case zvec::MetricType::L2:
    case zvec::MetricType::IP:
    case zvec::MetricType::COSINE:
      return static_cast<zvec::MetricType>(raw);
    default:
      return tl::make_unexpected(zvec::Status::InvalidArgument(
          "Unknown MetricType value: " + std::to_string(raw)));
  }
}


Napi::Object CreateQuantizeTypeObject(Napi::Env env) {
  Napi::Object obj = Napi::Object::New(env);
  obj.Set("UNDEFINED", static_cast<uint32_t>(zvec::QuantizeType::UNDEFINED));
  obj.Set("FP16", static_cast<uint32_t>(zvec::QuantizeType::FP16));
  obj.Set("INT8", static_cast<uint32_t>(zvec::QuantizeType::INT8));
  obj.Set("INT4", static_cast<uint32_t>(zvec::QuantizeType::INT4));
  obj.Freeze();
  return obj;
}


zvec::Result<zvec::QuantizeType> ParseQuantizeType(const Napi::Value &value) {
  if (!value.IsNumber()) {
    return tl::make_unexpected(
        zvec::Status::InvalidArgument("Expected a number for QuantizeType"));
  }

  auto raw = value.As<Napi::Number>().Uint32Value();
  switch (static_cast<zvec::QuantizeType>(raw)) {
    case zvec::QuantizeType::UNDEFINED:
    case zvec::QuantizeType::FP16:
    case zvec::QuantizeType::INT8:
    case zvec::QuantizeType::INT4:
      return static_cast<zvec::QuantizeType>(raw);
    default:
      return tl::make_unexpected(zvec::Status::InvalidArgument(
          "Unknown QuantizeType value: " + std::to_string(raw)));
  }
}


Napi::Object InitTypes(Napi::Env env, Napi::Object exports) {
  exports.Set("DataType", CreateDataTypeObject(env));
  exports.Set("IndexType", CreateIndexTypeObject(env));
  exports.Set("MetricType", CreateMetricTypeObject(env));
  exports.Set("QuantizeType", CreateQuantizeTypeObject(env));
  return exports;
}


}  // namespace binding