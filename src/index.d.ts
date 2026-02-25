/**
 * Numeric enum object defining supported data types in Zvec.
 *
 * Scalar types (e.g., `INT64`, `STRING`) are used with {@link ZVecFieldSchema}.
 * Vector types (e.g., `VECTOR_FP32`) are used with {@link ZVecVectorSchema}.
 *
 * @group Types
 */
export declare const ZVecDataType: {
  // readonly UNDEFINED: 0;
  // readonly BINARY: 1;
  readonly STRING: 2;
  readonly BOOL: 3;
  readonly INT32: 4;
  readonly INT64: 5;
  readonly UINT32: 6;
  readonly UINT64: 7;
  readonly FLOAT: 8;
  readonly DOUBLE: 9;
  // readonly VECTOR_BINARY32: 20;
  // readonly VECTOR_BINARY64: 21;
  readonly VECTOR_FP16: 22;
  readonly VECTOR_FP32: 23;
  readonly VECTOR_FP64: 24;
  // readonly VECTOR_INT4: 25;
  readonly VECTOR_INT8: 26;
  // readonly VECTOR_INT16: 27;
  readonly SPARSE_VECTOR_FP16: 30;
  readonly SPARSE_VECTOR_FP32: 31;
  // readonly ARRAY_BINARY: 40;
  readonly ARRAY_STRING: 41;
  readonly ARRAY_BOOL: 42;
  readonly ARRAY_INT32: 43;
  readonly ARRAY_INT64: 44;
  readonly ARRAY_UINT32: 45;
  readonly ARRAY_UINT64: 46;
  readonly ARRAY_FLOAT: 47;
  readonly ARRAY_DOUBLE: 48;
};

export type ZVecDataType = typeof ZVecDataType[keyof typeof ZVecDataType];


/**
 * Numeric enum object defining supported index types in Zvec.
 *
 * @group Types
 */
export declare const ZVecIndexType: {
  readonly UNDEFINED: 0;
  readonly HNSW: 1;
  readonly IVF: 3;
  readonly FLAT: 4;
  readonly INVERT: 10;
};

export type ZVecIndexType = typeof ZVecIndexType[keyof typeof ZVecIndexType];


/**
 * Numeric enum object defining distance metrics for vector indexes.
 *
 * @group Types
 */
export declare const ZVecMetricType: {
  readonly UNDEFINED: 0;
  /** Euclidean distance (L2 norm). */
  readonly L2: 1;
  /** Inner product (dot product). */
  readonly IP: 2;
  /** Cosine similarity. */
  readonly COSINE: 3;
};

export type ZVecMetricType = typeof ZVecMetricType[keyof typeof ZVecMetricType];


/**
 * Numeric enum object defining quantization types for vector compression.
 *
 * @group Types
 */
export declare const ZVecQuantizeType: {
  readonly UNDEFINED: 0;
  readonly FP16: 1;
  readonly INT8: 2;
  readonly INT4: 3;
};

export type ZVecQuantizeType = typeof ZVecQuantizeType[keyof typeof ZVecQuantizeType];


/**
 * Union type of all valid vectors.
 *
 * @group Types
 */
export type ZVecVector = number[] | Float32Array | Int8Array | Record<number, number>;


/**
 * String enum object defining supported log output types in Zvec.
 *
 * @group Global Configuration
 */
export declare const ZVecLogType: {
  readonly CONSOLE: "console";
  readonly FILE: "file";
};

export type ZVecLogType = typeof ZVecLogType[keyof typeof ZVecLogType];


/**
 * Numeric enum object defining supported log levels in Zvec.
 *
 * @group Global Configuration
 */
export declare const ZVecLogLevel: {
  readonly DEBUG: 0;
  readonly INFO: 1;
  readonly WARN: 2;
  readonly ERROR: 3;
  readonly FATAL: 4;
};

export type ZVecLogLevel = typeof ZVecLogLevel[keyof typeof ZVecLogLevel];


/**
 * Configuration options for initializing Zvec globally.
 *
 * @group Global Configuration
 */
export interface ZVecInitOptions {
  /**
   * Type of log output.
   * @default ZVecLogType.CONSOLE
   */
  readonly logType?: ZVecLogType;

