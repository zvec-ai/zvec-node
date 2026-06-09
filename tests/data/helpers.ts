import {
  ZVecCollection,
  ZVecCollectionSchema,
  ZVecDataType,
  ZVecDoc,
  ZVecDocInput,
  ZVecIndexType,
  ZVecMetricType,
  ZVecQuantizeType
} from '../../src/index';


// ─── Schema ──────────────────────────────────────────────────────────────────

const DIMENSION = 128;

const TEST_SCHEMA = {
  vectors: [
    {
      name: 'dense',
      dataType: ZVecDataType.VECTOR_FP32,
      dimension: DIMENSION,
      indexParams: {
        indexType: ZVecIndexType.IVF,
        metricType: ZVecMetricType.COSINE,
        efConstruction: 200,
        quantizeType: ZVecQuantizeType.FP16
      }
    },
    {
      name: 'sparse',
      dataType: ZVecDataType.SPARSE_VECTOR_FP32,
      indexParams: { indexType: ZVecIndexType.HNSW }
    }
  ],
  fields: [
    {
      name: 'title',
      dataType: ZVecDataType.STRING,
      indexParams: { indexType: ZVecIndexType.INVERT }
    },
    {
      name: 'price',
      dataType: ZVecDataType.FLOAT,
      nullable: true,
      indexParams: { indexType: ZVecIndexType.INVERT, enableRangeOptimization: true }
    },
    {
      name: 'content',
      dataType: ZVecDataType.STRING,
      indexParams: { indexType: ZVecIndexType.FTS }
    }
  ]
};

export function createTestSchema(name: string): ZVecCollectionSchema {
  return new ZVecCollectionSchema({ name, ...TEST_SCHEMA });
}


// ─── Versioned doc generation ────────────────────────────────────────────────

function title(k: number, version: number): string { return `Product_${k}_v${version}`; }
function price(k: number, version: number): number { return (k + 0.99) * version; }
function content(k: number, version: number): string {
  return `product ${k} version ${version} search content`;
}
function dense(k: number, version: number): number[] {
  const s = k * 1000 + version;
  return Array.from({ length: DIMENSION }, (_, i) => Math.sin(s * (i + 1)));
}
function sparse(k: number, version: number): Record<number, number> {
  const s = k * 1000 + version;
  return Object.fromEntries([[0, 1.0], ...Array.from({ length: 4 }, (_, j) => [s * 10 + j + 1, j === 0 ? 5.0 : 0.5])]);
}

export function makeDoc(k: number, fieldVersion: number, vectorVersion: number): ZVecDocInput {
  return {
    id: `doc_${k}`,
    vectors: { dense: dense(k, vectorVersion), sparse: sparse(k, vectorVersion) },
    fields: {
      title: title(k, fieldVersion),
      price: price(k, fieldVersion),
      content: content(k, fieldVersion)
    }
  };
}

export function makeUpdate(k: number, fieldVersion: number): ZVecDocInput {
  return {
    id: `doc_${k}`,
    fields: {
      title: title(k, fieldVersion),
      price: price(k, fieldVersion),
      content: content(k, fieldVersion)
    }
  };
}


// ─── Batch operations ────────────────────────────────────────────────────────

const BATCH_SIZE = 500;
type Operation = 'insert' | 'upsert' | 'update';

export function batch(
  collection: ZVecCollection,
  operation: Operation,
  start: number,
  end: number,
  fieldVersion: number,
  vectorVersion: number
): void {
  if (start < 1 || end < start) {
    throw new Error(`Invalid range: [${start}, ${end}]`);
  }

  const gen = operation === 'update'
    ? (k: number) => makeUpdate(k, fieldVersion)
    : (k: number) => makeDoc(k, fieldVersion, vectorVersion);
  const method = collection[`${operation}Sync`].bind(collection) as (docs: ZVecDocInput[]) => any;

  // Single doc first
  const singleResult = collection[`${operation}Sync`](gen(start));
  expect(singleResult.ok).toBe(true);

  // Remaining in batches
  for (let k = start + 1; k <= end; k += BATCH_SIZE) {
    const to = Math.min(k + BATCH_SIZE - 1, end);
    const docs = Array.from({ length: to - k + 1 }, (_, i) => gen(k + i));
    const results = method(docs);
    const statuses = Array.isArray(results) ? results : [results];
    for (const status of statuses) {
      expect(status.ok).toBe(true);
    }
  }
}

export function verifyDocs(
  collection: ZVecCollection,
  start: number,
  end: number,
  fieldVersion: number,
  vectorVersion: number
): void {
  for (let from = start; from <= end; from += BATCH_SIZE) {
    const to = Math.min(from + BATCH_SIZE - 1, end);
    const ids = Array.from({ length: to - from + 1 }, (_, i) => `doc_${from + i}`);
    const fetched = collection.fetchSync(ids);
    for (let k = from; k <= to; k++) {
      expectDoc(fetched[`doc_${k}`], k, fieldVersion, vectorVersion);
    }
  }
}


// ─── Verification helpers ────────────────────────────────────────────────────

export function expectDoc(
  doc: ZVecDoc,
  k: number,
  fieldVersion: number,
  vectorVersion: number
): void {
  expect(doc.id).toBe(`doc_${k}`);
  expect(doc.fields.title).toBe(title(k, fieldVersion));
  expect(doc.fields.price).toBeCloseTo(price(k, fieldVersion), 2);
  const expectedDense = dense(k, vectorVersion);
  for (let i = 0; i < expectedDense.length; i++) {
    expect(doc.vectors.dense[i]).toBeCloseTo(expectedDense[i], 4);
  }
  const expectedSparse = sparse(k, vectorVersion);
  for (const [dim, val] of Object.entries(expectedSparse)) {
    expect(doc.vectors.sparse[Number(dim)]).toBeCloseTo(val, 4);
  }
}
