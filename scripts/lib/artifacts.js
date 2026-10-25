// Locate, verify, and stage native build artifacts for local and platform packages.
const fs = require('fs');
const path = require('path');


const BINARY_FILENAME = 'zvec_node_binding.node';


function getBuildOutputDir(packageRoot, buildType = 'Release') {
  const buildSubdir = buildType.charAt(0).toUpperCase() + buildType.slice(1);
  return path.join(packageRoot, 'build', buildSubdir);
}


function getBuildArtifacts(packageRoot, buildType = 'Release') {
  const outputDir = getBuildOutputDir(packageRoot, buildType);
  return {
    outputDir,
    binaryPath: path.join(outputDir, BINARY_FILENAME),
  };
}


function verifyBuildArtifacts(packageRoot, buildType = 'Release') {
  const artifacts = getBuildArtifacts(packageRoot, buildType);
  if (!fs.existsSync(artifacts.binaryPath)) {
    throw new Error(`Binary not found at ${artifacts.binaryPath}`);
  }
  return artifacts;
}


function copyBuildArtifacts(packageRoot, targetDir, buildType = 'Release') {
  const artifacts = verifyBuildArtifacts(packageRoot, buildType);
  fs.mkdirSync(targetDir, { recursive: true });

  const binaryPath = path.join(targetDir, BINARY_FILENAME);
  fs.copyFileSync(artifacts.binaryPath, binaryPath);

  return {
    targetDir,
    binaryPath,
  };
}


function stageLocalArtifacts(packageRoot, buildType = 'Release') {
  return copyBuildArtifacts(packageRoot, packageRoot, buildType);
}


function stagePrebuiltArtifacts(packageRoot, prebuiltTarget, buildType = 'Release') {
  const targetDir = path.join(packageRoot, 'packages', prebuiltTarget.packageDirName);
  const packageJsonPath = path.join(targetDir, 'package.json');
  if (!fs.existsSync(packageJsonPath)) {
    throw new Error(`Prebuilt package directory does not exist: ${targetDir}.`);
  }
  return copyBuildArtifacts(packageRoot, targetDir, buildType);
}


module.exports = {
  BINARY_FILENAME,
  verifyBuildArtifacts,
  stageLocalArtifacts,
  stagePrebuiltArtifacts,
};
