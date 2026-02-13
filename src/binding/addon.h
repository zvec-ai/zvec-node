#pragma once


#include <napi.h>


inline void *kInternalConstructionMarker{reinterpret_cast<void *>(0x12345678)};


struct AddonConstructors {
  Napi::FunctionReference collectionSchemaConstructor;
  Napi::FunctionReference collectionConstructor;
};


AddonConstructors *get_constructors(Napi::Env &env);


void console_log(Napi::Env env, const std::string msg);