# Contributing to Zvec Node.js Binding

❤️ Thank you for your interest in contributing to the Zvec Node.js binding! This document provides the basic guidelines and instructions.

## Overview

The Zvec Node.js binding is built using [node-addon-api](https://github.com/nodejs/node-addon-api). The project architecture is as follows:

- **/src/zvec/**: Contains the core [Zvec](https://github.com/alibaba/zvec) source code (included as a Git submodule).
- **/src/binding**: Contains the binding logic.
- **/scripts**: Contains utilities for building, packing, and publishing the project.
- **/packages**: Holds the compiled binaries for specific platforms. Running `npm run build` generates a binary in this directory tailored to your current platform and architecture.

## Prerequisites

- Ensure you have **Node.js** installed on your system.
- This binding relies on **GNU Make** as the underlying build system. You must have **make** installed to compile the native components.
- This binding requires the [Zvec](https://github.com/alibaba/zvec) source code as a Git submodule. If you haven't initialized the submodules yet, run:

    ```bash
    git submodule update --init --recursive
    ```

## Installation and Setup

To set up the environment and install the binding, run:

```bash
npm install
```

This command will:

1. Fetch necessary dependencies.
1. Install pre-built binaries from optional dependencies based on your platform.
1. Ensure the resulting binary is placed in the correct location for **Node.js** to load it.

After installation, you can run a basic test to verify everything is working correctly:

```bash
npm test
```

## Development

### Developing Locally

When you modify the binding code in **/src/binding**, you can build and install your local version using:

```bash
npm run dev
```

This command will:

1. Build the native binding from source code (internally calling `npm run build`)
1. Package the resulting binary
1. Install the locally built version

> **Note**: Local Development Modifications
>
> During local development, the `dev` script will modify the **package.json** file. These modifications are intended solely for local development and testing purposes. Please ensure these changes are not included in your commits when submitting pull requests.

### Testing Your Changes

After modifying the binding code, you should run the test suite to ensure your changes don't break existing functionality:

```bash
npm run test:unit
```

### Editor Configuration

If you are using **clangd** for code intelligence, you may want to merge the compilation database. This is necessary because the project relies on external submodules, which results in separate **compile_commands.json** files.

```bash
python ./scripts/merge_compile_commands.py
```

## Local Distribution

If you prefer to build the binding from source and generate a single, self-contained package (including the pre-built binary for your current platform), use the local packing script.

```bash
npm run pack-local
```

This command will:

1. Build the native binding from source (internally calling `npm run build`).
1. Package and generate a tarball in the project root.

You can install this generated package directly in any Node.js project:

```bash
npm install ./zvec-zvec-<version>.tgz
```

## Releasing

### 1. Bump Binding Versions

```bash
npm run bump:bindings -- <patch|minor|major>
```

This bumps the version in every `packages/bindings-*/package.json`. Commit and push the change.

### 2. Publish Bindings

Trigger the **Publish to npm** workflow on GitHub Actions (`workflow_dispatch`). This builds and publishes all platform-specific binding packages.

Alternatively, you can publish the binding for the current platform manually:

```bash
npm run publish:bindings
```

### 3. Bump Main Package Version

```bash
npm run bump:main -- <patch|minor|major>
```

This syncs `optionalDependencies` in the root `package.json` to match the current binding versions, then bumps the main package version. Commit, tag, and push.

### 4. Publish Main Package

```bash
npm run publish:main
```

### Cross-Platform Considerations

Binaries built on one machine (OS/Architecture combination) are generally not compatible with other machines having different OS or architecture.

When you run `npm run build`, it creates a native binary specifically tailored for the operating system and CPU architecture of the machine where the build occurred.
