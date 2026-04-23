# Zvec Node.js Binding

<p align="left">
  <a href="https://zvec.org/en/docs/db/quickstart/">🚀 <strong>Quickstart</strong> </a> |
  <a href="https://zvec.org">🏠 <strong>Home</strong> </a> |
  <a href="https://zvec.org/en/docs/db/">📚 <strong>Docs</strong> </a> |
  <a href="https://discord.gg/rKddFBBu9z">🎮 <strong>Discord</strong> </a> |
  <a href="https://x.com/ZvecAI">🐦 <strong>X (Twitter)</strong> </a>
</p>

A Node.js binding for [Zvec](https://github.com/alibaba/zvec) —— a lightweight, lightning-fast, in-process vector database.

## 💫 Features

- **Blazing Fast**: Searches billions of vectors in milliseconds.
- **Simple, Just Works**: Install with `npm i @zvec/zvec` and start searching in seconds. Pure local, no servers, no config, no fuss.
- **Dense + Sparse Vectors**: Work with both dense and sparse embeddings, with native support for multi-vector queries in a single call.
- **Hybrid Search**: Combine semantic similarity with structured filters for precise results.
- **Durable Storage**: Write-ahead logging (WAL) guarantees persistence — data is never lost, even on process crash or power failure.
- **Concurrent Access**: Multiple processes can read the same collection simultaneously; writes are single-process exclusive.
- **Runs Anywhere**: As an in-process library, Zvec runs wherever your code runs — notebooks, servers, CLI tools, or even edge devices.

## 📦 Installation

Install the package using npm:

```bash
npm install @zvec/zvec
```

### 🖥️ Supported Platforms

- Linux (x86_64/ARM64)
- macOS (ARM64)
- Windows (x86_64)

### 🛠️ Building from Source

If you prefer to build Zvec from source, please check the [Building from Source](https://zvec.org/en/docs/db/build/) guide.
