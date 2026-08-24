#!/usr/bin/env node


const { execSync } = require('child_process');
const fs = require('fs');
const path = require('path');
const { resolvePrebuiltTargetForPackaging } = require('../src/prebuilt');
const { stagePrebuiltArtifacts } = require('./artifacts');


try {
  const PACKAGE_ROOT = path.resolve(__dirname, '..');
  const prebuiltTarget = resolvePrebuiltTargetForPackaging();

  // Build the bindings
  execSync(`npm run build -- --target=${prebuiltTarget.target}`, {
    stdio: 'inherit',
    cwd: PACKAGE_ROOT,
  });

  // Stage and verify the prebuilt platform package.
  const staged = stagePrebuiltArtifacts(PACKAGE_ROOT, prebuiltTarget);
  const platformPackageDir = staged.targetDir;

  // Pack
  const packResult = execSync('npm pack', { cwd: platformPackageDir, encoding: 'utf8' });
  const lines = packResult.trim().split('\n');
  const tarballFilename = lines[lines.length - 1].trim();
  const tarballPath = path.join(platformPackageDir, tarballFilename);
  if (!fs.existsSync(tarballPath)) {
    throw new Error(`Tarball not found: ${tarballPath}`);
  }

  // Install from local package
  execSync(`npm install ${tarballPath}`, { stdio: 'inherit', cwd: PACKAGE_ROOT });
} catch (error) {
  console.error('❌ Error during build and packaging:', error.message);
  process.exit(1);
}
