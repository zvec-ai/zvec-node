#include "doc_iterator.h"
#include <utility>
#include "doc.h"
#include "types.h"


namespace binding {


Napi::Object DocIterator::Init(Napi::Env env, Napi::Object exports,
                               AddonConstructors *ctors) {
  Napi::Function func =
      DefineClass(env, "DocIterator",
                  {
                      InstanceMethod("next", &DocIterator::Next),
                      InstanceMethod("return", &DocIterator::Return),
                      InstanceMethod("closeSync", &DocIterator::Close),
                      InstanceMethod(Napi::Symbol::WellKnown(env, "iterator"),
                                     &DocIterator::GetIterator),
                  });

  ctors->docIteratorConstructor = Napi::Persistent(func);
  return exports;
}


DocIterator::DocIterator(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<DocIterator>(info) {
  Napi::Env env = info.Env();
  const bool called_from_internal =
      info.Length() == 1 && info[0].IsExternal() &&
      info[0].As<Napi::External<void>>().Data() == kInternalConstructionMarker;
  if (!called_from_internal) {
    ThrowIfNotOk(env, zvec::Status::PermissionDenied(
                          "Direct instantiation of DocIterator is not allowed. "
                          "Use Collection.iterDocsSync() instead"));
  }
}


DocIterator::~DocIterator() {
  CloseInternal();
}


void DocIterator::set_wrapped(zvec::DocIterator::Ptr iterator,
                              zvec::Collection::Ptr collection,
                              zvec::CollectionSchema::Ptr schema) {
  collection_ = std::move(collection);
  schema_ = std::move(schema);
  iterator_ = std::move(iterator);
}


Napi::Value DocIterator::Next(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  if (info.Length() != 0) {
    ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                          "DocIterator.next(): Expected no arguments"));
    return env.Undefined();
  }
  if (!iterator_) {
    return CreateIteratorResult(env, env.Undefined(), true);
  }

  auto result = iterator_->next();
  if (!result) {
    const auto status = result.error();
    CloseInternal();
    ThrowIfNotOk(env, status);
    return env.Undefined();
  }

  auto doc = std::move(result.value());
  if (!doc) {
    CloseInternal();
    return CreateIteratorResult(env, env.Undefined(), true);
  }

  try {
    return CreateIteratorResult(env, CreateDoc(env, schema_, std::move(doc)),
                                false);
  } catch (...) {
    CloseInternal();
    throw;
  }
}


Napi::Value DocIterator::Return(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  CloseInternal();
  return CreateIteratorResult(env, env.Undefined(), true);
}


Napi::Value DocIterator::Close(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  if (info.Length() != 0) {
    ThrowIfNotOk(env, zvec::Status::InvalidArgument(
                          "DocIterator.closeSync(): Expected no arguments"));
    return env.Undefined();
  }
  CloseInternal();
  return env.Undefined();
}


Napi::Value DocIterator::GetIterator(const Napi::CallbackInfo &info) {
  return info.This();
}


Napi::Object DocIterator::CreateIteratorResult(Napi::Env env, Napi::Value value,
                                               bool done) {
  auto result = Napi::Object::New(env);
  result.Set("value", value);
  result.Set("done", done);
  return result;
}


void DocIterator::CloseInternal() {
  if (iterator_) {
    iterator_->close();
    iterator_.reset();
  }
  schema_.reset();
  collection_.reset();
}


}  // namespace binding
