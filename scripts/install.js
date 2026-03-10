#!/usr/bin/env node

const path = require('path');
const fs = require('fs');

const platform = process.platform;
const arch = process.arch;
const packageName = `@zvec/bindings-${platform}-${arch}`;

// Check if binary was bundled during tarball installation
const bundledBinaryPath = path.join(__dirname, '..', 'zvec_node_binding.node');
if (fs.existsSync(bundledBinaryPath)) {
  process.exit(0);
}

// Try to resolve prebuilt binary from optional dependencies
try {
  require.resolve(packageName);
} catch (error) {
  console.error(`❌ zvec Error: Failed to install prebuilt binary for ${platform}-${arch}`);
  process.exit(1);
}