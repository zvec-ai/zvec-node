const path = require('path');
const fs = require('fs');
const { resolvePrebuiltTarget } = require('./prebuilt');


let binding;
let prebuiltTarget = null;


try {
  const bundledBinaryPath = path.join(__dirname, '..', 'zvec_node_binding.node');
  if (fs.existsSync(bundledBinaryPath)) {   // Try bundled binary first
    binding = require(bundledBinaryPath);
  } else {  // Fall back to platform-specific prebuilt binary package
    prebuiltTarget = resolvePrebuiltTarget();
    const platformPackagePath = require.resolve(prebuiltTarget.packageName);
    binding = require(platformPackagePath);
  }
} catch (err) {
  const target = prebuiltTarget?.target ?? `${process.platform}-${process.arch}`;
  throw new Error(
    `Zvec Error: Failed to load prebuilt binary for ${target}. ` +
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
  ZVecIOBackendType: binding.IOBackendType,
  ZVecCollectionSchema: binding.CollectionSchema,
  ZVecInitialize: binding.initialize,
  ZVecGetIOBackendType: binding.getIOBackendType,
  ZVecGetIOBackendDescription: binding.getIOBackendDescription,
  ZVecCreateAndOpen: binding.createAndOpen,
  ZVecOpen: binding.open,
  isZVecError,
};
