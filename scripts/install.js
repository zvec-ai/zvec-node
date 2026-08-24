#!/usr/bin/env node

const path = require('path');
const fs = require('fs');
const { resolvePrebuiltTarget } = require('../src/prebuilt');

const packageRoot = path.resolve(__dirname, '..');
let prebuiltTarget = null;

// Check if binary was bundled during tarball installation
const bundledBinaryPath = path.join(packageRoot, 'zvec_node_binding.node');
if (fs.existsSync(bundledBinaryPath)) {
  process.exit(0);
}

// Try to resolve prebuilt binary from optional dependencies
try {
  prebuiltTarget = resolvePrebuiltTarget();
  require.resolve(prebuiltTarget.packageName);
} catch (error) {
  const target = prebuiltTarget?.target ?? `${process.platform}-${process.arch}`;
  const isSourceCheckout =
    fs.existsSync(path.join(packageRoot, 'CMakeLists.txt')) &&
    fs.existsSync(path.join(packageRoot, 'src', 'binding', 'addon.cc')) &&
    fs.existsSync(path.join(packageRoot, 'src', 'zvec', 'CMakeLists.txt'));

  if (isSourceCheckout) {
    console.warn(
      `⚠️ zvec: No prebuilt binding is installed for ${target}. ` +
      'Source checkout detected; run "npm run pack-local" to build an installable local package ' +
      '("npm run build" only compiles the addon).'
    );
    process.exit(0);
  }

  console.error(
    `❌ zvec Error: Failed to install prebuilt binary for ${target}. ` +
    error.message
  );
  process.exit(1);
}