  /**
   * Log level for messages.
   * @default ZVecLogLevel.WARN
   */
  readonly logLevel?: ZVecLogLevel;

  /**
   * Directory for log files (used when logType is 'FILE').
   * @default './logs'
   */
  readonly logDir?: string;

  /**
   * Base name for log files (used when logType is 'FILE').
   * @default 'zvec.log'
   */
  readonly logBaseName?: string;

  /**
   * Maximum size of each log file in bytes (used when logType is 'FILE').
   * @default 2048
   */
  readonly logFileSize?: number;

  /**
   * Number of days to keep old log files (used when logType is 'FILE').
   * @default 7
   */
  readonly logOverdueDays?: number;

  /**
   * Number of threads for query execution.
   * @default undefined // Let the system decide
   */
  readonly queryThreads?: number;

  /**
   * Number of threads for background tasks.
   * @default undefined // Let the system decide
   */
  readonly optimizeThreads?: number;
}


/**
 * Initializes global Zvec configurations.
 * This function should be called only once at the start of your application.
 * @param options - Optional configuration parameters.
 *
 * @group Global Configuration
 */
export function ZVecInitialize(options?: ZVecInitOptions): void;


/**
 * Base interface for index parameters, requiring the type of index.
 *
 * @group Index Parameters
 */
export interface ZVecIndexParams {
  readonly indexType: ZVecIndexType;
}


/**
 * Configuration parameters for a flat index on a vector field.
 *
 * @group Index Parameters
 */
export interface ZVecFlatIndexParams extends ZVecIndexParams {
  /** Must be `ZVecIndexType.FLAT` (4). */
  readonly indexType: typeof ZVecIndexType.FLAT;

  /**
   * Distance metric used for similarity computation.
   * @default ZVecMetricType.IP
   */
  readonly metricType?: ZVecMetricType;

  /**
   * Optional quantization type for vector compression.
   * @default ZVecQuantizeType.UNDEFINED
   */
  readonly quantizeType?: ZVecQuantizeType;
}


/**
 * Configuration parameters for an HNSW index on a vector field.
 *
 * @group Index Parameters
 */
export interface ZVecHnswIndexParams extends ZVecIndexParams {
  /** Must be `ZVecIndexType.HNSW` (1). */
  readonly indexType: typeof ZVecIndexType.HNSW;

  /**
   * Distance metric used for similarity computation.
   * @default ZVecMetricType.IP
   */
  readonly metricType?: ZVecMetricType;

  /**
   * Number of bi-directional links created for every new element during construction.
   * @default 50
   */
  readonly m?: number;

  /**
   * Size of the dynamic candidate list for nearest neighbors during index construction.
   * @default 500
   */
  readonly efConstruction?: number;

  /**
   * Optional quantization type for vector compression.
   * @default ZVecQuantizeType.UNDEFINED
   */
  readonly quantizeType?: ZVecQuantizeType;
}


/**
 * Configuration parameters for an IVF index on a vector field.
 *
 * @group Index Parameters
 */
export interface ZVecIVFIndexParams extends ZVecIndexParams {
  /** Must be `ZVecIndexType.IVF` (3). */
  readonly indexType: typeof ZVecIndexType.IVF;

  /**
   * Distance metric used for similarity computation.
   * @default ZVecMetricType.IP
   */
  readonly metricType?: ZVecMetricType;

  /**
   * Number of clusters (inverted lists) to partition the dataset into.
   * @default 10
   */
  readonly nList?: number;

  /**
   * Number of iterations for k-means clustering during index training.
   * @default 10
   */
  readonly nIters?: number;

  /**
   * Optional quantization type for vector compression.
   * @default ZVecQuantizeType.UNDEFINED
   */
  readonly quantizeType?: ZVecQuantizeType;
}


/**
 * Configuration parameters for an inverted index on a scalar field.
 *
 * @group Index Parameters
 */
export interface ZVecInvertIndexParams extends ZVecIndexParams {
  /** Must be `ZVecIndexType.INVERT` (10). */
  readonly indexType: typeof ZVecIndexType.INVERT;

  /**
   * Enables range optimization (e.g., faster `>`, `<` queries).
   * @default true
   */
  readonly enableRangeOptimization?: boolean;

