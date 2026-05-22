import * as fs from 'fs';
import {
  isZVecError,
  ZVecCollection,
  ZVecCollectionSchema,
  ZVecCreateAndOpen,
  ZVecDataType,
  ZVecIndexType,
  ZVecInitialize,
  ZVecLogLevel,
  ZVecLogType,
  ZVecMetricType,
} from '../../src/index';


ZVecInitialize({
  logType: ZVecLogType.CONSOLE,
  logLevel: ZVecLogLevel.WARN,
});


describe('Collection Columns', () => {
  const collectionPath = './test_columns_collection';
  const dimension = 17;
  let collection: ZVecCollection;

  const makeEmbedding = (hotIndex: number) =>
    Array.from({ length: dimension }, (_, j) => (j === hotIndex % dimension ? 1 : 0.1));

  beforeAll(() => {
    if (fs.existsSync(collectionPath)) {
      fs.rmSync(collectionPath, { recursive: true, force: true });
    }

    const schema = new ZVecCollectionSchema({
      name: 'columns_test',
      vectors: {
        name: 'embedding',
        dataType: ZVecDataType.VECTOR_FP32,
        dimension,
        indexParams: {
          indexType: ZVecIndexType.HNSW,
          metricType: ZVecMetricType.COSINE,
        }
      },
      fields: {
        name: 'tag',
        dataType: ZVecDataType.ARRAY_STRING,
        nullable: true,
        indexParams: { indexType: ZVecIndexType.INVERT }
      }
    });

    collection = ZVecCreateAndOpen(collectionPath, schema);

    const docs = Array.from({ length: 20 }, (_, i) => ({
      id: `doc${i + 1}`,
      vectors: { embedding: makeEmbedding(i) },
      fields: { tag: [`category_${i % 5}`] }
    }));
    collection.insertSync(docs);
    collection.optimizeSync();
  });

  afterAll(() => {
    if (collection) {
      collection.destroySync();
    }
    if (fs.existsSync(collectionPath)) {
      fs.rmSync(collectionPath, { recursive: true, force: true });
    }
  });


  describe('addColumn', () => {
    it('should add a new scalar column', () => {
      collection.addColumnSync({
        fieldSchema: {
          name: 'price',
          dataType: ZVecDataType.INT64,
          nullable: true,
          indexParams: { indexType: ZVecIndexType.INVERT, enableRangeOptimization: true }
        }
      });

      const fields = collection.schema.fields();
      expect(fields.length).toBe(2);

      const priceField = collection.schema.field('price');
      expect(priceField.name).toBe('price');
      expect(priceField.dataType).toBe(ZVecDataType.INT64);
      expect(priceField.nullable).toBe(true);
      expect(priceField.indexParams!.indexType).toBe(ZVecIndexType.INVERT);

      const tagField = collection.schema.field('tag');
      expect(tagField.name).toBe('tag');
      expect(tagField.dataType).toBe(ZVecDataType.ARRAY_STRING);
      expect(tagField.nullable).toBe(true);
      expect(tagField.indexParams!.indexType).toBe(ZVecIndexType.INVERT);

      const vectors = collection.schema.vectors();
      expect(vectors.length).toBe(1);
      expect(vectors[0].name).toBe('embedding');
      expect(vectors[0].dimension).toBe(17);
    });

    it('should support insert, query, and delete with the new column', () => {
      const insertResults = collection.insertSync([
        {
          id: 'doc21',
          vectors: { embedding: makeEmbedding(3) },
          fields: { tag: ['tech'], price: 199 }
        },
        {
          id: 'doc22',
          vectors: { embedding: makeEmbedding(5) },
          fields: { tag: ['art'], price: 50 }
        },
        {
          id: 'doc23',
          vectors: { embedding: makeEmbedding(7) },
          fields: { tag: ['science'], price: 320 }
        },
      ]);
      expect(insertResults.every(r => r.ok)).toBe(true);

      const fetched = collection.fetchSync(['doc21', 'doc22', 'doc23']);
      expect(fetched['doc21'].fields['price']).toBe(199);
      expect(fetched['doc22'].fields['price']).toBe(50);
      expect(fetched['doc23'].fields['price']).toBe(320);

      const queryResults = collection.querySync({ filter: 'price > 100' });
      const ids = queryResults.map(r => r.id);
      expect(ids).toContain('doc21');
      expect(ids).toContain('doc23');
      expect(ids).not.toContain('doc22');

      expect(collection.deleteSync('doc5').ok).toBe(true);
      expect(collection.deleteSync('doc22').ok).toBe(true);

      const afterDelete = collection.fetchSync(['doc5', 'doc22']);
      expect('doc5' in afterDelete).toBe(false);
      expect('doc22' in afterDelete).toBe(false);

      expect(collection.fetchSync('doc1')['doc1']).toBeDefined();
      expect(collection.fetchSync('doc21')['doc21']).toBeDefined();
    });

    it('should throw on duplicate column name', () => {
      try {
        collection.addColumnSync({
          fieldSchema: { name: 'price', dataType: ZVecDataType.UINT32 }
        });
        fail('Expected an error to be thrown');
      } catch (error) {
        expect(isZVecError(error)).toBe(true);
        if (isZVecError(error)) {
          expect(error.code).toBe('ZVEC_INVALID_ARGUMENT');
        }
      }
    });
  });


  describe('alterColumn', () => {
    it('should rename an existing column', () => {
      collection.alterColumnSync({
        columnName: 'price',
        newColumnName: 'cost'
      });

      expect(collection.schema.fields().length).toBe(2);
      const costField = collection.schema.field('cost');
      expect(costField.name).toBe('cost');
      expect(costField.dataType).toBe(ZVecDataType.INT64);
    });

    it('should not find the old column name after rename', () => {
      try {
        collection.schema.field('price');
        fail('Expected an error to be thrown');
      } catch (error) {
        expect(isZVecError(error)).toBe(true);
        if (isZVecError(error)) {
          expect(error.code).toBe('ZVEC_NOT_FOUND');
        }
      }
    });

    it('should preserve data under the new column name', () => {
      const fetched = collection.fetchSync(['doc21', 'doc23']);
      expect(fetched['doc21'].fields['cost']).toBe(199);
      expect(fetched['doc23'].fields['cost']).toBe(320);
    });
  });


  describe('dropColumn', () => {
    it('should remove an existing column', () => {
      collection.dropColumnSync('cost');
      expect(collection.schema.fields().length).toBe(1);
    });

    it('should throw when dropping a non-existing column', () => {
      try {
        collection.dropColumnSync('cost');
        fail('Expected an error to be thrown');
      } catch (error) {
        expect(isZVecError(error)).toBe(true);
        if (isZVecError(error)) {
          expect(error.code).toBe('ZVEC_INVALID_ARGUMENT');
        }
      }
    });

    it('should still allow fetching and querying after column drop', () => {
      const fetched = collection.fetchSync(['doc1', 'doc10', 'doc21']);
      expect(fetched['doc1'].fields['tag']).toEqual(['category_0']);
      expect(fetched['doc10'].fields['tag']).toEqual(['category_4']);
      expect(fetched['doc21'].fields['tag']).toEqual(['tech']);
      expect('cost' in fetched['doc21'].fields).toBe(false);

      const queryResult = collection.querySync({
        fieldName: 'embedding',
        vector: makeEmbedding(0),
        topk: 1,
      });
      expect(queryResult[0].id).toBe('doc1');
    });
  });
});
