#pragma once


#include <napi.h>


namespace binding {


Napi::Object CreateLogTypeObject(Napi::Env env);

Napi::Object CreateLogLevelObject(Napi::Env env);

Napi::Value Initialize(const Napi::CallbackInfo &info);

Napi::Object InitConfig(Napi::Env env, Napi::Object exports);


}  // namespace binding