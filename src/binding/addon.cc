#include "addon.h"
#include "collection.h"
#include "config.h"
#include "schema.h"
#include "types.h"


AddonConstructors *get_constructors(Napi::Env &env) {
  auto *addon_constructors = env.GetInstanceData<AddonConstructors>();
  if (addon_constructors) {
    return addon_constructors;
  } else {
    binding::ThrowIfNotOk(
        env, zvec::Status::InternalError(
                 "AddonConstructors not found in environment data"));
    return nullptr;
  }
}


void console_log(Napi::Env env, const std::string msg) {
  Napi::Object console = env.Global().Get("console").As<Napi::Object>();
  Napi::Function log = console.Get("log").As<Napi::Function>();
  log.Call(console, {Napi::String::New(env, msg)});
}


Napi::Object Init(Napi::Env env, Napi::Object exports) {
  auto *ctors = new AddonConstructors{};

  exports = binding::InitTypes(env, exports);
  exports = binding::InitConfig(env, exports);
  exports = binding::CollectionSchema::Init(env, exports, ctors);
  exports = binding::Collection::Init(env, exports, ctors);

  env.SetInstanceData(ctors);

  return exports;
}


NODE_API_MODULE(zvec_node_binding, Init)