const fs = require('fs');
const path = require('path');
const { PREBUILT_TARGETS } = require('../src/prebuilt');


function getBindingPackages(packageRoot) {
  return PREBUILT_TARGETS.map((target) => {
    const packageDir = path.join(packageRoot, 'packages', `bindings-${target}`);
    const packageJsonPath = path.join(packageDir, 'package.json');
    if (!fs.existsSync(packageJsonPath)) {
      throw new Error(`Missing prebuilt package manifest: ${packageJsonPath}.`);
    }

    const packageJson = JSON.parse(fs.readFileSync(packageJsonPath, 'utf8'));
    const expectedName = `@zvec/bindings-${target}`;
    if (packageJson.name !== expectedName) {
      throw new Error(
        `Unexpected prebuilt package name in ${packageJsonPath}: ` +
        `expected ${expectedName}, received ${packageJson.name}.`
      );
    }

    return {
      target,
      packageDir,
      packageJsonPath,
      packageJson,
    };
  });
}


function assertUniformBindingVersion(bindingPackages) {
  if (bindingPackages.length === 0) {
    throw new Error('No prebuilt binding packages are configured.');
  }

  const expectedVersion = bindingPackages[0].packageJson.version;
  const mismatches = bindingPackages.filter(
    ({ packageJson }) => packageJson.version !== expectedVersion
  );
  if (mismatches.length > 0) {
    const versions = bindingPackages
      .map(({ packageJson }) => `${packageJson.name}@${packageJson.version}`)
      .join(', ');
    throw new Error(`Prebuilt binding package versions are not aligned: ${versions}.`);
  }

  return expectedVersion;
}


function syncBindingOptionalDependencies(rootPackageJson, bindingPackages) {
  const optionalDependencies = Object.fromEntries(
    Object.entries(rootPackageJson.optionalDependencies || {}).filter(
      ([name]) => !name.startsWith('@zvec/bindings-')
    )
  );

  for (const { packageJson } of bindingPackages) {
    optionalDependencies[packageJson.name] = packageJson.version;
  }

  rootPackageJson.optionalDependencies = optionalDependencies;
  return rootPackageJson;
}


module.exports = {
  assertUniformBindingVersion,
  getBindingPackages,
  syncBindingOptionalDependencies,
};
