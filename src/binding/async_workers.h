#pragma once


#include <napi.h>
#include <string>
#include <zvec/db/collection.h>
#include <zvec/db/status.h>


namespace binding {


class DeleteByFilterWorker : public Napi::AsyncWorker {
 public:
  DeleteByFilterWorker(Napi::Env env, zvec::Collection::Ptr collection,
                       std::string filter, Napi::Promise::Deferred deferred);

  void Execute() override;
  void OnOK() override;
  void OnError(const Napi::Error &error) override;

 private:
  zvec::Collection::Ptr collection_;
  std::string filter_;
  zvec::Status status_;
  Napi::Promise::Deferred deferred_;
};


class QueryWorker : public Napi::AsyncWorker {
 public:
  QueryWorker(Napi::Env env, zvec::Collection::Ptr collection,
              zvec::CollectionSchema::Ptr schema, zvec::SearchQuery query,
              Napi::Promise::Deferred deferred);

  void Execute() override;
  void OnOK() override;
  void OnError(const Napi::Error &error) override;

 private:
  zvec::Collection::Ptr collection_;
  zvec::CollectionSchema::Ptr schema_;
  zvec::SearchQuery query_;
  Napi::Promise::Deferred deferred_;
  zvec::Status status_;
  zvec::DocPtrList results_;
};


class OptimizeWorker : public Napi::AsyncWorker {
 public:
  OptimizeWorker(Napi::Env env, zvec::Collection::Ptr collection,
                 zvec::OptimizeOptions options,
                 Napi::Promise::Deferred deferred);

  void Execute() override;
  void OnOK() override;
  void OnError(const Napi::Error &error) override;

 private:
  zvec::Collection::Ptr collection_;
  zvec::OptimizeOptions options_;
  zvec::Status status_;
  Napi::Promise::Deferred deferred_;
};


}  // namespace binding
