const path = require('path');
const fs = require('fs');


let binding;


try {
  const bundledBinaryPath = path.join(__dirname, '..', 'zvec_node_binding.node');
  if (fs.existsSync(bundledBinaryPath)) {   // Try bundled binary first
    binding = require(bundledBinaryPath);
  } else {  // Fall back to platform-specific prebuilt binary package
    const platformPackage = `@zvec/bindings-${process.platform}-${process.arch}`;
    binding = require(platformPackage);
  }
} catch (err) {
  throw new Error(
    `Zvec Error: Failed to load prebuilt binary for ${process.platform}-${process.arch}. ` +
    `This platform may not be supported. ` +
    `Original error: ${err.message}`
  );
}


function isZVecError(error) {
  return (
    typeof error === "object" &&
    error !== null &&
    typeof error.name === "string" &&
    typeof error.code === "string" &&
    error.code.startsWith("ZVEC_")
  );
}


module.exports = {
  ZVecDataType: binding.DataType,
  ZVecIndexType: binding.IndexType,
  ZVecMetricType: binding.MetricType,
  ZVecQuantizeType: binding.QuantizeType,
  ZVecLogType: binding.LogType,
  ZVecLogLevel: binding.LogLevel,
  ZVecCollectionSchema: binding.CollectionSchema,
  ZVecInitialize: binding.initialize,
  ZVecSetDefaultJiebaDictDir: binding.setDefaultJiebaDictDir,
  ZVecGetDefaultJiebaDictDir: binding.getDefaultJiebaDictDir,
  ZVecCreateAndOpen: binding.createAndOpen,
  ZVecOpen: binding.open,
  isZVecError,
};
