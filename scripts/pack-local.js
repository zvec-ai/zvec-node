#!/usr/bin/env node


const { execSync } = require('child_process');
const fs = require('fs');
const path = require('path');


try {
  const PACKAGE_ROOT = path.resolve(__dirname, '..');

  // Build the bindings
  execSync('npm run build', { stdio: 'inherit', cwd: PACKAGE_ROOT });

  // Verify the platform package exists
  const platform = process.platform;
  const arch = process.arch;
  const platformPackageDir = path.join(PACKAGE_ROOT, 'packages', `bindings-${platform}-${arch}`);
  const targetPath = path.join(platformPackageDir, 'zvec_node_binding.node');
  if (!fs.existsSync(targetPath)) {
    throw new Error(`Platform package does not exist: ${targetPath}`);
  }

  // Copy the binary to the root directory
  const destPath = path.join(PACKAGE_ROOT, 'zvec_node_binding.node');
  fs.copyFileSync(targetPath, destPath);
  console.log(`Binary copied from ${targetPath} to ${destPath}`);

  // Pack
  execSync('npm pack', { stdio: 'inherit', cwd: PACKAGE_ROOT, encoding: 'utf8' });
} catch (error) {
  console.error('❌ Error during build and packaging:', error.message);
  process.exit(1);
}