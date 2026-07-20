<div align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://zvec.oss-cn-hongkong.aliyuncs.com/logo/github_log_2.svg" />
    <img src="https://zvec.oss-cn-hongkong.aliyuncs.com/logo/github_logo_1.svg" width="400" alt="zvec logo" />
  </picture>
</div>

<p align="center">
  <a href="https://github.com/zvec-ai/zvec-node/actions/workflows/test-source.yml"><img src="https://github.com/zvec-ai/zvec-node/actions/workflows/test-source.yml/badge.svg?branch=main" alt="Build Status"/></a>
  <a href="https://github.com/zvec-ai/zvec-node/blob/main/LICENSE"><img src="https://img.shields.io/badge/license-Apache%202.0-blue.svg" alt="License"/></a>
  <a href="https://www.npmjs.com/package/@zvec/zvec"><img src="https://img.shields.io/npm/v/@zvec/zvec.svg" alt="npm Release"/></a>
</p>

<p align="center">
  <a href="https://zvec.org/en/docs/db/quickstart/">🚀 <strong>Quickstart</strong> </a> |
  <a href="https://zvec.org/en/">🏠 <strong>Home</strong> </a> |
  <a href="https://zvec.org/en/docs/db/">📚 <strong>Docs</strong> </a> |
  <a href="https://zvec.org/en/docs/db/benchmarks/">📊 <strong>Benchmarks</strong> </a> |
  <a href="https://deepwiki.com/alibaba/zvec">🔎 <strong>DeepWiki</strong> </a> |
  <a href="https://discord.gg/rKddFBBu9z">🎮 <strong>Discord</strong> </a> |
  <a href="https://x.com/ZvecAI">🐦 <strong>X (Twitter)</strong> </a>
</p>

The official Node.js binding for [Zvec](https://github.com/alibaba/zvec), an open-source, in-process vector database that is lightweight, lightning-fast, and designed to embed directly into applications. Battle-tested within Alibaba Group, it delivers production-grade, low-latency and scalable similarity search with minimal setup.

## 💫 Features

- **Blazing Fast**: Searches billions of vectors in milliseconds.
- **Simple, Just Works**: [Install](#-installation) with npm and start searching in seconds. Pure local, no servers, no config, no fuss.
- **Dense + Sparse Vectors**: Support dense and sparse embeddings, multi-vector queries, and a rich selection of [vector index types](https://zvec.org/en/docs/db/concepts/vector-index/#vector-index-types) that scale from memory to disk.
- **Full-Text Search (FTS)**: Native keyword-based full-text search — query string fields with natural-language or structured expressions.
- **Hybrid Search**: Fuse vector similarity, full-text search, and structured filters in a single query for precise results.
- **Durable Storage**: Write-ahead logging (WAL) guarantees persistence — data is never lost, even on process crash or power failure.
- **Concurrent Access**: Multiple processes can read the same collection simultaneously; writes are single-process exclusive.
- **Runs Anywhere**: As an in-process library, Zvec runs wherever your code runs — notebooks, servers, CLI tools, or even edge devices.

## 📦 Installation

```bash
npm install @zvec/zvec
```

### ✅ Supported Platforms

- Linux (x86_64, ARM64)
- macOS (ARM64)
- Windows (x86_64)

### 🛠️ Building from Source

If you prefer to build Zvec from source, please check the [Building from Source](https://zvec.org/en/docs/db/build/) guide.

## ⚡ One-Minute Example

```typescript
import { ZVecCreateAndOpen, ZVecCollectionSchema, ZVecDataType } from "@zvec/zvec";

// Define collection schema
const schema = new ZVecCollectionSchema({
  name: "example",
  vectors: { name: "embedding", dataType: ZVecDataType.VECTOR_FP32, dimension: 4 },
});

// Create collection
const collection = ZVecCreateAndOpen("./zvec_example", schema);

// Insert documents
collection.insertSync([
  { id: "doc_1", vectors: { embedding: [0.1, 0.2, 0.3, 0.4] } },
  { id: "doc_2", vectors: { embedding: [0.2, 0.3, 0.4, 0.1] } },
]);

// Search by vector similarity
const results = collection.querySync({
  fieldName: "embedding",
  vector: [0.4, 0.3, 0.3, 0.1],
  topk: 10,
});

// Results: array of { id, score, vectors, fields }, sorted by relevance
console.log(results);
```

## 📈 Performance at Scale

Zvec delivers exceptional speed and efficiency, making it ideal for demanding production workloads.

<img src="https://zvec.oss-cn-hongkong.aliyuncs.com/qps_10M.svg" width="800" alt="Zvec Performance Benchmarks" />

For detailed benchmark methodology, configurations, and complete results, see the [Benchmarks documentation](https://zvec.org/en/docs/db/benchmarks/).

## ❤️ Contributing

We welcome and appreciate contributions from the community! Whether you're fixing a bug, adding a feature, or improving documentation, your help makes Zvec better for everyone.

Check out our [Contributing Guide](./CONTRIBUTING.md) to get started.
