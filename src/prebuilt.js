const { familySync, GLIBC, MUSL } = require('detect-libc');


const PREBUILT_TARGETS = Object.freeze([
  'darwin-arm64',
  'darwin-x64',
  'linux-arm64',
  'linux-arm64-musl',
  'linux-x64',
  'linux-x64-musl',
  'win32-arm64',
  'win32-x64',
]);
const PREBUILT_TARGET_SET = new Set(PREBUILT_TARGETS);


function detectLinuxLibc() {
  try {
    const family = familySync();
    return family === GLIBC || family === MUSL ? family : null;
  } catch {
    return null;
  }
}


function normalizeLinuxLibc(libc) {
  if (typeof libc !== 'string') {
    throw new TypeError(`Linux libc must be "${GLIBC}" or "${MUSL}".`);
  }

  const normalized = libc.toLowerCase();
  if (normalized !== GLIBC && normalized !== MUSL) {
    throw new Error(`Unsupported Linux libc: ${libc}. Expected "${GLIBC}" or "${MUSL}".`);
  }
  return normalized;
}


function resolvePrebuiltTarget(options = {}) {
  const platform = options.platform ?? process.platform;
  const arch = options.arch ?? process.arch;
  const platformArch = `${platform}-${arch}`;

  if (platform !== 'linux' && options.libc !== undefined) {
    throw new Error(`A libc target can only be specified for Linux, not ${platform}.`);
  }

  let libc = null;
  let libcSource = null;
  if (platform === 'linux') {
    if (options.libc !== undefined) {
      libc = normalizeLinuxLibc(options.libc);
      libcSource = 'explicit';
    } else {
      const detectedLibc = detectLinuxLibc();
      libc = detectedLibc ?? GLIBC;
      libcSource = detectedLibc === null ? 'fallback' : 'detected';
    }
  }

  const target = platform === 'linux' && libc === MUSL
    ? `${platformArch}-musl`
    : platformArch;

  if (!PREBUILT_TARGET_SET.has(target)) {
    throw new Error(`No prebuilt binding is published for ${target}.`);
  }

  return Object.freeze({
    platform,
    arch,
    libc,
    libcSource,
    platformArch,
    target,
    packageName: `@zvec/bindings-${target}`,
    packageDirName: `bindings-${target}`,
  });
}


function readExpectedPrebuiltTarget(argv = process.argv.slice(2)) {
  let expectedTarget = null;

  for (let index = 0; index < argv.length; index += 1) {
    const argument = argv[index];
    let value = null;

    if (argument === '--target') {
      value = argv[index + 1];
      index += 1;
    } else if (argument.startsWith('--target=')) {
      value = argument.slice('--target='.length);
    } else {
      continue;
    }

    if (!value || value.startsWith('--')) {
      throw new Error('Expected a prebuilt target after --target.');
    }
    if (expectedTarget !== null) {
      throw new Error('Prebuilt target may only be specified once.');
    }
    if (!PREBUILT_TARGET_SET.has(value)) {
      throw new Error(`No prebuilt binding is published for ${value}.`);
    }
    expectedTarget = value;
  }

  return expectedTarget;
}


function resolvePrebuiltTargetForPackaging(options = {}) {
  const {
    argv = process.argv.slice(2),
    ...targetOptions
  } = options;
  const prebuiltTarget = resolvePrebuiltTarget(targetOptions);
  const expectedTarget = readExpectedPrebuiltTarget(argv);

  if (expectedTarget !== null && expectedTarget !== prebuiltTarget.target) {
    throw new Error(
      `Prebuilt target mismatch: expected ${expectedTarget}, ` +
      `but the current environment is ${prebuiltTarget.target}.`
    );
  }

  return prebuiltTarget;
}


module.exports = {
  PREBUILT_TARGETS,
  detectLinuxLibc,
  readExpectedPrebuiltTarget,
  resolvePrebuiltTarget,
  resolvePrebuiltTargetForPackaging,
};
