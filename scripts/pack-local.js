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

  // DiskANN currently ships as a runtime-loaded plugin. Local packs bundle it
  // beside the addon binary so the C++ engine can auto-load it when needed.
  const pluginPath = path.join(platformPackageDir, 'libzvec_diskann_plugin.so');
  if (fs.existsSync(pluginPath)) {
    const pluginDestPath = path.join(PACKAGE_ROOT, 'libzvec_diskann_plugin.so');
    fs.copyFileSync(pluginPath, pluginDestPath);
    console.log(`DiskANN plugin copied from ${pluginPath} to ${pluginDestPath}`);
  }

  // Temporarily remove optionalDependencies from package.json
  // (local pack bundles the binary directly, no need for platform packages)
  const packageJsonPath = path.join(PACKAGE_ROOT, 'package.json');
  const originalPackageJson = fs.readFileSync(packageJsonPath, 'utf8');
  const pkg = JSON.parse(originalPackageJson);
  delete pkg.optionalDependencies;
  fs.writeFileSync(packageJsonPath, JSON.stringify(pkg, null, 2) + '\n');

  try {
    // Pack
    execSync('npm pack', { stdio: 'inherit', cwd: PACKAGE_ROOT, encoding: 'utf8' });
  } finally {
    // Restore original package.json
    fs.writeFileSync(packageJsonPath, originalPackageJson);
  }
} catch (error) {
  console.error('❌ Error during build and packaging:', error.message);
  process.exit(1);
}
