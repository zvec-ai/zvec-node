#!/usr/bin/env node


const { execSync } = require('child_process');
const fs = require('fs');
const path = require('path');
const readline = require('readline');


const shouldForce = process.argv.includes('-f') || process.argv.includes('--force');


const rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout
});


function publish(packageName, version, packageDir) {
  execSync('npm publish --access public', { stdio: 'inherit', cwd: packageDir });
  console.log(`\n✅ Success! ${packageName}@${version} has been published.`);
}


try {
  // Path variables
  const PACKAGE_ROOT = path.resolve(__dirname, '..');
  const binaryPath = path.join(PACKAGE_ROOT, 'zvec_node_binding.node');
  const packageJsonPath = path.join(PACKAGE_ROOT, 'package.json');
  const packageJson = JSON.parse(fs.readFileSync(packageJsonPath, 'utf8'));

  if (fs.existsSync(binaryPath)) {
    console.warn(`⚠️ Found stray binary: ${binaryPath}`);
    fs.unlinkSync(binaryPath);
    console.log(`✅ Binary ${binaryPath} removed to prevent accidental inclusion in the generic package...`);
  }

  if (shouldForce) {
    console.log('\n🤖 --force flag detected. Skipping confirmation...');
    publish(packageJson.name, packageJson.version, PACKAGE_ROOT);
  } else {
    console.log('\n==================================');
    console.log('📦 PUBLISH PREVIEW');
    console.log(`Package: ${packageJson.name}`);
    console.log(`Version: ${packageJson.version}`);
    console.log('==================================');
    rl.question('\n🚀 Do you want to publish this version? [y/N]: ', (answer) => {
      if (answer.toLowerCase().trim() === 'y' || answer.toLowerCase().trim() === 'yes') {
        publish(packageJson.name, packageJson.version, PACKAGE_ROOT);
        rl.close();
      } else {
        console.log('\n🛑 Publish cancelled by user.');
        rl.close();
        process.exit(0);
      }
    });
  }
} catch (error) {
  console.error('❌ Error during publish:', error.message);
  rl.close();
  process.exit(1);
}