  /**
   * Enables extended wildcard matching (e.g., `foo*bar`) for string fields.
   * @default false
   */
  readonly enableExtendedWildcard?: boolean;
}


/**
 * Base interface for query-time parameters, requiring the type of index.
 *
 * @group Query Parameters
 */
export interface ZVecQueryParams {
  readonly indexType: ZVecIndexType;

  /**
   * Search radius for range queries. Used in combination with top-k to filter results.
   * @default 0.0 (disabled)
   */
  readonly radius?: number;

  /**
   * If True, forces brute-force linear search instead of using the index. Useful for debugging or small datasets.
   * @default false
   */
  readonly isLinear?: boolean;

  /**
   * Whether to use refiner for the query.
   * @default false
   */
  readonly isUsingRefiner?: boolean;
}


/**
 * Query-time parameters for searches performed against an HNSW index.
 *
 * @group Query Parameters
 */
export interface ZVecHnswQueryParams extends ZVecQueryParams {
  /** Must be `ZVecIndexType.HNSW` (1). */
  readonly indexType: typeof ZVecIndexType.HNSW;

  /**
   * Size of the dynamic candidate list during search. Larger values improve recall but slow down search.
   * @default 300
   */
  readonly ef?: number;
}


/**
 * Query-time parameters for searches performed against an IVF index.
 *
 * @group Query Parameters
 */
export interface ZVecIVFQueryParams extends ZVecQueryParams {
  /** Must be `ZVecIndexType.IVF` (3). */
  readonly indexType: typeof ZVecIndexType.IVF;

  /**
   * Number of closest clusters (inverted lists) to search. Higher values improve recall but increase latency.
   * @default 10
   */
  readonly nprobe?: number;
}


/**
 * Query object used to perform searches against a collection.
 *
 * You can use it for:
 * - Vector similarity search: provide both `fieldName` and `vector`.
 * - Scalar-only filtering: provide only `filter`.
 * - Hybrid search: provide `fieldName` + `vector` along with a scalar `filter`.
 *
 * @group Query Parameters
 */
export interface ZVecQuery {
  /**
   * The name of the vector field to search against.
   */
  fieldName?: string;

  /**
   * Number of nearest neighbors to return.
   * @default 10
   */
  topk?: number;

  /**
   * The query vector. Can be a standard JavaScript array, a typed array, or a sparse vector object map.
   * @default undefined
   */
  vector?: ZVecVector;

  /**
   * Boolean expression to pre-filter candidates.
   * @default undefined
   */
  filter?: string;

  /**
   * Whether to include vector data in results.
   * @default false
   */
  includeVector?: boolean;

  /**
   * Scalar fields to include. If undefined, all fields are returned.
   * @default undefined
   */
  outputFields?: string[];

  /**
   * Query-time parameters to fine-tune search behavior.
   * @default undefined
   */
  params?: ZVecHnswQueryParams | ZVecIVFQueryParams;
}


/**
 * Configuration options for opening or creating a collection.
 *
 * @group Options
 */
export interface ZVecCollectionOptions {
  /**
   * Whether the collection is opened in read-only mode.
   * @default false
   */
  readonly readOnly?: boolean;

  /**
   * Whether to use memory-mapped I/O for data files.
   * @default true
   */
  readonly enableMMAP?: boolean;
}


/**
 * Configuration options for adding a new column to a collection.
 *
 * @group Options
 */
export interface ZVecAddColumnOptions {
  /**
   * Number of threads to use when backfilling data for the new column. If 0, auto-detect is used.
   * @default 0
   */
  readonly concurrency?: number;
}


/**
 * Configuration options for altering an existing column of a collection.
 *
 * @group Options
 */
export interface ZVecAlterColumnOptions {
  /**
   * Number of threads to use during the alteration process. If 0, auto-detect is used.
   * @default 0
   */
  readonly concurrency?: number;
}


/**
 * Configuration options for creating a new index on a field.
 *
 * @group Options
 */
export interface ZVecCreateIndexOptions {
  /**
   * Number of threads to use during index creation. If 0, auto-detect is used.
   * @default 0
   */
  readonly concurrency?: number;
}


/**
 * Configuration options for optimizing a collection.
 *
 * @group Options
 */
