#include "config.h"
#include "zvec/db/config.h"
#include "types.h"


namespace binding {


Napi::Object CreateLogTypeObject(Napi::Env env) {
  Napi::Object obj = Napi::Object::New(env);
  obj.Set("CONSOLE", "console");
  obj.Set("FILE", "file");
  obj.Freeze();
  return obj;
}


Napi::Object CreateLogLevelObject(Napi::Env env) {
  Napi::Object obj = Napi::Object::New(env);
  obj.Set("DEBUG", static_cast<uint8_t>(zvec::GlobalConfig::LogLevel::DEBUG));
  obj.Set("INFO", static_cast<uint8_t>(zvec::GlobalConfig::LogLevel::INFO));
  obj.Set("WARN", static_cast<uint8_t>(zvec::GlobalConfig::LogLevel::WARN));
  obj.Set("ERROR", static_cast<uint8_t>(zvec::GlobalConfig::LogLevel::ERROR));
  obj.Set("FATAL", static_cast<uint8_t>(zvec::GlobalConfig::LogLevel::FATAL));
  obj.Freeze();
  return obj;
}


Napi::Value Initialize(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  if (info.Length() != 1 || !info[0].IsObject()) {
    ThrowIfNotOk(
        env, zvec::Status::InvalidArgument("Initialize() expects exactly one "
                                           "argument: a configuration object"));
    return env.Undefined();
  }

  zvec::GlobalConfig::ConfigData config{};
  auto obj = info[0].As<Napi::Object>();

  if (obj.Has("logType") || obj.Has("logLevel")) {
    std::string log_type = "console";
    zvec::GlobalConfig::LogLevel log_level{zvec::GlobalConfig::LogLevel::WARN};
    if (obj.Has("logType")) {
      if (obj.Get("logType").IsString()) {
        log_type = obj.Get("logType").As<Napi::String>().Utf8Value();
      } else {
        ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                              "Expected a string for 'logType'"));
        return env.Undefined();
      }
    }
    if (obj.Has("logLevel")) {
      if (obj.Get("logLevel").IsNumber()) {
        log_level = static_cast<zvec::GlobalConfig::LogLevel>(
            obj.Get("logLevel").As<Napi::Number>().Uint32Value());
      } else {
        ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                              "Expected a number for 'logLevel'"));
        return env.Undefined();
      }
    }
    if (log_type == "file") {
      std::string dir = zvec::DEFAULT_LOG_DIR;
      std::string basename = zvec::DEFAULT_LOG_BASENAME;
      uint32_t file_size = zvec::DEFAULT_LOG_FILE_SIZE;
      uint32_t overdue_days = zvec::DEFAULT_LOG_OVERDUE_DAYS;
      if (obj.Has("logDir")) {
        if (obj.Get("logDir").IsString()) {
          dir = obj.Get("logDir").As<Napi::String>().Utf8Value();
        } else {
          ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                                "Expected a string for 'logDir'"));
          return env.Undefined();
        }
      }
      if (obj.Has("logBaseName")) {
        if (obj.Get("logBaseName").IsString()) {
          basename = obj.Get("logBaseName").As<Napi::String>().Utf8Value();
        } else {
          ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                                "Expected a string for 'logBaseName'"));
          return env.Undefined();
        }
      }
      if (obj.Has("logFileSize")) {
        if (obj.Get("logFileSize").IsNumber()) {
          file_size = obj.Get("logFileSize").As<Napi::Number>().Uint32Value();
        } else {
          ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                                "Expected a number for 'logFileSize'"));
          return env.Undefined();
        }
      }
      if (obj.Has("logOverdueDays")) {
        if (obj.Get("logOverdueDays").IsNumber()) {
          overdue_days =
              obj.Get("logOverdueDays").As<Napi::Number>().Uint32Value();
        } else {
          ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                                "Expected a number for 'logOverdueDays'"));
          return env.Undefined();
        }
      }
      config.log_config = std::make_shared<zvec::GlobalConfig::FileLogConfig>(
          log_level, dir, basename, file_size, overdue_days);
    } else if (log_type == "console") {
      config.log_config =
          std::make_shared<zvec::GlobalConfig::ConsoleLogConfig>(log_level);
    } else {
      ThrowIfNotOk(
          env, zvec::Status::InvalidArgument("Invalid value for 'logType'"));
      return env.Undefined();
    }
  }

  if (obj.Has("queryThreads")) {
    if (obj.Get("queryThreads").IsNumber()) {
      config.query_thread_count =
          obj.Get("queryThreads").As<Napi::Number>().Uint32Value();
    } else {
      ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                            "Expected a number for 'queryThreads'"));
      return env.Undefined();
    }
  }

  if (obj.Has("optimizeThreads")) {
    if (obj.Get("optimizeThreads").IsNumber()) {
      config.optimize_thread_count =
          obj.Get("optimizeThreads").As<Napi::Number>().Uint32Value();
    } else {
      ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                            "Expected a number for 'optimizeThreads'"));
      return env.Undefined();
    }
  }

  if (obj.Has("invertToForwardScanRatio")) {
    if (obj.Get("invertToForwardScanRatio").IsNumber()) {
      config.invert_to_forward_scan_ratio =
          obj.Get("invertToForwardScanRatio").As<Napi::Number>().FloatValue();
    } else {
      ThrowIfNotOk(env,
                   zvec::Status::InvalidArgument(
                       "Expected a number for 'invertToForwardScanRatio'"));
      return env.Undefined();
    }
  }

  if (obj.Has("bruteForceByKeysRatio")) {
    if (obj.Get("bruteForceByKeysRatio").IsNumber()) {
      config.brute_force_by_keys_ratio =
          obj.Get("bruteForceByKeysRatio").As<Napi::Number>().FloatValue();
    } else {
      ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                            "Expected a number for 'bruteForceByKeysRatio'"));
      return env.Undefined();
    }
  }

  if (auto s = zvec::GlobalConfig::Instance().Initialize(config); !s.ok()) {
    ThrowIfNotOk(env, s);
  }
  return env.Undefined();
}


Napi::Object InitConfig(Napi::Env env, Napi::Object exports) {
  exports.Set("LogType", CreateLogTypeObject(env));
  exports.Set("LogLevel", CreateLogLevelObject(env));
  exports.Set("initialize", Napi::Function::New(env, Initialize));
  return exports;
}


}  // namespace binding