#include "async_workers.h"
#include "doc.h"
#include "types.h"


namespace binding {


DeleteByFilterWorker::DeleteByFilterWorker(Napi::Env env,
                                           zvec::Collection::Ptr collection,
                                           std::string filter,
                                           Napi::Promise::Deferred deferred)
    : Napi::AsyncWorker(env),
      collection_(collection),
      filter_(std::move(filter)),
      deferred_(deferred) {}

void DeleteByFilterWorker::Execute() {
  status_ = collection_->DeleteByFilter(filter_);
}

void DeleteByFilterWorker::OnOK() {
  Napi::Env env = Env();
  deferred_.Resolve(CreateStatusObject(env, status_));
}

void DeleteByFilterWorker::OnError(const Napi::Error &error) {
  deferred_.Reject(error.Value());
}


QueryWorker::QueryWorker(Napi::Env env, zvec::Collection::Ptr collection,
                         zvec::CollectionSchema::Ptr schema,
                         zvec::SearchQuery query,
                         Napi::Promise::Deferred deferred)
    : Napi::AsyncWorker(env),
      collection_(collection),
      schema_(schema),
      query_(std::move(query)),
      deferred_(deferred) {}

void QueryWorker::Execute() {
  auto res = collection_->Query(query_);
  if (res) {
    results_ = std::move(res.value());
  } else {
    status_ = res.error();
  }
}

void QueryWorker::OnOK() {
  Napi::Env env = Env();
  if (status_.ok()) {
    Napi::Array array = Napi::Array::New(env);
    for (size_t i = 0; i < results_.size(); i++) {
      array.Set(i, CreateDoc(env, schema_, results_[i]));
    }
    deferred_.Resolve(array);
  } else {
    RejectIfNotOk(env, status_, deferred_);
  }
}

void QueryWorker::OnError(const Napi::Error &error) {
  deferred_.Reject(error.Value());
}


OptimizeWorker::OptimizeWorker(Napi::Env env, zvec::Collection::Ptr collection,
                               zvec::OptimizeOptions options,
                               Napi::Promise::Deferred deferred)
    : Napi::AsyncWorker(env),
      collection_(collection),
      options_(options),
      deferred_(deferred) {}

void OptimizeWorker::Execute() {
  status_ = collection_->Optimize(options_);
}

void OptimizeWorker::OnOK() {
  Napi::Env env = Env();
  if (status_.ok()) {
    deferred_.Resolve(env.Undefined());
  } else {
    RejectIfNotOk(env, status_, deferred_);
  }
}
void OptimizeWorker::OnError(const Napi::Error &error) {
  deferred_.Reject(error.Value());
}


}  // namespace binding
