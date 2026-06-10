#include "params.h"
#include <algorithm>
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
    case zvec::IndexType::HNSW_RABITQ:
      return ParseHnswRabitqIndexParams(obj);
    case zvec::IndexType::IVF:
      return ParseIVFIndexParams(obj);
    case zvec::IndexType::DISKANN:
      return ParseDiskAnnIndexParams(obj);
    case zvec::IndexType::INVERT:
      return ParseInvertIndexParams(obj);
    case zvec::IndexType::FTS:
      return ParseFtsIndexParams(obj);
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
    case zvec::IndexType::HNSW_RABITQ:
      return CreateHnswRabitqIndexParams(env, params);
    case zvec::IndexType::IVF:
      return CreateIVFIndexParams(env, params);
    case zvec::IndexType::DISKANN:
      return CreateDiskAnnIndexParams(env, params);
    case zvec::IndexType::INVERT:
      return CreateInvertIndexParams(env, params);
    case zvec::IndexType::FTS:
      return CreateFtsIndexParams(env, params);
    default:
      return Napi::Object::New(env);
  }
}


zvec::Result<zvec::FlatIndexParams::OPtr> ParseFlatIndexParams(
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


zvec::Result<zvec::HnswIndexParams::OPtr> ParseHnswIndexParams(
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

  bool use_contiguous_memory{false};
  if (obj.Has("useContiguousMemory")) {
    if (obj.Get("useContiguousMemory").IsBoolean()) {
      use_contiguous_memory =
          obj.Get("useContiguousMemory").As<Napi::Boolean>().Value();
    } else {
      return tl::make_unexpected(zvec::Status::InvalidArgument(
          "Expected a boolean for 'useContiguousMemory'"));
    }
  }

  return std::make_shared<zvec::HnswIndexParams>(
      metric_type, m, ef_construction, quantize_type, use_contiguous_memory);
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
  obj.Set("useContiguousMemory", hnsw_params->use_contiguous_memory());
  return obj;
}


zvec::Result<zvec::HnswRabitqIndexParams::OPtr> ParseHnswRabitqIndexParams(
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

  int total_bits{7};
  if (obj.Has("totalBits")) {
    if (obj.Get("totalBits").IsNumber()) {
      total_bits = obj.Get("totalBits").As<Napi::Number>();
    } else {
      return tl::make_unexpected(
          zvec::Status::InvalidArgument("Expected a number for 'totalBits'"));
    }
  }

  int num_clusters{16};
  if (obj.Has("numClusters")) {
    if (obj.Get("numClusters").IsNumber()) {
      num_clusters = obj.Get("numClusters").As<Napi::Number>();
    } else {
      return tl::make_unexpected(
          zvec::Status::InvalidArgument("Expected a number for 'numClusters'"));
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

  int sample_count{0};
  if (obj.Has("sampleCount")) {
    if (obj.Get("sampleCount").IsNumber()) {
      sample_count = obj.Get("sampleCount").As<Napi::Number>();
    } else {
      return tl::make_unexpected(
          zvec::Status::InvalidArgument("Expected a number for 'sampleCount'"));
    }
  }

  return std::make_shared<zvec::HnswRabitqIndexParams>(
      metric_type, total_bits, num_clusters, m, ef_construction, sample_count);
}


Napi::Object CreateHnswRabitqIndexParams(Napi::Env env,
                                         zvec::IndexParams::Ptr params) {
  auto obj = Napi::Object::New(env);
  auto rabitq_params =
      std::dynamic_pointer_cast<zvec::HnswRabitqIndexParams>(params);
  obj.Set("indexType", static_cast<uint32_t>(rabitq_params->type()));
  obj.Set("metricType", static_cast<uint32_t>(rabitq_params->metric_type()));
  obj.Set("totalBits", rabitq_params->total_bits());
  obj.Set("numClusters", rabitq_params->num_clusters());
  obj.Set("m", rabitq_params->m());
  obj.Set("efConstruction", rabitq_params->ef_construction());
  obj.Set("sampleCount", rabitq_params->sample_count());
  return obj;
}


zvec::Result<zvec::IVFIndexParams::OPtr> ParseIVFIndexParams(
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


zvec::Result<zvec::DiskAnnIndexParams::OPtr> ParseDiskAnnIndexParams(
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

  int max_degree{100};
  if (obj.Has("maxDegree")) {
    if (obj.Get("maxDegree").IsNumber()) {
      max_degree = obj.Get("maxDegree").As<Napi::Number>().Int32Value();
    } else {
      return tl::make_unexpected(
          zvec::Status::InvalidArgument("Expected a number for 'maxDegree'"));
    }
  }

  int list_size{50};
  if (obj.Has("listSize")) {
    if (obj.Get("listSize").IsNumber()) {
      list_size = obj.Get("listSize").As<Napi::Number>().Int32Value();
    } else {
      return tl::make_unexpected(
          zvec::Status::InvalidArgument("Expected a number for 'listSize'"));
    }
  }

  int pq_chunk_num{0};
  if (obj.Has("pqChunkNum")) {
    if (obj.Get("pqChunkNum").IsNumber()) {
      pq_chunk_num = obj.Get("pqChunkNum").As<Napi::Number>().Int32Value();
    } else {
      return tl::make_unexpected(
          zvec::Status::InvalidArgument("Expected a number for 'pqChunkNum'"));
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

  return std::make_shared<zvec::DiskAnnIndexParams>(
      metric_type, max_degree, list_size, pq_chunk_num, quantize_type);
}


Napi::Object CreateDiskAnnIndexParams(Napi::Env env,
                                      zvec::IndexParams::Ptr params) {
  auto obj = Napi::Object::New(env);
  auto diskann_params =
      std::dynamic_pointer_cast<zvec::DiskAnnIndexParams>(params);
  obj.Set("indexType", static_cast<uint32_t>(diskann_params->type()));
  obj.Set("metricType", static_cast<uint32_t>(diskann_params->metric_type()));
  obj.Set("maxDegree", diskann_params->max_degree());
  obj.Set("listSize", diskann_params->list_size());
  obj.Set("pqChunkNum", diskann_params->pq_chunk_num());
  obj.Set("quantizeType",
          static_cast<uint32_t>(diskann_params->quantize_type()));
  return obj;
}


zvec::Result<zvec::InvertIndexParams::OPtr> ParseInvertIndexParams(
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


zvec::Result<std::shared_ptr<zvec::FtsIndexParams>> ParseFtsIndexParams(
    const Napi::Object &obj) {
  std::string tokenizer_name{"standard"};
  if (obj.Has("tokenizerName")) {
    if (obj.Get("tokenizerName").IsString()) {
      tokenizer_name = obj.Get("tokenizerName").As<Napi::String>().Utf8Value();
    } else {
      return tl::make_unexpected(zvec::Status::InvalidArgument(
          "Expected a string for 'tokenizerName'"));
    }
  }

  std::vector<std::string> filters{"lowercase"};
  if (obj.Has("filters")) {
    if (!obj.Get("filters").IsArray()) {
      return tl::make_unexpected(zvec::Status::InvalidArgument(
          "Expected an array of strings for 'filters'"));
    }
    filters.clear();
    Napi::Array array = obj.Get("filters").As<Napi::Array>();
    for (uint32_t i = 0; i < array.Length(); i++) {
      auto item = array.Get(i);
      if (!item.IsString()) {
        return tl::make_unexpected(zvec::Status::InvalidArgument(
            "Expected an array of strings for 'filters'"));
      }
      filters.emplace_back(item.As<Napi::String>().Utf8Value());
    }
  }

  std::string extra_params{};
  if (obj.Has("extraParams")) {
    if (obj.Get("extraParams").IsString()) {
      extra_params = obj.Get("extraParams").As<Napi::String>().Utf8Value();
    } else {
      return tl::make_unexpected(
          zvec::Status::InvalidArgument("Expected a string for 'extraParams'"));
    }
  }

  return std::make_shared<zvec::FtsIndexParams>(tokenizer_name, filters,
                                                extra_params);
}


Napi::Object CreateFtsIndexParams(Napi::Env env,
                                  zvec::IndexParams::Ptr params) {
  auto obj = Napi::Object::New(env);
  auto fts_params = std::dynamic_pointer_cast<zvec::FtsIndexParams>(params);
  obj.Set("indexType", static_cast<uint32_t>(fts_params->type()));
  obj.Set("tokenizerName", fts_params->tokenizer_name());
  auto filters = Napi::Array::New(env, fts_params->filters().size());
  for (size_t i = 0; i < fts_params->filters().size(); i++) {
    filters.Set(i, fts_params->filters()[i]);
  }
  obj.Set("filters", filters);
  obj.Set("extraParams", fts_params->extra_params());
  return obj;
}


namespace {

zvec::Status ParseVectorToString(const Napi::Value &value,
                                 zvec::FieldSchema *schema,
                                 zvec::QueryTarget *target) {
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
        target->set_vector(std::move(buf));
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
        target->set_vector(std::move(buf));
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
        target->set_vector(std::move(buf));
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
        target->set_vector(std::move(buf));
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
        target->set_vector(std::move(buf));
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
        target->set_vector(std::move(buf));
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
                              zvec::QueryTarget *target) {
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
      target->set_sparse_vector(std::move(indices), std::move(values));
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
      target->set_sparse_vector(std::move(indices), std::move(values));
      break;
    }
    default: {
      return zvec::Status::InternalError(
          "Unknown data type: ", static_cast<uint32_t>(schema->data_type()));
    }
  }
  return zvec::Status::OK();
}


zvec::Result<zvec::FtsClause> ParseFtsClause(const Napi::Value &value) {
  if (!value.IsObject()) {
    return tl::make_unexpected(
        zvec::Status::InvalidArgument("Expected an object for 'fts'"));
  }

  auto obj = value.As<Napi::Object>();
  zvec::FtsClause fts_clause{};
  if (obj.Has("queryString")) {
    if (!obj.Get("queryString").IsString()) {
      return tl::make_unexpected(zvec::Status::InvalidArgument(
          "Expected a string for 'queryString' in 'fts'"));
    }
    fts_clause.query_string_ =
        obj.Get("queryString").As<Napi::String>().Utf8Value();
  }
  if (obj.Has("matchString")) {
    if (!obj.Get("matchString").IsString()) {
      return tl::make_unexpected(zvec::Status::InvalidArgument(
          "Expected a string for 'matchString' in 'fts'"));
    }
    fts_clause.match_string_ =
        obj.Get("matchString").As<Napi::String>().Utf8Value();
  }
  return fts_clause;
}


zvec::Result<zvec::QueryTarget> ParseQueryTarget(
    const Napi::Object &obj, zvec::CollectionSchema::Ptr schema) {
  zvec::QueryTarget target{};
  zvec::FieldSchema *field_schema{nullptr};

  if (obj.Has("fieldName")) {
    if (obj.Get("fieldName").IsString()) {
      target.field_name_ = obj.Get("fieldName").As<Napi::String>().Utf8Value();
      field_schema = schema->get_field(target.field_name_);
    } else {
      return tl::make_unexpected(zvec::Status::InvalidArgument(
          "Expected a string for 'fieldName' in Query"));
    }
  }

  const bool has_vector = obj.Has("vector") &&
                          !obj.Get("vector").IsUndefined() &&
                          !obj.Get("vector").IsNull();
  const bool has_fts = obj.Has("fts") && !obj.Get("fts").IsUndefined() &&
                       !obj.Get("fts").IsNull();

  if (has_vector && has_fts) {
    return tl::make_unexpected(zvec::Status::InvalidArgument(
        "Cannot combine 'vector' and 'fts' in a single Query"));
  }

  if (has_vector) {
    if (!field_schema) {
      if (target.field_name_.empty()) {
        return tl::make_unexpected(zvec::Status::InvalidArgument(
            "Missing required 'fieldName' when performing vector queries"));
      }
      return tl::make_unexpected(zvec::Status::InvalidArgument(
          "Field '", target.field_name_, "' not found in collection schema"));
    }
    auto vector = obj.Get("vector");
    if (field_schema->is_dense_vector()) {
      if (auto s = ParseVectorToString(vector, field_schema, &target);
          !s.ok()) {
        return tl::make_unexpected(s);
      }
    } else if (field_schema->is_sparse_vector()) {
      if (auto s = ParseVectorToMap(vector, field_schema, &target); !s.ok()) {
        return tl::make_unexpected(s);
      }
    } else {
      return tl::make_unexpected(
          zvec::Status::InvalidArgument("Field '", target.field_name_,
                                        "' is not a vector field and cannot be "
                                        "queried using vector operations."));
    }
  } else if (has_fts) {
    auto parsed_fts = ParseFtsClause(obj.Get("fts"));
    if (parsed_fts) {
      target.clause_ = std::move(parsed_fts.value());
    } else {
      return tl::make_unexpected(parsed_fts.error());
    }
  }

  if (obj.Has("params")) {
    auto parsed_params = ParseQueryParams(obj.Get("params"));
    if (parsed_params) {
      target.query_params_ = parsed_params.value();
    } else {
      return tl::make_unexpected(parsed_params.error());
    }
  }

  return target;
}


zvec::Status ParseCommonQueryOptions(
    const Napi::Object &obj, int *topk, std::string *filter,
    bool *include_vector,
    std::optional<std::vector<std::string>> *output_fields) {
  if (obj.Has("topk")) {
    if (obj.Get("topk").IsNumber()) {
      *topk = obj.Get("topk").As<Napi::Number>().Int32Value();
    } else {
      return zvec::Status::InvalidArgument("Expected a number for 'topk'");
    }
  }
  if (obj.Has("filter")) {
    if (obj.Get("filter").IsString()) {
      *filter = obj.Get("filter").As<Napi::String>().Utf8Value();
    } else {
      return zvec::Status::InvalidArgument("Expected a string for 'filter'");
    }
  }
  if (obj.Has("includeVector")) {
    if (obj.Get("includeVector").IsBoolean()) {
      *include_vector = obj.Get("includeVector").As<Napi::Boolean>().Value();
    } else {
      return zvec::Status::InvalidArgument(
          "Expected a boolean for 'includeVector'");
    }
  }
  if (obj.Has("outputFields")) {
    if (!obj.Get("outputFields").IsArray()) {
      return zvec::Status::InvalidArgument(
          "Expected an array of strings for 'outputFields'");
    }
    std::vector<std::string> parsed_output_fields{};
    auto array = obj.Get("outputFields").As<Napi::Array>();
    for (uint32_t i = 0; i < array.Length(); i++) {
      if (!array.Get(i).IsString()) {
        return zvec::Status::InvalidArgument(
            "Expected an array of strings for 'outputFields'");
      }
      parsed_output_fields.push_back(
          array.Get(i).As<Napi::String>().Utf8Value());
    }
    *output_fields = std::move(parsed_output_fields);
  }
  return zvec::Status::OK();
}


zvec::Result<zvec::reranker::RerankParams> ParseRerankParams(
    const Napi::Value &value) {
  if (!value.IsObject()) {
    return tl::make_unexpected(
        zvec::Status::InvalidArgument("Expected an object for 'rerank'"));
  }

  auto obj = value.As<Napi::Object>();
  if (!obj.Has("type") || !obj.Get("type").IsString()) {
    return tl::make_unexpected(zvec::Status::InvalidArgument(
        "Expected a string for 'type' in 'rerank'"));
  }
  std::string type = obj.Get("type").As<Napi::String>().Utf8Value();

  if (type == "rrf") {
    int rank_constant{60};
    if (obj.Has("rankConstant")) {
      if (!obj.Get("rankConstant").IsNumber()) {
        return tl::make_unexpected(zvec::Status::InvalidArgument(
            "Expected a number for 'rankConstant' in 'rerank'"));
      }
      rank_constant = obj.Get("rankConstant").As<Napi::Number>().Int32Value();
    }
    return zvec::reranker::RrfParams{rank_constant};
  }

  if (type == "weighted") {
    if (!obj.Has("weights") || !obj.Get("weights").IsArray()) {
      return tl::make_unexpected(zvec::Status::InvalidArgument(
          "Expected an array of numbers for 'weights' in weighted rerank"));
    }

    std::vector<double> weights{};
    auto weights_array = obj.Get("weights").As<Napi::Array>();
    for (uint32_t i = 0; i < weights_array.Length(); i++) {
      if (!weights_array.Get(i).IsNumber()) {
        return tl::make_unexpected(zvec::Status::InvalidArgument(
            "Expected an array of numbers for 'weights' in weighted rerank"));
      }
      weights.push_back(weights_array.Get(i).As<Napi::Number>().DoubleValue());
    }
    return zvec::reranker::WeightedParams{std::move(weights)};
  }

  return tl::make_unexpected(zvec::Status::InvalidArgument(
      "Unsupported rerank type '", type, "'. Expected 'rrf' or 'weighted'"));
}

}  // namespace


zvec::Result<zvec::SearchQuery> ParseSearchQuery(
    const Napi::Value &value, zvec::CollectionSchema::Ptr schema) {
  if (!value.IsObject()) {
    return tl::make_unexpected(
        zvec::Status::InvalidArgument("Expected an object for Query"));
  }

  zvec::SearchQuery query{};
  query.topk_ = 10;
  auto obj = value.As<Napi::Object>();

  if (auto s = ParseCommonQueryOptions(obj, &query.topk_, &query.filter_,
                                       &query.include_vector_,
                                       &query.output_fields_);
      !s.ok()) {
    return tl::make_unexpected(s);
  }

  auto parsed_target = ParseQueryTarget(obj, schema);
  if (parsed_target) {
    query.target_ = std::move(parsed_target.value());
  } else {
    return tl::make_unexpected(parsed_target.error());
  }

  return query;
}


zvec::Result<zvec::MultiQuery> ParseMultiQuery(
    const Napi::Value &value, zvec::CollectionSchema::Ptr schema) {
  if (!value.IsObject()) {
    return tl::make_unexpected(
        zvec::Status::InvalidArgument("Expected an object for MultiQuery"));
  }

  zvec::MultiQuery query{};
  query.topk = 10;
  auto obj = value.As<Napi::Object>();

  if (auto s =
          ParseCommonQueryOptions(obj, &query.topk, &query.filter,
                                  &query.include_vector, &query.output_fields);
      !s.ok()) {
    return tl::make_unexpected(s);
  }

  if (!obj.Has("queries") || !obj.Get("queries").IsArray()) {
    return tl::make_unexpected(zvec::Status::InvalidArgument(
        "Expected an array of sub-queries for 'queries'"));
  }

  const int default_num_candidates = std::max(query.topk, 10);
  const auto parse_sub_query =
      [&](const Napi::Value &sub_query_value,
          uint32_t index) -> zvec::Result<zvec::SubQuery> {
    if (!sub_query_value.IsObject()) {
      return tl::make_unexpected(zvec::Status::InvalidArgument(
          "Expected an object for sub-query at index ", index,
          " in 'queries' array"));
    }
    auto sub_query_obj = sub_query_value.As<Napi::Object>();
    auto parsed_target = ParseQueryTarget(sub_query_obj, schema);
    if (!parsed_target) {
      return tl::make_unexpected(parsed_target.error());
    }
    zvec::SubQuery sub_query{};
    sub_query.target_ = std::move(parsed_target.value());
    sub_query.num_candidates_ = default_num_candidates;
    if (sub_query_obj.Has("numCandidates")) {
      if (sub_query_obj.Get("numCandidates").IsNumber()) {
        sub_query.num_candidates_ =
            sub_query_obj.Get("numCandidates").As<Napi::Number>().Int32Value();
      } else {
        return tl::make_unexpected(zvec::Status::InvalidArgument(
            "Expected a number for 'numCandidates' in sub-query at index ",
            index, " in 'queries' array"));
      }
    }
    return sub_query;
  };

  auto queries = obj.Get("queries").As<Napi::Array>();
  query.queries.reserve(queries.Length());
  for (uint32_t i = 0; i < queries.Length(); i++) {
    auto parsed_sub_query = parse_sub_query(queries.Get(i), i);
    if (parsed_sub_query) {
      query.queries.push_back(std::move(parsed_sub_query.value()));
    } else {
      return tl::make_unexpected(parsed_sub_query.error());
    }
  }

  if (obj.Has("rerank")) {
    auto rerank = ParseRerankParams(obj.Get("rerank"));
    if (rerank) {
      query.rerank = std::move(rerank.value());
    } else {
      return tl::make_unexpected(rerank.error());
    }
  } else {
    query.rerank = zvec::reranker::RrfParams{60};
  }

  return query;
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
    case zvec::IndexType::HNSW_RABITQ:
      return ParseHnswRabitqQueryParams(obj);
    case zvec::IndexType::IVF:
      return ParseIVFQueryParams(obj);
    case zvec::IndexType::DISKANN:
      return ParseDiskAnnQueryParams(obj);
    case zvec::IndexType::INVERT:
      return tl::make_unexpected(zvec::Status::InvalidArgument(
          "Inverted index type is not allowed for QueryParams"));
    case zvec::IndexType::FTS:
      return ParseFtsQueryParams(obj);
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


zvec::Result<zvec::HnswRabitqQueryParams::Ptr> ParseHnswRabitqQueryParams(
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

  return std::make_shared<zvec::HnswRabitqQueryParams>(ef, radius, is_linear,
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

  auto params =
      std::make_shared<zvec::IVFQueryParams>(nprobe, is_using_refiner);
  params->set_radius(radius);
  params->set_is_linear(is_linear);
  return params;
}


zvec::Result<zvec::DiskAnnQueryParams::Ptr> ParseDiskAnnQueryParams(
    const Napi::Object &obj) {
  int list_size{300};
  if (obj.Has("listSize")) {
    if (obj.Get("listSize").IsNumber()) {
      list_size = obj.Get("listSize").As<Napi::Number>().Int32Value();
    } else {
      return tl::make_unexpected(
          zvec::Status::InvalidArgument("Expected a number for 'listSize'"));
    }
  }
  return std::make_shared<zvec::DiskAnnQueryParams>(list_size);
}


zvec::Result<zvec::FtsQueryParams::Ptr> ParseFtsQueryParams(
    const Napi::Object &obj) {
  auto params = std::make_shared<zvec::FtsQueryParams>();
  if (obj.Has("defaultOperator")) {
    if (obj.Get("defaultOperator").IsString()) {
      params->set_default_operator(
          obj.Get("defaultOperator").As<Napi::String>().Utf8Value());
    } else {
      return tl::make_unexpected(zvec::Status::InvalidArgument(
          "Expected a string for 'defaultOperator'"));
    }
  }
  return params;
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
