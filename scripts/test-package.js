#!/usr/bin/env node

const { execFileSync } = require('child_process');
const fs = require('fs');
const os = require('os');
const path = require('path');
const { createRequire } = require('module');
const { resolvePrebuiltTarget } = require('../src/prebuilt');

const packageRoot = path.resolve(__dirname, '..');
const packageName = '@zvec/zvec';
const readJson = (filename) => JSON.parse(fs.readFileSync(filename, 'utf8'));
const writeJson = (filename, value) =>
  fs.writeFileSync(filename, JSON.stringify(value, null, 2) + '\n');

function assertInside(directory, filename) {
  const relative = path.relative(fs.realpathSync(directory), fs.realpathSync(filename));
  if (relative === '..' || relative.startsWith(`..${path.sep}`) || path.isAbsolute(relative)) {
    throw new Error(`Expected an installed file inside ${directory}, received ${filename}.`);
  }
}

// Rewrite only the copied tests so they load the installed package.
function patchTestImports(directory) {
  for (const entry of fs.readdirSync(directory, { withFileTypes: true })) {
    const filename = path.join(directory, entry.name);
    if (entry.isDirectory()) {
      patchTestImports(filename);
    } else if (filename.endsWith('.ts') || filename.endsWith('.js')) {
      const content = fs.readFileSync(filename, 'utf8');
      const sourcePath = String.raw`(?:\.\.\/)+src(?:\/index)?(?:\.[cm]?[jt]s)?`;
      const patched = content
        .replace(
          new RegExp(String.raw`from\s+['"]${sourcePath}['"]`, 'g'),
          `from '${packageName}'`
        )
        .replace(
          new RegExp(String.raw`require\(\s*['"]${sourcePath}['"]\s*\)`, 'g'),
          `require('${packageName}')`
        );
      if (patched !== content) fs.writeFileSync(filename, patched);
    }
  }
}

let consumerDir;
let passed = false;

try {
  const npmCli = process.env.npm_execpath;
  if (!npmCli || !fs.existsSync(npmCli)) {
    throw new Error('Run this command with npm run test:package -- [package-spec].');
  }
  if (process.argv.length > 3) {
    throw new Error('Usage: npm run test:package -- [package-spec-or-tarball]');
  }

  let packageSpec = process.argv[2] || `${packageName}@latest`;
  if (/^\s*file:/i.test(packageSpec)) {
    throw new Error('Use a .tgz path without the file: prefix, e.g. ./package.tgz.');
  }
  if (fs.existsSync(packageSpec)) {
    if (!fs.statSync(packageSpec).isFile() || !packageSpec.endsWith('.tgz')) {
      throw new Error('Use an npm package spec or a packed .tgz file.');
    }
    packageSpec = path.resolve(packageSpec);
  }

  consumerDir = fs.mkdtempSync(path.join(os.tmpdir(), 'zvec-package-test-'));
  console.log(`Testing ${packageSpec} in ${consumerDir}`);

  const env = { ...process.env };
  delete env.NODE_PATH;
  const runNode = (args) => execFileSync(process.execPath, args, {
    cwd: consumerDir,
    env,
    stdio: 'inherit',
  });
  const runNpm = (args) => runNode([npmCli, ...args]);

  writeJson(path.join(consumerDir, 'package.json'), {
    name: 'zvec-package-test',
    version: '0.0.0',
    private: true,
    type: 'commonjs',
  });

  // Only the package under test is installed in the consumer. Its install script
  // runs normally, and its exact resolved dependency tree is recorded locally.
  runNpm(['install', '--save-exact', '--no-audit', '--no-fund', packageSpec]);
  runNpm(['ls', packageName]);

  const consumerRequire = createRequire(path.join(consumerDir, 'package.json'));
  const installedPackageDir = path.join(consumerDir, 'node_modules', '@zvec', 'zvec');
  const entry = consumerRequire.resolve(packageName);
  assertInside(consumerDir, installedPackageDir);
  assertInside(installedPackageDir, entry);
  console.log(`Package entry: ${entry}`);

  const installedManifest = readJson(path.join(installedPackageDir, 'package.json'));
  const target = resolvePrebuiltTarget();
  if (installedManifest.optionalDependencies?.[target.packageName]) {
    const binding = createRequire(entry).resolve(target.packageName);
    assertInside(consumerDir, binding);
    console.log(`Platform binding (${target.target}): ${binding}`);
  }

  runNode(['-e', `require(${JSON.stringify(packageName)})`]);
  runNode(['--input-type=module', '-e', `await import(${JSON.stringify(packageName)})`]);

  fs.cpSync(path.join(packageRoot, 'tests'), path.join(consumerDir, 'tests'), {
    recursive: true,
  });
  patchTestImports(path.join(consumerDir, 'tests'));

  // Reuse the repository's locked tools through explicit paths. Tests resolve
  // @zvec/zvec from the consumer; no source files or shared node_modules are copied.
  const tsconfig = readJson(path.join(packageRoot, 'tsconfig.json'));
  tsconfig.compilerOptions.typeRoots = [path.join(packageRoot, 'node_modules', '@types')];
  tsconfig.include = ['tests/**/*.ts'];
  writeJson(path.join(consumerDir, 'tsconfig.json'), tsconfig);

  const jestConfig = readJson(path.join(packageRoot, 'jest.config.json'));
  jestConfig.rootDir = consumerDir;
  jestConfig.testEnvironment = require.resolve('jest-environment-node');
  jestConfig.transform = Object.fromEntries(
    Object.entries(jestConfig.transform).map(([pattern, transformer]) => [
      pattern,
      Array.isArray(transformer)
        ? [require.resolve(transformer[0]), ...transformer.slice(1)]
        : require.resolve(transformer),
    ])
  );
  const configPath = path.join(consumerDir, 'jest.config.json');
  writeJson(configPath, jestConfig);
  runNode([require.resolve('jest/bin/jest'), '--config', configPath, '--runInBand']);
  passed = true;
} catch (error) {
  console.error(`Package test failed: ${error.message}`);
  if (consumerDir) console.error(`Test project retained at: ${consumerDir}`);
  process.exitCode = 1;
} finally {
  if (passed) {
    fs.rmSync(consumerDir, { recursive: true, force: true });
  }
}