export interface ZVecOptimizeOptions {
  /**
   * Number of threads to use during optimization. If 0, auto-detect is used.
   * @default 0
   */
  readonly concurrency?: number;
}


/**
 * Status of an operation.
 *
 * @group Types
 */
export interface ZVecStatus {
  /**
   * Indicates whether the operation was successful.
   */
  readonly ok: boolean;

  /**
   * A string code identifying the type of status or error.
   */
  readonly code: string;

  /**
   * A human-readable message providing details about error if any.
   */
  readonly message: string;
}


/**
 * An error thrown by Zvec.
 *
 * Zvec errors are standard JavaScript `Error` objects with additional `name`
 * and `code` property that identifies the failure reason.
 *
 * @group Types
 */
export interface ZVecError extends Error {
  /** The error name. */
  name: string;

  /** The error code. */
  code:
  | 'ZVEC_NOT_FOUND'
  | 'ZVEC_ALREADY_EXISTS'
  | 'ZVEC_INVALID_ARGUMENT'
  | 'ZVEC_PERMISSION_DENIED'
  | 'ZVEC_FAILED_PRECONDITION'
  | 'ZVEC_RESOURCE_EXHAUSTED'
  | 'ZVEC_UNAVAILABLE'
  | 'ZVEC_INTERNAL_ERROR'
  | 'ZVEC_NOT_SUPPORTED'
  | 'ZVEC_UNKNOWN'
  | 'ZVEC_INVALID_STATUS_CODE';
}


/**
 * Type guard for narrowing an `unknown` caught value to a {@link ZVecError}.
 *
 * Use this in `catch (e: unknown)` blocks to safely access `e.code`.
 *
 * @group Types
 */
export declare function isZVecError(error: unknown): error is ZVecError;


/**
 * Scalar field schema.
 *
 * @group Schema
 */
export interface ZVecFieldSchema {
  /**
   * Name of the scalar field.
   */
  readonly name: string;

  /**
   * Data type of the scalar field (e.g., INT64, STRING).
   */
  readonly dataType: ZVecDataType;

  /**
   * Whether the field can contain null values.
   * @default false
   */
  readonly nullable?: boolean;

  /**
   * Optional inverted index parameters for this field.
   * @default undefined
   */
  readonly indexParams?: ZVecInvertIndexParams;
}


/**
 * Vector field schema.
 *
 * @group Schema
 */
export interface ZVecVectorSchema {
  /**
   * Name of the vector field.
   */
  readonly name: string;

  /**
   * Data type of the vector field.
   */
  readonly dataType: ZVecDataType;

  /**
   *  Dimension of the vector. Must be > 0 for dense vectors.
   * @default 0
   */
  readonly dimension?: number;

  /**
   * Vector index parameters for this vector field.
   * @default undefined
   */
  readonly indexParams?: ZVecFlatIndexParams | ZVecHnswIndexParams | ZVecIVFIndexParams;
}


/**
 * Collection Schema.
 *
 * @group Schema
 */
export declare class ZVecCollectionSchema {
  /**
   * Creates a new collection schema.
   * @param params - An object containing the name, vectors, and optional fields.
   * @param params.name - The name of the collection.
   * @param params.vectors - An array of vector schemas, or a single vector schema object.
   * @param params.fields - An optional array of scalar field schemas, or a single scalar field schema object.
   */
  constructor(params: {
    name: string;
    vectors: ZVecVectorSchema[] | ZVecVectorSchema;
    fields?: ZVecFieldSchema[] | ZVecFieldSchema;
  });

  /**
   * Get the name of the collection.
   */
  readonly name: string;

  /**
   * Retrieves a scalar field schema by its name.
   * @param fieldName - The name of the field to retrieve.
   * @returns The ZVecFieldSchema object.
   * @throws Error if the field with the given name is not found.
   */
  field(fieldName: string): ZVecFieldSchema;

  /**
   * Retrieves a vector field schema by its name.
   * @param vectorName - The name of the vector field to retrieve.
   * @returns The ZVecVectorSchema object.
   * @throws Error if the vector field with the given name is not found.
   */
  vector(vectorName: string): ZVecVectorSchema;

  /**
   * Gets the array of scalar field schemas.
   * @returns The array of ZVecFieldSchema objects.
   */
  fields(): ZVecFieldSchema[];

