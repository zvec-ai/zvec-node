#pragma once


#include <napi.h>
#include <zvec/db/collection.h>
#include <zvec/db/doc_iterator.h>
#include "addon.h"


namespace binding {


class DocIterator : public Napi::ObjectWrap<DocIterator> {
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports,
                           AddonConstructors *ctors);

  DocIterator(const Napi::CallbackInfo &info);

  ~DocIterator() override;

  void set_wrapped(zvec::DocIterator::Ptr iterator,
                   zvec::Collection::Ptr collection,
                   zvec::CollectionSchema::Ptr schema);


 private:
  Napi::Value Next(const Napi::CallbackInfo &info);

  Napi::Value Return(const Napi::CallbackInfo &info);

  Napi::Value Close(const Napi::CallbackInfo &info);

  Napi::Value GetIterator(const Napi::CallbackInfo &info);

  Napi::Object CreateIteratorResult(Napi::Env env, Napi::Value value,
                                    bool done);

  void CloseInternal();


  // The native iterator releases an active-iterator slot through a callback
  // into its collection. Keep the collection alive until iterator_ has been
  // closed or destroyed. CloseInternal() enforces that destruction order.
  zvec::Collection::Ptr collection_{nullptr};
  zvec::CollectionSchema::Ptr schema_{nullptr};
  zvec::DocIterator::Ptr iterator_{nullptr};
};


}  // namespace binding
