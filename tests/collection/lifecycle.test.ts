import * as fs from 'fs';
import {
  isZVecError,
  ZVecCollection,
  ZVecCollectionSchema,
  ZVecCreateAndOpen,
  ZVecDataType,
  ZVecHnswIndexParams,
  ZVecIndexType,
  ZVecInitialize,
  ZVecLogLevel,
  ZVecLogType,
  ZVecMetricType,
  ZVecOpen,
  ZVecQuantizeType,
  ZVecStatus
} from '../../src/index';


ZVecInitialize({
  logType: ZVecLogType.CONSOLE,
  logLevel: ZVecLogLevel.WARN,
});


describe('Collection Lifecycle', () => {
  const collectionPath = './test_lifecycle_collection';
  const collectionName = 'lifecycle_test';

  const schema = new ZVecCollectionSchema({
    name: collectionName,
    vectors: {
      name: 'embedding',
      dataType: ZVecDataType.VECTOR_FP32,
      dimension: 8,
      indexParams: {
        indexType: ZVecIndexType.HNSW,
        metricType: ZVecMetricType.COSINE,
        m: 16,
        quantizeType: ZVecQuantizeType.FP16
      }
    },
    fields: {
      name: 'label',
      dataType: ZVecDataType.STRING,
      nullable: true,
      indexParams: { indexType: ZVecIndexType.INVERT }
    }
  });

  beforeAll(() => {
    if (fs.existsSync(collectionPath)) {
      fs.rmSync(collectionPath, { recursive: true, force: true });
    }
  });

  afterAll(() => {
    if (fs.existsSync(collectionPath)) {
      fs.rmSync(collectionPath, { recursive: true, force: true });
    }
  });


  describe('create', () => {
    it('should create a new collection on disk', () => {
      const collection = ZVecCreateAndOpen(collectionPath, schema);
      expect(collection).toBeDefined();
      expect(collection.path).toBe(collectionPath);
      expect(fs.existsSync(collectionPath)).toBe(true);
      collection.closeSync();
    });

    it('should reflect the schema correctly after creation', () => {
      const collection = ZVecOpen(collectionPath);

      expect(collection.schema.name).toBe(collectionName);

      const vectors = collection.schema.vectors();
      expect(vectors.length).toBe(1);
      expect(vectors[0].name).toBe('embedding');
      expect(vectors[0].dataType).toBe(ZVecDataType.VECTOR_FP32);
      expect(vectors[0].dimension).toBe(8);
      expect(vectors[0].indexParams!.indexType).toBe(ZVecIndexType.HNSW);
      expect(vectors[0].indexParams!.metricType).toBe(ZVecMetricType.COSINE);
      expect((vectors[0].indexParams as ZVecHnswIndexParams).m).toBe(16);
      expect((vectors[0].indexParams as ZVecHnswIndexParams).quantizeType).toBe(ZVecQuantizeType.FP16);

      const fields = collection.schema.fields();
      expect(fields.length).toBe(1);
      expect(fields[0].name).toBe('label');
      expect(fields[0].dataType).toBe(ZVecDataType.STRING);
      expect(fields[0].nullable).toBe(true);
      expect(fields[0].indexParams!.indexType).toBe(ZVecIndexType.INVERT);

      expect(collection.stats.docCount).toBe(0);

      collection.closeSync();
    });

    it('should fail to create over an existing collection path', () => {
      try {
        ZVecCreateAndOpen(collectionPath, schema);
        fail('Expected an error to be thrown');
      } catch (error) {
        expect(isZVecError(error)).toBe(true);
      }
    });
  });


  describe('open', () => {
    it('should open an existing collection', () => {
      const collection = ZVecOpen(collectionPath);
      expect(collection).toBeDefined();
      expect(collection.path).toBe(collectionPath);
      expect(collection.schema.name).toBe(collectionName);
      expect(collection.stats.docCount).toBe(0);
      collection.closeSync();
    });

    it('should fail to open a non-existing path', () => {
      try {
        ZVecOpen('./non_existing_path');
        fail('Expected an error to be thrown');
      } catch (error) {
        expect(isZVecError(error)).toBe(true);
      }
    });
  });


  describe('basic operations after open', () => {
    let collection: ZVecCollection;

    beforeAll(() => {
      collection = ZVecOpen(collectionPath);
    });

    afterAll(() => {
      collection.closeSync();
    });

    it('should insert documents', () => {
      const result = collection.insertSync([
        {
          id: 'a1',
          vectors: { embedding: [1, 0, 0, 0, 0, 0, 0, 0] },
          fields: { label: 'first' }
        },
        {
          id: 'a2',
          vectors: { embedding: [0, 1, 0, 0, 0, 0, 0, 0] },
          fields: { label: 'second' }
        },
        {
          id: 'a3',
          vectors: { embedding: [0, 0, 1, 0, 0, 0, 0, 0] },
          fields: { label: 'third' }
        }
      ]);
      expect(result.every((r: ZVecStatus) => r.ok)).toBe(true);
      expect(collection.stats.docCount).toBe(3);
    });

    it('should fetch inserted documents', () => {
      const fetched = collection.fetchSync(['a1', 'a2']);
      expect(fetched['a1'].fields.label).toBe('first');
      expect(fetched['a2'].fields.label).toBe('second');
    });

    it('should query by vector similarity', () => {
      const results = collection.querySync({
        fieldName: 'embedding',
        vector: [1, 0, 0, 0, 0, 0, 0, 0],
        topk: 1,
      });
      expect(results[0].id).toBe('a1');
    });

    it('should delete a document', () => {
      const result = collection.deleteSync('a3');
      expect(result.ok).toBe(true);
      expect(collection.stats.docCount).toBe(2);

      const fetched = collection.fetchSync('a3');
      expect('a3' in fetched).toBe(false);
    });
  });


  describe('close', () => {
    it('should close without error', () => {
      const collection = ZVecOpen(collectionPath);
      expect(() => collection.closeSync()).not.toThrow();
    });

    it('should throw when using collection after close', () => {
      const collection = ZVecOpen(collectionPath);
      collection.closeSync();

      const ops: [string, () => void][] = [
        ['insertSync', () => collection.insertSync({
          id: 'should_fail',
          vectors: { embedding: [0, 0, 0, 0, 0, 0, 0, 0] },
          fields: { label: 'fail' }
        })],
        ['fetchSync', () => collection.fetchSync('a1')],
        ['querySync', () => collection.querySync({
          fieldName: 'embedding',
          vector: [1, 0, 0, 0, 0, 0, 0, 0],
        })],
      ];

      for (const [name, op] of ops) {
        try {
          op();
          fail(`${name} should have thrown`);
        } catch (error) {
          expect(isZVecError(error)).toBe(true);
        }
      }
    });
  });


  describe('destroy', () => {
    it('should remove the collection from disk', () => {
      const collection = ZVecOpen(collectionPath);
      collection.destroySync();
      expect(fs.existsSync(collectionPath)).toBe(false);
    });
  });
});
