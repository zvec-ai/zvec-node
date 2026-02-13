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


function publish(packageName, version, platformPackageDir) {
  execSync('npm publish --access public', { stdio: 'inherit', cwd: platformPackageDir });
  console.log(`\n✅ Success! ${packageName}@${version} has been published.`);
}


try {
  // Path variables
  const PACKAGE_ROOT = path.resolve(__dirname, '..');
  const platform = process.platform;
  const arch = process.arch;
  const platformPackageDir = path.join(PACKAGE_ROOT, 'packages', `bindings-${platform}-${arch}`);
  const targetPath = path.join(platformPackageDir, 'zvec_node_binding.node');
  const packageJsonPath = path.join(platformPackageDir, 'package.json');
  const packageJson = JSON.parse(fs.readFileSync(packageJsonPath, 'utf8'));

  // Build the bindings
  execSync('npm run build', { stdio: 'inherit', cwd: PACKAGE_ROOT });

  // Verify the platform package exists
  if (!fs.existsSync(targetPath)) {
    throw new Error(`Platform package does not exist: ${targetPath}`);
  }

  if (shouldForce) {
    console.log('\n🤖 --force flag detected. Skipping confirmation...');
    publish(packageJson.name, packageJson.version, platformPackageDir);
  } else {
    console.log('\n==================================');
    console.log('📦 PUBLISH PREVIEW');
    console.log(`Package: ${packageJson.name}`);
    console.log(`Version: ${packageJson.version}`);
    console.log(`Target : ${platformPackageDir}`);
    console.log('==================================');
    rl.question('\n🚀 Do you want to publish this version? [y/N]: ', (answer) => {
      if (answer.toLowerCase().trim() === 'y' || answer.toLowerCase().trim() === 'yes') {
        publish(packageJson.name, packageJson.version, platformPackageDir);
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