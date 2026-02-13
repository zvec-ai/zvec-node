let binding;


try {
  const platformPackage = `@zvec/bindings-${process.platform}-${process.arch}`;
  binding = require(platformPackage);
} catch (err) {
  throw new Error(
    `zvec Error: Prebuilt binary not found for ${process.platform}-${process.arch}. ` +
    `This platform may not be supported. `
  );
}


binding.Collection.prototype.querySync = function (queryObj) {
  if (arguments.length !== 1) {
    const err = new Error("Collection.querySync(): Expected exactly 1 argument. Argument must be an Query object");
    err.name = "InvalidArgumentError";
    err.code = "ZVEC_INVALID_ARGUMENT";
    throw err;
  }
  if (queryObj === null || typeof queryObj !== 'object') {
    const err = new Error("Collection.querySync(): Expected exactly 1 argument. Argument must be an Query object");
    err.name = "InvalidArgumentError";
    err.code = "ZVEC_INVALID_ARGUMENT";
    throw err;
  }
  if ('vectors' in queryObj) {
    const err = new Error("MultiQuery functionality has not been implemented yet.");
    err.name = "NotSupportedError";
    err.code = "ZVEC_NOT_SUPPORTED";
    throw err;
  } else {
    return this._internalQuery(queryObj);
  }
};


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
  ZVecCreateAndOpen: binding.createAndOpen,
  ZVecOpen: binding.open,
  isZVecError,
};