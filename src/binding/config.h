#pragma once


#include <napi.h>


namespace binding {


Napi::Object CreateLogTypeObject(Napi::Env env);

Napi::Object CreateLogLevelObject(Napi::Env env);

Napi::Object CreateIOBackendTypeObject(Napi::Env env);

Napi::Value Initialize(const Napi::CallbackInfo &info);

Napi::Value SetDefaultJiebaDictDir(const Napi::CallbackInfo &info);

Napi::Value GetDefaultJiebaDictDir(const Napi::CallbackInfo &info);

Napi::Value GetIOBackendType(const Napi::CallbackInfo &info);

Napi::Value GetIOBackendDescription(const Napi::CallbackInfo &info);

Napi::Object InitConfig(Napi::Env env, Napi::Object exports);


}  // namespace binding
