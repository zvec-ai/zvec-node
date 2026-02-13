#include "params.h"
#include "types.h"


namespace binding {


zvec::Result<zvec::IndexParams::Ptr> ParseIndexParams(
    const Napi::Value &value) {
  if (!value.IsObject()) {
    return tl::make_unexpected(
        zvec::Status::InvalidArgument("Expected an object for IndexParams"));
  }

  auto obj = value.As<Napi::Object>();

  if (!obj.Has("indexType")) {
    return tl::make_unexpected(zvec::Status::InvalidArgument(
        "Missing required argument 'indexType' in IndexParams"));
  }
  auto parsed_index_type = ParseIndexType(obj.Get("indexType"));
  if (!parsed_index_type) {
    return tl::make_unexpected(parsed_index_type.error());
  }
  auto index_type = parsed_index_type.value();

  switch (index_type) {
    case zvec::IndexType::FLAT:
      return ParseFlatIndexParams(obj);
    case zvec::IndexType::HNSW:
      return ParseHnswIndexParams(obj);
    case zvec::IndexType::IVF:
      return ParseIVFIndexParams(obj);
    case zvec::IndexType::INVERT:
      return ParseInvertIndexParams(obj);
    default:
      uint32_t type_val = static_cast<uint32_t>(index_type);
      return tl::make_unexpected(zvec::Status::InvalidArgument(
          "Invalid index type enum value: " + std::to_string(type_val)));
  }
}


Napi::Object CreateIndexParams(Napi::Env env, zvec::IndexParams::Ptr params) {
  switch (params->type()) {
    case zvec::IndexType::FLAT:
      return CreateFlatIndexParams(env, params);
    case zvec::IndexType::HNSW:
      return CreateHnswIndexParams(env, params);
    case zvec::IndexType::IVF:
      return CreateIVFIndexParams(env, params);
    case zvec::IndexType::INVERT:
      return CreateInvertIndexParams(env, params);
    default:
      return Napi::Object::New(env);
  }
}


zvec::Result<zvec::FlatIndexParams::Ptr> ParseFlatIndexParams(
    const Napi::Object &obj) {
  zvec::MetricType metric_type{zvec::MetricType::IP};
  if (obj.Has("metricType")) {
    auto parsed_metric_type = ParseMetricType(obj.Get("metricType"));
    if (parsed_metric_type) {
      metric_type = parsed_metric_type.value();
    } else {
      return tl::make_unexpected(parsed_metric_type.error());
    }
  }

  zvec::QuantizeType quantize_type{zvec::QuantizeType::UNDEFINED};
  if (obj.Has("quantizeType")) {
    auto parsed_quantize_type = ParseQuantizeType(obj.Get("quantizeType"));
    if (parsed_quantize_type) {
      quantize_type = parsed_quantize_type.value();
    } else {
      return tl::make_unexpected(parsed_quantize_type.error());
    }
  }

  return std::make_shared<zvec::FlatIndexParams>(metric_type, quantize_type);
}


Napi::Object CreateFlatIndexParams(Napi::Env env,
                                   zvec::IndexParams::Ptr params) {
  auto obj = Napi::Object::New(env);
  auto flat_params = std::dynamic_pointer_cast<zvec::FlatIndexParams>(params);
  obj.Set("indexType", static_cast<uint32_t>(flat_params->type()));
  obj.Set("metricType", static_cast<uint32_t>(flat_params->metric_type()));
  obj.Set("quantizeType", static_cast<uint32_t>(flat_params->quantize_type()));
  return obj;
}


zvec::Result<zvec::HnswIndexParams::Ptr> ParseHnswIndexParams(
    const Napi::Object &obj) {
  zvec::MetricType metric_type{zvec::MetricType::IP};
  if (obj.Has("metricType")) {
    auto parsed_metric_type = ParseMetricType(obj.Get("metricType"));
    if (parsed_metric_type) {
      metric_type = parsed_metric_type.value();
    } else {
      return tl::make_unexpected(parsed_metric_type.error());
    }
  }

  int m{50};
  if (obj.Has("m")) {
    if (obj.Get("m").IsNumber()) {
      m = obj.Get("m").As<Napi::Number>();
    } else {
      return tl::make_unexpected(
          zvec::Status::InvalidArgument("Expected a number for 'm'"));
    }
  }

  int ef_construction{500};
  if (obj.Has("efConstruction")) {
    if (obj.Get("efConstruction").IsNumber()) {
      ef_construction = obj.Get("efConstruction").As<Napi::Number>();
    } else {
      return tl::make_unexpected(zvec::Status::InvalidArgument(
          "Expected a number for 'efConstruction'"));
    }
  }

  zvec::QuantizeType quantize_type{zvec::QuantizeType::UNDEFINED};
  if (obj.Has("quantizeType")) {
    auto parsed_quantize_type = ParseQuantizeType(obj.Get("quantizeType"));
    if (parsed_quantize_type) {
      quantize_type = parsed_quantize_type.value();
    } else {
      return tl::make_unexpected(parsed_quantize_type.error());
    }
  }

  return std::make_shared<zvec::HnswIndexParams>(
      metric_type, m, ef_construction, quantize_type);
}


Napi::Object CreateHnswIndexParams(Napi::Env env,
                                   zvec::IndexParams::Ptr params) {
  auto obj = Napi::Object::New(env);
  auto hnsw_params = std::dynamic_pointer_cast<zvec::HnswIndexParams>(params);
  obj.Set("indexType", static_cast<uint32_t>(hnsw_params->type()));
  obj.Set("metricType", static_cast<uint32_t>(hnsw_params->metric_type()));
  obj.Set("m", hnsw_params->m());
  obj.Set("efConstruction", hnsw_params->ef_construction());
  obj.Set("quantizeType", static_cast<uint32_t>(hnsw_params->quantize_type()));
  return obj;
}


zvec::Result<zvec::IVFIndexParams::Ptr> ParseIVFIndexParams(
    const Napi::Object &obj) {
  zvec::MetricType metric_type{zvec::MetricType::IP};
  if (obj.Has("metricType")) {
    auto parsed_metric_type = ParseMetricType(obj.Get("metricType"));
    if (parsed_metric_type) {
      metric_type = parsed_metric_type.value();
    } else {
      return tl::make_unexpected(parsed_metric_type.error());
    }
  }

  int n_list{10};
  if (obj.Has("nList")) {
    if (obj.Get("nList").IsNumber()) {
      n_list = obj.Get("nList").As<Napi::Number>();
    } else {
      return tl::make_unexpected(
          zvec::Status::InvalidArgument("Expected a number for 'nList'"));
    }
  }

  int n_iters{10};
  if (obj.Has("nIters")) {
    if (obj.Get("nIters").IsNumber()) {
      n_iters = obj.Get("nIters").As<Napi::Number>();
    } else {
      return tl::make_unexpected(
          zvec::Status::InvalidArgument("Expected a number for 'nIters'"));
    }
  }

  bool use_soar{false};

  zvec::QuantizeType quantize_type{zvec::QuantizeType::UNDEFINED};
  if (obj.Has("quantizeType")) {
    auto parsed_quantize_type = ParseQuantizeType(obj.Get("quantizeType"));
    if (parsed_quantize_type) {
      quantize_type = parsed_quantize_type.value();
    } else {
      return tl::make_unexpected(parsed_quantize_type.error());
    }
  }

  return std::make_shared<zvec::IVFIndexParams>(metric_type, n_list, n_iters,
                                                use_soar, quantize_type);
}


Napi::Object CreateIVFIndexParams(Napi::Env env,
                                  zvec::IndexParams::Ptr params) {
  auto obj = Napi::Object::New(env);
  auto ivf_params = std::dynamic_pointer_cast<zvec::IVFIndexParams>(params);
  obj.Set("indexType", static_cast<uint32_t>(ivf_params->type()));
  obj.Set("metricType", static_cast<uint32_t>(ivf_params->metric_type()));
  obj.Set("nList", ivf_params->n_list());
  obj.Set("nIters", ivf_params->n_iters());
  obj.Set("quantizeType", static_cast<uint32_t>(ivf_params->quantize_type()));
  return obj;
}


zvec::Result<zvec::InvertIndexParams::Ptr> ParseInvertIndexParams(
    const Napi::Object &obj) {
  bool enable_range_optimization{true};
  if (obj.Has("enableRangeOptimization")) {
    if (!obj.Get("enableRangeOptimization").IsBoolean()) {
      return tl::make_unexpected(zvec::Status::InvalidArgument(
          "Expected a boolean for 'enableRangeOptimization'"));
    }
    enable_range_optimization =
        obj.Get("enableRangeOptimization").As<Napi::Boolean>();
  }

  bool enable_extended_wildcard{false};
  if (obj.Has("enableExtendedWildcard")) {
    if (!obj.Get("enableExtendedWildcard").IsBoolean()) {
      return tl::make_unexpected(zvec::Status::InvalidArgument(
          "Expected a boolean for 'enableExtendedWildcard'"));
    }
    enable_extended_wildcard =
        obj.Get("enableExtendedWildcard").As<Napi::Boolean>();
  }

  return std::make_shared<zvec::InvertIndexParams>(enable_range_optimization,
                                                   enable_extended_wildcard);
}


Napi::Object CreateInvertIndexParams(Napi::Env env,
                                     zvec::IndexParams::Ptr params) {
  auto obj = Napi::Object::New(env);
  auto invert_params =
      std::dynamic_pointer_cast<zvec::InvertIndexParams>(params);
  obj.Set("indexType", static_cast<uint32_t>(invert_params->type()));
  obj.Set("enableRangeOptimization",
          invert_params->enable_range_optimization());
  obj.Set("enableExtendedWildcard", invert_params->enable_extended_wildcard());
  return obj;
}


zvec::Result<zvec::VectorQuery> ParseVectorQuery(
    const Napi::Value &value, zvec::CollectionSchema::Ptr schema) {
  if (!value.IsObject()) {
    return tl::make_unexpected(
        zvec::Status::InvalidArgument("Expected an object for Query"));
  }

  zvec::VectorQuery query{};
  auto obj = value.As<Napi::Object>();
  zvec::FieldSchema *field_schema{nullptr};

  if (obj.Has("fieldName")) {
    if (obj.Get("fieldName").IsString()) {
      query.field_name_ = obj.Get("fieldName").As<Napi::String>().Utf8Value();
      field_schema = schema->get_field(query.field_name_);
      if (!field_schema) {
        return tl::make_unexpected(zvec::Status::InvalidArgument(
            "'", query.field_name_, "' not found in collection schema"));
      }
    } else {
      return tl::make_unexpected(zvec::Status::InvalidArgument(
          "Expected a string for 'fieldName' in Query"));
    }
  }

  if (obj.Has("topk")) {
    if (obj.Get("topk").IsNumber()) {
      query.topk_ = obj.Get("topk").As<Napi::Number>().Int32Value();
    } else {
      return tl::make_unexpected(
          zvec::Status::InvalidArgument("Expected a number for 'topk'"));
    }
  }
  if (obj.Has("vector")) {
    if (!field_schema) {
      return tl::make_unexpected(zvec::Status::InvalidArgument(
          "A vector name must be provided when performing vector queries"));
    }
    auto vector = obj.Get("vector");
    if (field_schema->is_dense_vector()) {
      if (auto s = ParseVectorToString(vector, field_schema, &query); !s.ok()) {
        return tl::make_unexpected(s);
      }
    } else if (field_schema->is_sparse_vector()) {
      if (auto s = ParseVectorToMap(vector, field_schema, &query); !s.ok()) {
        return tl::make_unexpected(s);
      }
    } else {
      return tl::make_unexpected(
          zvec::Status::InvalidArgument("Field '", query.field_name_,
                                        "' is not a vector field and cannot be "
                                        "queried using vector operations."));
    }
    if (obj.Has("params")) {
      auto parsed_params = ParseQueryParams(obj.Get("params"));
      if (parsed_params) {
        if (parsed_params.value()->type() == field_schema->index_type()) {
          query.query_params_ = parsed_params.value();
        } else {
          return tl::make_unexpected(zvec::Status::InvalidArgument(
              "Unmatched queryParams type in Query"));
        }
      } else {
        return tl::make_unexpected(parsed_params.error());
      }
    }
  } else {
    if (field_schema) {
      return tl::make_unexpected(zvec::Status::InvalidArgument(
          "A vector must be provided when performing vector queries"));
    }
  }
  if (obj.Has("filter")) {
    if (obj.Get("filter").IsString()) {
      query.filter_ = obj.Get("filter").As<Napi::String>().Utf8Value();
    } else {
      return tl::make_unexpected(
          zvec::Status::InvalidArgument("Expected a string for 'filter'"));
    }
  }
  if (obj.Has("includeVector")) {
    if (obj.Get("includeVector").IsBoolean()) {
      query.include_vector_ = obj.Get("includeVector").As<Napi::Boolean>();
    } else {
      return tl::make_unexpected(zvec::Status::InvalidArgument(
          "Expected a boolean for 'includeVector'"));
    }
  }
  if (obj.Has("outputFields")) {
    if (obj.Get("outputFields").IsArray()) {
      std::vector<std::string> output_fields{};
      Napi::Array array = obj.Get("outputFields").As<Napi::Array>();
      for (uint32_t i = 0; i < array.Length(); i++) {
        if (array.Get(i).IsString()) {
          output_fields.push_back(array.Get(i).As<Napi::String>().Utf8Value());
        } else {
          return tl::make_unexpected(zvec::Status::InvalidArgument(
              "Expected an array of strings for 'outputFields'"));
        }
      }
      query.output_fields_ = output_fields;
    } else {
      return tl::make_unexpected(zvec::Status::InvalidArgument(
          "Expected an array of strings for 'outputFields'"));
    }
  }

  return query;
}


zvec::Status ParseVectorToString(const Napi::Value &value,
                                 zvec::FieldSchema *schema,
                                 zvec::VectorQuery *query) {
  if (value.IsTypedArray()) {
    auto array = value.As<Napi::TypedArray>();
    auto ta_type = array.TypedArrayType();
    size_t ta_length = array.ElementLength();
    switch (schema->data_type()) {
      case zvec::DataType::VECTOR_FP16: {
        if (ta_type != napi_float32_array) {
          return zvec::Status::InvalidArgument("Expected dense vector[",
                                               schema->name(),
                                               "] to be a Float32Array");
        }
        Napi::Float32Array float32Array = array.As<Napi::Float32Array>();
        std::string buf;
        buf.resize(ta_length * sizeof(uint16_t));
        for (size_t i = 0; i < ta_length; i++) {
          zvec::ailego::Float16 val = float32Array[i];
          std::memcpy(buf.data() + (i * sizeof(uint16_t)), &val,
                      sizeof(uint16_t));
        }
        query->query_vector_ = buf;
        break;
      }
      case zvec::DataType::VECTOR_FP32: {
        if (ta_type != napi_float32_array) {
          return zvec::Status::InvalidArgument("Expected dense vector[",
                                               schema->name(),
                                               "] to be a Float32Array");
        }
        Napi::Float32Array float32Array = array.As<Napi::Float32Array>();
        std::string buf;
        buf.resize(ta_length * sizeof(float));
        std::memcpy(buf.data(), float32Array.Data(), ta_length * sizeof(float));
        query->query_vector_ = buf;
        break;
      }
      case zvec::DataType::VECTOR_INT8: {
        if (ta_type != napi_int8_array) {
          return zvec::Status::InvalidArgument(
              "Expected dense vector[", schema->name(), "] to be an Int8Array");
        }
        Napi::Int8Array int8Array = array.As<Napi::Int8Array>();
        std::string buf;
        buf.resize(ta_length * sizeof(int8_t));
        std::memcpy(buf.data(), int8Array.Data(), ta_length * sizeof(int8_t));
        query->query_vector_ = buf;
        break;
      }
      default: {
        return zvec::Status::InternalError(
            "Unknown data type: ", static_cast<uint32_t>(schema->data_type()));
      }
    }
  } else if (value.IsArray()) {
    auto array = value.As<Napi::Array>();
    uint32_t length = array.Length();
    switch (schema->data_type()) {
      case zvec::DataType::VECTOR_FP16: {
        std::string buf;
        buf.resize(length * sizeof(uint16_t));
        for (uint32_t i = 0; i < length; i++) {
          auto value = array.Get(i);
          if (value.IsNumber()) {
            zvec::ailego::Float16 val = value.As<Napi::Number>().FloatValue();
            std::memcpy(buf.data() + (i * sizeof(uint16_t)), &val,
                        sizeof(uint16_t));
          } else {
            return zvec::Status::InvalidArgument(
                "All elements in the vector array for '", schema->name(),
                "' must be numbers");
          }
        }
        query->query_vector_ = buf;
        break;
      }
      case zvec::DataType::VECTOR_FP32: {
        std::string buf;
        buf.resize(length * sizeof(float));
        for (uint32_t i = 0; i < length; i++) {
          auto value = array.Get(i);
          if (value.IsNumber()) {
            float val = value.As<Napi::Number>().FloatValue();
            std::memcpy(buf.data() + (i * sizeof(float)), &val, sizeof(float));
          } else {
            return zvec::Status::InvalidArgument(
                "All elements in the vector array for '", schema->name(),
                "' must be numbers");
          }
        }
        query->query_vector_ = buf;
        break;
      }
      case zvec::DataType::VECTOR_INT8: {
        std::string buf;
        buf.resize(length * sizeof(int8_t));
        for (uint32_t i = 0; i < length; i++) {
          auto value = array.Get(i);
          if (value.IsNumber()) {
            int8_t val = value.As<Napi::Number>().Int32Value();
            std::memcpy(buf.data() + (i * sizeof(int8_t)), &val,
                        sizeof(int8_t));
          } else {
            return zvec::Status::InvalidArgument(
                "All elements in the vector array for '", schema->name(),
                "' must be numbers");
          }
        }
        query->query_vector_ = buf;
        break;
      }
      default: {
        return zvec::Status::InternalError(
            "Unknown data type: ", static_cast<uint32_t>(schema->data_type()));
      }
    }
  } else {
    return zvec::Status::InvalidArgument("Expected dense vector[",
                                         schema->name(),
                                         "] to be an array of numbers");
  }
  return zvec::Status::OK();
}


zvec::Status ParseVectorToMap(const Napi::Value &value,
                              zvec::FieldSchema *schema,
                              zvec::VectorQuery *query) {
  if (!value.IsObject()) {
    return zvec::Status::InvalidArgument("Expected sparse vector[",
                                         schema->name(), "] to be an object");
  }
  auto obj = value.As<Napi::Object>();
  auto stringIndices = obj.GetPropertyNames();
  uint32_t length = stringIndices.Length();
  std::string indices{};
  indices.resize(length * sizeof(uint32_t));
  std::string values{};
  switch (schema->data_type()) {
    case zvec::DataType::SPARSE_VECTOR_FP32: {
      values.resize(length * sizeof(float));
      auto *indices_ptr = reinterpret_cast<uint32_t *>(indices.data());
      auto *values_ptr = reinterpret_cast<float *>(values.data());
      for (uint32_t i = 0; i < length; ++i) {
        auto stringIndex = stringIndices.Get(i).As<Napi::String>().Utf8Value();
        uint32_t index;
        try {
          unsigned long parsed_ulong = std::stoul(stringIndex);
          if (parsed_ulong > std::numeric_limits<uint32_t>::max()) {
            return zvec::Status::InvalidArgument(
                "Index '", stringIndex, "' for '", schema->name(),
                "' is out of range for uint32_t.");
          }
          index = static_cast<uint32_t>(parsed_ulong);
        } catch (const std::exception &) {
          return zvec::Status::InvalidArgument(
              "Index '", stringIndex, "' for '", schema->name(),
              "' is invalid. Only non-negative integers are allowed.");
        }
        auto vectorValue = obj.Get(stringIndex);
        if (!vectorValue.IsNumber()) {
          return zvec::Status::InvalidArgument(
              "All elements in the vector object for '", schema->name(),
              "' must be numbers");
        }
        indices_ptr[i] = index;
        values_ptr[i] = vectorValue.As<Napi::Number>().FloatValue();
      }
      query->query_sparse_indices_ = std::move(indices);
      query->query_sparse_values_ = std::move(values);
      break;
    }
    case zvec::DataType::SPARSE_VECTOR_FP16: {
      values.resize(length * sizeof(uint16_t));
      auto *indices_ptr = reinterpret_cast<uint32_t *>(indices.data());
      auto *values_ptr =
          reinterpret_cast<zvec::ailego::Float16 *>(values.data());
      for (uint32_t i = 0; i < length; ++i) {
        auto stringIndex = stringIndices.Get(i).As<Napi::String>().Utf8Value();
        uint32_t index;
        try {
          unsigned long parsed_ulong = std::stoul(stringIndex);
          if (parsed_ulong > std::numeric_limits<uint32_t>::max()) {
            return zvec::Status::InvalidArgument(
                "Index '", stringIndex, "' for '", schema->name(),
                "' is out of range for uint32_t.");
          }
          index = static_cast<uint32_t>(parsed_ulong);
        } catch (const std::exception &) {
          return zvec::Status::InvalidArgument(
              "Index '", stringIndex, "' for '", schema->name(),
              "' is invalid. Only non-negative integers are allowed.");
        }
        auto vectorValue = obj.Get(stringIndex);
        if (!vectorValue.IsNumber()) {
          return zvec::Status::InvalidArgument(
              "All elements in the vector object for '", schema->name(),
              "' must be numbers");
        }
        indices_ptr[i] = index;
        zvec::ailego::Float16 val = vectorValue.As<Napi::Number>().FloatValue();
        values_ptr[i] = val;
      }
      query->query_sparse_indices_ = std::move(indices);
      query->query_sparse_values_ = std::move(values);
      break;
    }
    default: {
      return zvec::Status::InternalError(
          "Unknown data type: ", static_cast<uint32_t>(schema->data_type()));
    }
  }
  return zvec::Status::OK();
}


zvec::Result<zvec::QueryParams::Ptr> ParseQueryParams(
    const Napi::Value &value) {
  if (!value.IsObject()) {
    return tl::make_unexpected(
        zvec::Status::InvalidArgument("Expected an object for QueryParams"));
  }

  auto obj = value.As<Napi::Object>();

  if (!obj.Has("indexType")) {
    return tl::make_unexpected(zvec::Status::InvalidArgument(
        "Missing required argument 'indexType' in QueryParams"));
  }
  auto parsed_query_type = ParseIndexType(obj.Get("indexType"));
  if (!parsed_query_type) {
    return tl::make_unexpected(parsed_query_type.error());
  }
  auto query_type = parsed_query_type.value();

  switch (query_type) {
    case zvec::IndexType::FLAT:
      return tl::make_unexpected(zvec::Status::InvalidArgument(
          "Flat index type is not allowed for QueryParams"));
    case zvec::IndexType::HNSW:
      return ParseHnswQueryParams(obj);
    case zvec::IndexType::IVF:
      return ParseIVFQueryParams(obj);
    case zvec::IndexType::INVERT:
      return tl::make_unexpected(zvec::Status::InvalidArgument(
          "Inverted index type is not allowed for QueryParams"));
    default:
      uint32_t type_val = static_cast<uint32_t>(query_type);
      return tl::make_unexpected(zvec::Status::InvalidArgument(
          "Invalid index type enum value: " + std::to_string(type_val)));
  }
}


zvec::Result<zvec::FlatQueryParams::Ptr> ParseFlatQueryParams(
    const Napi::Object &obj) {
  return std::make_shared<zvec::FlatQueryParams>();
}


zvec::Result<zvec::HnswQueryParams::Ptr> ParseHnswQueryParams(
    const Napi::Object &obj) {
  int ef{300};
  if (obj.Has("ef")) {
    if (obj.Get("ef").IsNumber()) {
      ef = obj.Get("ef").As<Napi::Number>().Int32Value();
    } else {
      return tl::make_unexpected(
          zvec::Status::InvalidArgument("Expected a number for 'ef'"));
    }
  }

  float radius{0.0f};
  if (obj.Has("radius")) {
    if (obj.Get("radius").IsNumber()) {
      radius = obj.Get("radius").As<Napi::Number>().FloatValue();
    } else {
      return tl::make_unexpected(
          zvec::Status::InvalidArgument("Expected a number for 'radius'"));
    }
  }

  bool is_linear{false};
  if (obj.Has("isLinear")) {
    if (obj.Get("isLinear").IsBoolean()) {
      is_linear = obj.Get("isLinear").As<Napi::Boolean>().Value();
    } else {
      return tl::make_unexpected(
          zvec::Status::InvalidArgument("Expected a boolean for 'isLinear'"));
    }
  }

  bool is_using_refiner{false};
  if (obj.Has("isUsingRefiner")) {
    if (obj.Get("isUsingRefiner").IsBoolean()) {
      is_using_refiner = obj.Get("isUsingRefiner").As<Napi::Boolean>().Value();
    } else {
      return tl::make_unexpected(zvec::Status::InvalidArgument(
          "Expected a boolean for 'isUsingRefiner'"));
    }
  }

  return std::make_shared<zvec::HnswQueryParams>(ef, radius, is_linear,
                                                 is_using_refiner);
}


zvec::Result<zvec::IVFQueryParams::Ptr> ParseIVFQueryParams(
    const Napi::Object &obj) {
  int nprobe{10};
  if (obj.Has("nprobe")) {
    if (obj.Get("nprobe").IsNumber()) {
      nprobe = obj.Get("nprobe").As<Napi::Number>().Int32Value();
    } else {
      return tl::make_unexpected(
          zvec::Status::InvalidArgument("Expected a number for 'nprobe'"));
    }
  }

  float radius{0.0f};
  if (obj.Has("radius")) {
    if (obj.Get("radius").IsNumber()) {
      radius = obj.Get("radius").As<Napi::Number>().FloatValue();
    } else {
      return tl::make_unexpected(
          zvec::Status::InvalidArgument("Expected a number for 'radius'"));
    }
  }

  bool is_linear{false};
  if (obj.Has("isLinear")) {
    if (obj.Get("isLinear").IsBoolean()) {
      is_linear = obj.Get("isLinear").As<Napi::Boolean>().Value();
    } else {
      return tl::make_unexpected(
          zvec::Status::InvalidArgument("Expected a boolean for 'isLinear'"));
    }
  }

  bool is_using_refiner{false};
  if (obj.Has("isUsingRefiner")) {
    if (obj.Get("isUsingRefiner").IsBoolean()) {
      is_using_refiner = obj.Get("isUsingRefiner").As<Napi::Boolean>().Value();
    } else {
      return tl::make_unexpected(zvec::Status::InvalidArgument(
          "Expected a boolean for 'isUsingRefiner'"));
    }
  }

  return std::make_shared<zvec::IVFQueryParams>(nprobe, is_using_refiner);
}


zvec::Result<zvec::CollectionOptions> ParseCollectionOptions(
    const Napi::Object &obj) {
  bool read_only{false};
  if (obj.Has("readOnly")) {
    if (obj.Get("readOnly").IsBoolean()) {
      read_only = obj.Get("readOnly").As<Napi::Boolean>();
    } else {
      return tl::make_unexpected(
          zvec::Status::InvalidArgument("Expected a boolean for 'readOnly'"));
    }
  }

  bool enable_mmap{true};
  if (obj.Has("enableMMAP")) {
    if (obj.Get("enableMMAP").IsBoolean()) {
      enable_mmap = obj.Get("enableMMAP").As<Napi::Boolean>();
    } else {
      return tl::make_unexpected(
          zvec::Status::InvalidArgument("Expected a boolean for 'enableMMAP'"));
    }
  }

  return zvec::CollectionOptions{read_only, enable_mmap};
}


Napi::Object CreateCollectionOptions(Napi::Env env,
                                     zvec::CollectionOptions options) {
  auto obj = Napi::Object::New(env);
  obj.Set("readOnly", Napi::Boolean::New(env, options.read_only_));
  obj.Set("enableMMAP", Napi::Boolean::New(env, options.enable_mmap_));
  return obj;
}


zvec::Result<zvec::AddColumnOptions> ParseAddColumnOptions(
    const Napi::Object &obj) {
  int concurrency{0};
  if (obj.Has("concurrency")) {
    if (obj.Get("concurrency").IsNumber()) {
      concurrency = obj.Get("concurrency").As<Napi::Number>().Int32Value();
    } else {
      return tl::make_unexpected(
          zvec::Status::InvalidArgument("Expected a number for 'concurrency'"));
    }
  }

  return zvec::AddColumnOptions{concurrency};
}


zvec::Result<zvec::AlterColumnOptions> ParseAlterColumnOptions(
    const Napi::Object &obj) {
  int concurrency{0};
  if (obj.Has("concurrency")) {
    if (obj.Get("concurrency").IsNumber()) {
      concurrency = obj.Get("concurrency").As<Napi::Number>().Int32Value();
    } else {
      return tl::make_unexpected(
          zvec::Status::InvalidArgument("Expected a number for 'concurrency'"));
    }
  }

  return zvec::AlterColumnOptions{concurrency};
}


zvec::Result<zvec::CreateIndexOptions> ParseCreateIndexOptions(
    const Napi::Object &obj) {
  int concurrency{0};
  if (obj.Has("concurrency")) {
    if (obj.Get("concurrency").IsNumber()) {
      concurrency = obj.Get("concurrency").As<Napi::Number>().Int32Value();
    } else {
      return tl::make_unexpected(
          zvec::Status::InvalidArgument("Expected a number for 'concurrency'"));
    }
  }

  return zvec::CreateIndexOptions{concurrency};
}


zvec::Result<zvec::OptimizeOptions> ParseOptimizeOptions(
    const Napi::Object &obj) {
  int concurrency{0};
  if (obj.Has("concurrency")) {
    if (obj.Get("concurrency").IsNumber()) {
      concurrency = obj.Get("concurrency").As<Napi::Number>().Int32Value();
    } else {
      return tl::make_unexpected(
          zvec::Status::InvalidArgument("Expected a number for 'concurrency'"));
    }
  }

  return zvec::OptimizeOptions{concurrency};
}


}  // namespace binding