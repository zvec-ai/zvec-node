#include "async_workers.h"
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