  /**
   * Gets the array of vector field schemas.
   * @returns The array of ZVecVectorSchema objects.
   */
  vectors(): ZVecVectorSchema[];

  /**
   * @returns A formatted string representation of the collection schema.
   */
  toString(): string;
}


/**
 * Represents a document in Zvec, containing scalar fields, vector fields, and a score.
 *
 * @group Doc
 */
export interface ZVecDoc {
  /**
   * The unique identifier for the document.
   */
  readonly id: string;

  /**
   * An object mapping vector field names to their corresponding vector values.
   * Values can be standard JavaScript arrays or typed arrays (e.g., Float32Array).
   */
  readonly vectors: Record<string, ZVecVector>;

  /**
   * An object mapping scalar field names to their corresponding values.
   * Values can be scalar primitives (number, string, boolean), arrays of primitives, or typed arrays.
   */
  readonly fields: Record<string, any>;

  /**
   * A relevance score associated with the document, often returned by search operations.
   */
  readonly score: number;
}


/**
 * Represents an input document in Zvec, optionally containing scalar fields and vector fields.
 *
 * @group Doc
 */
export interface ZVecDocInput {
  /**
   * The unique identifier for the document.
   */
  readonly id: string;

  /**
   * An object mapping vector field names to their corresponding vector values.
   * Values can be standard JavaScript arrays or typed arrays (e.g., Float32Array).
   */
  readonly vectors?: Record<string, ZVecVector>;

  /**
   * An object mapping scalar field names to their corresponding values.
   * Values can be scalar primitives (number, string, boolean), arrays of primitives, or typed arrays.
   */
  readonly fields?: Record<string, any>;
}


/**
 * Represents a collection in Zvec.
 *
 * @group Collection
 */
export declare class ZVecCollection {
  /**
   * Gets the file system path associated with this collection.
   */
  readonly path: string;

  /**
   * Gets the schema defining the structure of this collection.
   */
  readonly schema: ZVecCollectionSchema;

  /**
   * Gets the options used when this collection was opened/created.
   */
  readonly options: ZVecCollectionOptions;

  /**
   * Gets statistics about this collection.
   */
  readonly stats: { docCount: number; indexCompleteness: Record<string, number>; };

  /**
   * Inserts a single document into the collection.
   * @param docs - A single document to insert.
   * @returns The status of the operation.
   */
  insertSync(docs: ZVecDocInput): ZVecStatus;

  /**
   * Inserts a batch of documents into the collection.
   * @param docs - An array of documents to insert.
   * @returns An array of statuses of the operation.
   */
  insertSync(docs: ZVecDocInput[]): ZVecStatus[];

  /**
   * Inserts a single document into the collection, or updates it if it already exists based on its ID.
   * @param docs - A single document to upsert.
   * @returns The status of the operation.
   */
  upsertSync(docs: ZVecDocInput): ZVecStatus;

  /**
   * Inserts a batch of documents into the collection, or updates them if they already exist based on their ID.
   * @param docs - An array of documents to upsert.
   * @returns An array of statuses of the operation.
   */
  upsertSync(docs: ZVecDocInput[]): ZVecStatus[];

  /**
   * Updates a single existing document by ID.
   * Only specified fields and vectors are updated; others remain unchanged.
   * @param docs - A single document containing the updated data.
   * @returns The status of the operation.
   */
  updateSync(docs: ZVecDocInput): ZVecStatus;

  /**
   * Updates multiple existing documents by ID.
   * Only specified fields and vectors are updated; others remain unchanged.
   * @param docs - An array of documents containing the updated data.
   * @returns An array of statuses of the operation.
   */
  updateSync(docs: ZVecDocInput[]): ZVecStatus[];

  /**
   * Deletes a single document by its ID.
   * @param ids - A single document ID to delete.
   * @returns The status of the operation.
   */
  deleteSync(ids: string): ZVecStatus;

  /**
   * Deletes multiple documents by their IDs.
   * @param ids - An array of document IDs to delete.
   * @returns An array of statuses of the operation.
   */
  deleteSync(ids: string[]): ZVecStatus[];

