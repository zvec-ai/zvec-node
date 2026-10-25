#!/usr/bin/env node


const { execSync } = require('child_process');
const fs = require('fs');
const path = require('path');
const {
  BINARY_FILENAME,
  stageLocalArtifacts,
} = require('./lib/artifacts');


const PACKAGE_ROOT = path.resolve(__dirname, '..');


try {
  execSync('npm run build:release', { stdio: 'inherit', cwd: PACKAGE_ROOT });

  const packageJsonPath = path.join(PACKAGE_ROOT, 'package.json');
  const originalPackageJson = fs.readFileSync(packageJsonPath, 'utf8');
  const pkg = JSON.parse(originalPackageJson);
  const binaryPath = path.join(PACKAGE_ROOT, BINARY_FILENAME);
  const backupDir = fs.mkdtempSync(path.join(PACKAGE_ROOT, '.zvec-pack-local-'));
  const backupPath = path.join(backupDir, BINARY_FILENAME);
  let binaryPrepared = false;

  try {
    if (fs.existsSync(binaryPath)) {
      fs.renameSync(binaryPath, backupPath);
    }
    binaryPrepared = true;

    const staged = stageLocalArtifacts(PACKAGE_ROOT);
    console.log(`Binary staged for local package at ${staged.binaryPath}`);

    delete pkg.optionalDependencies;
    fs.writeFileSync(packageJsonPath, JSON.stringify(pkg, null, 2) + '\n');
    execSync('npm pack', { stdio: 'inherit', cwd: PACKAGE_ROOT });
  } finally {
    fs.writeFileSync(packageJsonPath, originalPackageJson);
    if (binaryPrepared) {
      fs.rmSync(binaryPath, { force: true });
      if (fs.existsSync(backupPath)) {
        fs.renameSync(backupPath, binaryPath);
      }
    }
    fs.rmSync(backupDir, { recursive: true, force: true });
  }
} catch (error) {
  console.error('❌ Error during build and packaging:', error.message);
  process.exitCode = 1;
}
