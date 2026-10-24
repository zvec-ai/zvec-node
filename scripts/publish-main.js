#!/usr/bin/env node


const { execSync } = require('child_process');
const fs = require('fs');
const path = require('path');
const readline = require('readline');


function isLocalDependency(spec) {
  if (typeof spec !== 'string') return false;
  const value = spec.trim();
  if (/^(?:(?:git\+)?file:|link:|workspace:|\.|[\\/]|~[\\/]|[a-z]:)/i.test(value)) {
    return true;
  }
  // Remote URLs, Git URLs and npm aliases are not filesystem paths.
  if (/^[a-z][a-z\d+.-]*:/i.test(value)) return false;
  if (/^[^@\s]+@[^:\s]+:.+/.test(value)) return false;
  if (/^[a-z\d-]+\/[\w.-]+(?:#[^%]*)?$/i.test(value)) return false;
  return /\.(?:tgz|tar(?:\.gz)?)$/i.test(value) || /[\\/]/.test(value);
}


function assertPublishableDependencies(packageJson, manifestPath) {
  const localDependencies = [];
  for (const field of ['dependencies', 'optionalDependencies', 'peerDependencies']) {
    for (const [name, spec] of Object.entries(packageJson[field] || {})) {
      if (isLocalDependency(spec)) localDependencies.push(`  ${field}.${name}: ${spec}`);
    }
  }
  if (localDependencies.length) {
    throw new Error(
      `Cannot publish ${manifestPath} with local dependencies:\n` +
      localDependencies.join('\n') +
      '\nRestore published versions or remote dependency references before publishing. ' +
      '--force only skips confirmation; it does not bypass this check.'
    );
  }
}


const shouldForce = process.argv.includes('-f') || process.argv.includes('--force');


const rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout
});


function publish(packageName, version, packageDir) {
  for (const runtimeAsset of [
    'zvec_node_binding.node',
    'jieba_dict'
  ]) {
    const generatedPath = path.join(packageDir, runtimeAsset);
    if (fs.existsSync(generatedPath)) {
      console.warn(`⚠️ Found stray runtime asset: ${generatedPath}`);
      fs.rmSync(generatedPath, { recursive: true, force: true });
      console.log(`✅ Removed ${generatedPath} to prevent accidental inclusion in the generic package...`);
    }
  }

  execSync('npm publish --access public', { stdio: 'inherit', cwd: packageDir });
  console.log(`\n✅ Success! ${packageName}@${version} has been published.`);
}


try {
  // Path variables
  const PACKAGE_ROOT = path.resolve(__dirname, '..');
  const packageJsonPath = path.join(PACKAGE_ROOT, 'package.json');
  const packageJson = JSON.parse(fs.readFileSync(packageJsonPath, 'utf8'));
  assertPublishableDependencies(packageJson, packageJsonPath);

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
