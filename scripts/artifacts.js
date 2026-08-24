const fs = require('fs');
const path = require('path');


const BINARY_FILENAME = 'zvec_node_binding.node';
const JIEBA_DICT_DIRNAME = 'jieba_dict';
const JIEBA_DICT_FILES = [
  'jieba.dict.utf8',
  'hmm_model.utf8',
];


function getBuildOutputDir(packageRoot, buildType = 'Release') {
  const buildSubdir = buildType.charAt(0).toUpperCase() + buildType.slice(1);
  return path.join(packageRoot, 'build', buildSubdir);
}


function getBuildArtifacts(packageRoot, buildType = 'Release') {
  const outputDir = getBuildOutputDir(packageRoot, buildType);
  return {
    outputDir,
    binaryPath: path.join(outputDir, BINARY_FILENAME),
    jiebaDictDir: path.join(outputDir, JIEBA_DICT_DIRNAME),
  };
}


function verifyBuildArtifacts(packageRoot, buildType = 'Release') {
  const artifacts = getBuildArtifacts(packageRoot, buildType);
  if (!fs.existsSync(artifacts.binaryPath)) {
    throw new Error(`Binary not found at ${artifacts.binaryPath}`);
  }
  for (const filename of JIEBA_DICT_FILES) {
    const filePath = path.join(artifacts.jiebaDictDir, filename);
    if (!fs.existsSync(filePath)) {
      throw new Error(`Jieba dictionary file not found at ${filePath}`);
    }
  }
  return artifacts;
}


function copyBuildArtifacts(packageRoot, targetDir, buildType = 'Release') {
  const artifacts = verifyBuildArtifacts(packageRoot, buildType);
  fs.mkdirSync(targetDir, { recursive: true });

  const binaryPath = path.join(targetDir, BINARY_FILENAME);
  const jiebaDictDir = path.join(targetDir, JIEBA_DICT_DIRNAME);
  fs.copyFileSync(artifacts.binaryPath, binaryPath);
  fs.cpSync(artifacts.jiebaDictDir, jiebaDictDir, {
    recursive: true,
    force: true,
  });

  return {
    targetDir,
    binaryPath,
    jiebaDictDir,
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
  JIEBA_DICT_DIRNAME,
  getBuildArtifacts,
  verifyBuildArtifacts,
  stageLocalArtifacts,
  stagePrebuiltArtifacts,
};
