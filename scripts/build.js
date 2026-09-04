#!/usr/bin/env node


const { execSync } = require('child_process');
const os = require('os');
const path = require('path');
const { verifyBuildArtifacts } = require('./artifacts');
const {
  readExpectedPrebuiltTarget,
  resolvePrebuiltTargetForPackaging,
} = require('../src/prebuilt');


// Build variables
const IS_WINDOWS = process.platform === 'win32';
const JOBS = os.cpus().length;
const CMAKE_GENERATOR = process.env.ZVEC_CMAKE_GENERATOR;
const BUILD_TYPE = process.argv.includes('--debug') ? 'Debug'
  : process.argv.includes('--release') ? 'Release'
    : process.env.BUILD_TYPE || process.env.npm_config_build_type || 'Release';


// Path variables
const PACKAGE_ROOT = path.resolve(__dirname, '..');
const BUILD_DIR = path.join(PACKAGE_ROOT, 'build');
console.log(`Building Zvec Node.js binding ...`);


try {
  if (CMAKE_GENERATOR !== undefined && CMAKE_GENERATOR !== 'Ninja') {
    throw new Error('ZVEC_CMAKE_GENERATOR only supports "Ninja".');
  }

  const expectedPrebuiltTarget = readExpectedPrebuiltTarget();
  if (expectedPrebuiltTarget !== null) {
    const prebuiltTarget = resolvePrebuiltTargetForPackaging();
    console.log(`Validated prebuilt build target: ${prebuiltTarget.target}.`);
  }

  // Compile
  const cmdParts = ['cmake-js'];
  if (CMAKE_GENERATOR === 'Ninja') {
    cmdParts.push('--generator=Ninja');
  } else if (!IS_WINDOWS) {
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
  const artifacts = verifyBuildArtifacts(PACKAGE_ROOT, BUILD_TYPE);
  console.log(`✅ Native addon compiled at: ${artifacts.binaryPath}`);
} catch (error) {
  console.error('❌ Error during build:', error.message);
  process.exit(1);
}