  /**
   * Deletes documents based on a filter expression.
   * @param filter - A string representing the filter expression.
   * @returns The status of the operation.
   */
  deleteByFilterSync(filter: string): ZVecStatus;

  /**
   * Performs a vector similarity search query.
   * @param params - The query parameters.
   * @returns An array of documents ranked by similarity to the query vector.
   */
  querySync(params: ZVecQuery): ZVecDoc[];

  /**
   * Fetches documents by their IDs.
   * @param ids - A single document ID or an array of document IDs to fetch.
   * @returns An object mapping the requested IDs to their corresponding documents. If an ID is not found, it will not be present in the returned object.
   */
  fetchSync(ids: string | string[]): Record<string, ZVecDoc>;

  /**
   * Optimizes the collection's internal structures for better performance.
   * @param options - Optional parameters for the operation.
   * @throws {Error} If the operation fails.
   */
  optimizeSync(options?: ZVecOptimizeOptions): void;

  /**
   * Closes the collection and releases its resources.
   * After calling this method, the collection object should not be used.
   * @throws {Error} If the operation fails.
   */
  closeSync(): void;

  /**
   * Permanently deletes the collection from disk.
   * After calling this method, the collection object should not be used.
   * @throws {Error} If the operation fails.
   */
  destroySync(): void;

  /**
   * Adds a new column to the collection.
   * @param params - An object containing the parameters for the operation.
   * @param params.fieldSchema - The schema definition for the new column.
   * @param params.expression - Optional expression to compute values for existing documents.
   * @param params.options - Optional parameters for the operation.
   * @throws {Error} If the operation fails.
   */
  addColumnSync(params: {
    fieldSchema: ZVecFieldSchema;
    expression?: string;
    options?: ZVecAddColumnOptions;
  }): void;

  /**
   * Permanently deletes a column from the collection.
   * After dropping, the data and schema for this column are removed.
   * @param fieldName - The name of the column to drop.
   * @throws {Error} If the operation fails.
   */
  dropColumnSync(fieldName: string): void;

  /**
   * Alters an existing column in the collection.
   * You can either rename the column or modify its schema.
   * @param params - An object containing the parameters for the operation.
   * @param params.columnName - The current name of the column to alter.
   * @param params.newColumnName - The new name for the column (mutually exclusive with fieldSchema).
   * @param params.fieldSchema - The new schema definition for the column (mutually exclusive with newColumnName).
   * @param params.options - Optional parameters for the operation.
   * @throws {Error} If the operation fails.
   */
  alterColumnSync(params: {
    columnName: string;
    newColumnName?: string;
    fieldSchema?: ZVecFieldSchema;
    options?: ZVecAlterColumnOptions;
  }): void;

  /**
   * Creates an index on a specific column in the collection.
   * @param params - An object containing the parameters for the operation.
   * @param params.fieldName - The name of the column to create the index on.
   * @param params.indexParams - The configuration parameters defining the index type and properties.
   * @param params.indexOptions - Optional parameters for the index creation operation.
   * @throws {Error} If the operation fails.
   */
  createIndexSync(params: {
    fieldName: string;
    indexParams: ZVecFlatIndexParams | ZVecHnswIndexParams | ZVecIVFIndexParams | ZVecInvertIndexParams;
    indexOptions?: ZVecCreateIndexOptions;
  }): void;

  /**
   * Drops (removes) an existing index from the collection.
   * @param fieldName - The name of the column whose index should be dropped.
   * @throws {Error} If the operation fails.
   */
  dropIndexSync(fieldName: string): void;
}


/**
 * Creates and opens a Zvec collection.
 * @param path - The file system path where the collection will be created.
 * @param schema - The schema defining the collection's structure.
 * @param options - Optional parameters for opening the collection.
 * @returns An instance of ZVecCollection.
 *
 * @group Collection
 */
export function ZVecCreateAndOpen(
  path: string,
  schema: ZVecCollectionSchema,
  options?: ZVecCollectionOptions
): ZVecCollection;


/**
  * Opens an existing Zvec collection.
  * @param path - The file system path where the collection is stored.
  * @param options - Optional parameters for opening the collection.
  * @returns An instance of ZVecCollection.
  *
  * @group Collection
  */
export function ZVecOpen(
  path: string,
  options?: ZVecCollectionOptions
): ZVecCollection;