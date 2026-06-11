#!/usr/bin/env node


const { execSync } = require('child_process');
const fs = require('fs');
const os = require('os');
const path = require('path');


// Build variables
const IS_WINDOWS = process.platform === 'win32';
const JOBS = os.cpus().length;
const BUILD_TYPE = process.argv.includes('--debug') ? 'Debug'
  : process.argv.includes('--release') ? 'Release'
    : process.env.BUILD_TYPE || process.env.npm_config_build_type || 'Release';


// Path variables
const PACKAGE_ROOT = path.resolve(__dirname, '..');
const BUILD_DIR = path.join(PACKAGE_ROOT, 'build');
const BUILD_SUBDIR = BUILD_TYPE.charAt(0).toUpperCase() + BUILD_TYPE.slice(1);
const BUILD_TARGET_DIR = path.join(BUILD_DIR, BUILD_SUBDIR);


console.log(`Building and packaging Zvec Node.js binding ...`);


try {
  // Compile
  const cmdParts = ['cmake-js'];
  if (!IS_WINDOWS) {
    cmdParts.push('--prefer-make');
  }
  cmdParts.push(`--out=${BUILD_DIR}`);
  cmdParts.push(`--parallel=${JOBS}`);
  cmdParts.push('compile');
  cmdParts.push(`--CD=CMAKE_BUILD_TYPE=${BUILD_TYPE}`);
  const cmd = cmdParts.join(' ');

  console.log(`\nCompiling native addon. Jobs: ${JOBS}, Build type: ${BUILD_TYPE}.`);
  console.log(`Running command: ${cmd}\n`);
  execSync(cmd, { stdio: 'inherit', cwd: PACKAGE_ROOT });
  const binaryPath = path.join(BUILD_TARGET_DIR, 'zvec_node_binding.node');
  if (!fs.existsSync(binaryPath)) {
    throw new Error(`Binary not found at ${binaryPath}`);
  }

  // Package
  const platform = process.platform;
  const arch = process.arch;
  const platformPackageName = `@zvec/bindings-${platform}-${arch}`;
  const platformPackageDir = path.join(PACKAGE_ROOT, 'packages', `bindings-${platform}-${arch}`);
  if (!fs.existsSync(platformPackageDir)) {
    throw new Error(`Platform package directory does not exist: ${platformPackageDir}. This platform (${platform}-${arch}) may not be supported.`);
  }
  const targetPath = path.join(platformPackageDir, 'zvec_node_binding.node');
  fs.copyFileSync(binaryPath, targetPath);

  // CMake stages jieba_dict next to the addon; keep that runtime layout in the
  // platform package so the JS wrapper can register it on load.
  const jiebaDictPath = path.join(BUILD_TARGET_DIR, 'jieba_dict');
  if (!fs.existsSync(jiebaDictPath)) {
    throw new Error(`Jieba dictionary directory not found at ${jiebaDictPath}`);
  }
  fs.cpSync(jiebaDictPath, path.join(platformPackageDir, 'jieba_dict'), {
    recursive: true,
    force: true
  });

  // DiskANN currently ships as a runtime-loaded plugin. Keep it next to the
  // addon binary so the C++ engine can auto-load it when DiskANN is used.
  const diskAnnPluginPath = path.join(BUILD_TARGET_DIR, 'libzvec_diskann_plugin.so');
  if (fs.existsSync(diskAnnPluginPath)) {
    fs.copyFileSync(
      diskAnnPluginPath,
      path.join(platformPackageDir, 'libzvec_diskann_plugin.so')
    );
  }

  console.log(`✅ Binary compiled and packaged for ${platformPackageName} at: ${targetPath}`);
} catch (error) {
  console.error('❌ Error during build and packaging:', error.message);
  process.exit(1);
}
