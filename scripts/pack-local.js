#!/usr/bin/env node


const { execSync } = require('child_process');
const fs = require('fs');
const path = require('path');
const {
  BINARY_FILENAME,
  JIEBA_DICT_DIRNAME,
  stageLocalArtifacts,
} = require('./artifacts');


const PACKAGE_ROOT = path.resolve(__dirname, '..');
const packageJsonPath = path.join(PACKAGE_ROOT, 'package.json');
const originalPackageJson = fs.readFileSync(packageJsonPath, 'utf8');
const backupDir = fs.mkdtempSync(path.join(PACKAGE_ROOT, '.zvec-pack-local-'));
const bundledEntries = [BINARY_FILENAME, JIEBA_DICT_DIRNAME];


function backupBundledEntries() {
  for (const entry of bundledEntries) {
    const sourcePath = path.join(PACKAGE_ROOT, entry);
    if (fs.existsSync(sourcePath)) {
      fs.renameSync(sourcePath, path.join(backupDir, entry));
    }
  }
}


function restoreBundledEntries() {
  for (const entry of bundledEntries) {
    const targetPath = path.join(PACKAGE_ROOT, entry);
    const backupPath = path.join(backupDir, entry);
    fs.rmSync(targetPath, { recursive: true, force: true });
    if (fs.existsSync(backupPath)) {
      fs.renameSync(backupPath, targetPath);
    }
  }
  fs.rmSync(backupDir, { recursive: true, force: true });
}


try {
  // Build the bindings
  execSync('npm run build', { stdio: 'inherit', cwd: PACKAGE_ROOT });

  // A local package bundles the just-built addon directly in the main package.
  // Preserve any existing local artifacts and restore them after packing.
  backupBundledEntries();
  const staged = stageLocalArtifacts(PACKAGE_ROOT);
  console.log(`Binary staged for local package at ${staged.binaryPath}`);
  console.log(`Jieba dictionary staged at ${staged.jiebaDictDir}`);

  // Temporarily remove optionalDependencies from package.json
  // (local pack bundles the binary directly, no need for platform packages)
  const pkg = JSON.parse(originalPackageJson);
  delete pkg.optionalDependencies;
  fs.writeFileSync(packageJsonPath, JSON.stringify(pkg, null, 2) + '\n');

  // Pack
  execSync('npm pack', { stdio: 'inherit', cwd: PACKAGE_ROOT, encoding: 'utf8' });
} catch (error) {
  console.error('❌ Error during build and packaging:', error.message);
  process.exitCode = 1;
} finally {
  fs.writeFileSync(packageJsonPath, originalPackageJson);
  restoreBundledEntries();
}
