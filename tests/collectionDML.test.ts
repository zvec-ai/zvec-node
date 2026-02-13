import * as fs from 'fs';
import {
  ZVecCollection,
  ZVecCollectionSchema,
  ZVecCreateAndOpen,
  ZVecDataType,
  ZVecIndexType,
  ZVecInitialize,
  ZVecLogLevel,
  ZVecLogType,
  ZVecMetricType,
  ZVecOpen,
  ZVecQuantizeType
} from '../src/index';


ZVecInitialize({
  logType: ZVecLogType.CONSOLE,
  logLevel: ZVecLogLevel.WARN,
});


describe('Collection Data Modification Operations', () => {
  const testCollectionName = 'test_dml_collection';
  const testCollectionPath = './test_dml_collection';
  const dimension = 512;

  const makeDense = (k: number) =>
    Array.from({ length: dimension }, (_, idx) => (idx === (k - 1) % dimension ? 1 : 0.1));

  const makeSparse = (k: number) =>
    Object.fromEntries(
      Array.from({ length: dimension }, (_, j) => {
        const key = j + 1;
        return [key, key === ((k - 1) % dimension) + 1 ? 5 : 0.1];
      })
    );

  const makeDoc = (k: number) => ({
    id: `doc${k}`,
    vectors: {
      dense: makeDense(k),
      sparse: makeSparse(k),
    },
    fields: {
      title: `Product_${k}`,
      price: k + 0.99,
    }
  });


  beforeAll(() => {
    if (fs.existsSync(testCollectionPath)) {
      fs.rmSync(testCollectionPath, { recursive: true, force: true });
    }
  });

  afterAll(() => {
    if (fs.existsSync(testCollectionPath)) {
      fs.rmSync(testCollectionPath, { recursive: true, force: true });
    }
  });


  it('should handle insert operations correctly', () => {
    const schema = new ZVecCollectionSchema({
      name: testCollectionName,
      vectors: [
        {
          name: 'dense',
          dataType: ZVecDataType.VECTOR_FP32,
          dimension: dimension,
          indexParams: {
            indexType: ZVecIndexType.HNSW,
            metricType: ZVecMetricType.COSINE,
            m: 17,
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
          indexParams: { indexType: ZVecIndexType.INVERT, enableRangeOptimization: false }
        },
        {
          name: 'price',
          dataType: ZVecDataType.FLOAT,
          nullable: true,
          indexParams: { indexType: ZVecIndexType.INVERT, enableRangeOptimization: true }
        }
      ]
    });

    const collection: ZVecCollection = ZVecCreateAndOpen(testCollectionPath, schema);
    expect(collection).toBeDefined();

    const singleInsertResult = collection.insertSync(makeDoc(1));
    expect(singleInsertResult.ok).toBe(true);
    expect(singleInsertResult.code).toBe('ZVEC_OK');
    expect(collection.stats.docCount).toBe(1);

    const multiInsertResult = collection.insertSync([makeDoc(2), makeDoc(3)]);
    expect(multiInsertResult.length).toBe(2);
    expect(multiInsertResult[0].ok).toBe(true);
    expect(multiInsertResult[0].code).toBe('ZVEC_OK');
    expect(multiInsertResult[1].ok).toBe(true);
    expect(multiInsertResult[1].code).toBe('ZVEC_OK');
    expect(collection.stats.docCount).toBe(3);

    const fetchResult = collection.fetchSync(['doc1', 'doc2', 'doc3']);
    for (let k = 1; k <= 3; k++) {
      const doc = fetchResult[`doc${k}`];
      expect(doc).toBeDefined();
      expect(doc.fields.title).toBe(`Product_${k}`);
      expect(doc.fields.price).toBeCloseTo(k + 0.99, 2);
      expect(doc.vectors.dense[(k - 1) % dimension]).toBeCloseTo(1, 6);
      expect(doc.vectors.dense[(k % dimension)]).toBeCloseTo(0.1, 6); // some other dimension
      expect(doc.vectors.sparse[((k - 1) % dimension) + 1]).toBeCloseTo(5, 6);
    }

    const queryResult = collection.querySync({
      fieldName: 'dense',
      vector: makeDense(3),
      outputFields: ['title'],
    });
    expect(queryResult).toBeDefined();
    expect(queryResult[0].id).toBe('doc3');
    expect('title' in queryResult[0].fields).toBe(true);
    expect('price' in queryResult[0].fields).toBe(false);
    expect(queryResult[0].fields['title']).toBe('Product_3');

    collection.closeSync();
  });


  it('should handle lots of insert operations correctly', () => {
    const collection: ZVecCollection = ZVecOpen(testCollectionPath);
    expect(collection).toBeDefined();

    for (let k = 4; k <= 500; k++) {
      const result = collection.insertSync(makeDoc(k));
      expect(result.ok).toBe(true);
      expect(result.code).toBe('ZVEC_OK');
    }
    collection.optimizeSync();
    expect(collection.stats.docCount).toBe(500);
    expect(collection.stats.indexCompleteness['dense']).toBeCloseTo(1);
    expect(collection.stats.indexCompleteness['sparse']).toBeCloseTo(1);

    const denseQuery = collection.querySync({
      fieldName: 'dense',
      vector: makeDense(300),
      topk: 25,
    });
    expect(denseQuery[0].id).toBe('doc300');
    expect('title' in denseQuery[0].fields).toBe(true);
    expect('price' in denseQuery[0].fields).toBe(true);
    expect(denseQuery[0].fields['title']).toBe('Product_300');
    expect(denseQuery[0].fields['price']).toBeCloseTo(300.99);

    const sparseQuery = collection.querySync({
      fieldName: 'sparse',
      vector: makeSparse(200),
      topk: 25,
    });
    expect(sparseQuery[0].id).toBe('doc200');
    expect('title' in sparseQuery[0].fields).toBe(true);
    expect('price' in sparseQuery[0].fields).toBe(true);
    expect(sparseQuery[0].fields['title']).toBe('Product_200');
    expect(sparseQuery[0].fields['price']).toBeCloseTo(200.99);

    const filteredQuery = collection.querySync({
      filter: 'price > 100 and price < 105.5'
    });
    expect(filteredQuery.length).toBe(5);

    const filteredVectorQuery = collection.querySync({
      fieldName: 'sparse',
      vector: makeSparse(200),
      filter: 'price > 190 and price < 210'
    });
    expect(filteredVectorQuery[0].id).toBe('doc200');

    collection.closeSync();
  });


  it('should handle update operations correctly', () => {
    const collection: ZVecCollection = ZVecOpen(testCollectionPath);
    expect(collection).toBeDefined();

    const updateResult = collection.updateSync({
      id: 'doc1',
      fields: {
        'title': 'updated_Product_1',
        'price': 10000
      }
    });
    expect(updateResult.ok).toBe(true);
    expect(updateResult.code).toBe('ZVEC_OK');

    const updateResults = collection.updateSync([
      {
        id: 'doc2',
        fields: { 'title': 'updated_Product_2' }
      },
      {
        id: 'doc3',
        fields: { 'title': 'updated_Product_3' }
      }
    ]);
    expect(updateResults[0].ok).toBe(true);
    expect(updateResults[0].code).toBe('ZVEC_OK');
    expect(updateResults[1].ok).toBe(true);
    expect(updateResults[1].code).toBe('ZVEC_OK');

    const fetchResult = collection.fetchSync('doc1');
    expect(fetchResult['doc1']).toBeDefined();
    expect(fetchResult['doc1'].fields.title).toBe('updated_Product_1');
    expect(fetchResult['doc1'].fields.price).toBeCloseTo(10000);

    const fetchResults = collection.fetchSync(['doc2', 'doc3']);
    expect(fetchResults['doc2']).toBeDefined();
    expect(fetchResults['doc2'].fields.title).toBe('updated_Product_2');
    expect(fetchResults['doc2'].fields.price).toBeCloseTo(2.99);
    expect(fetchResults['doc3']).toBeDefined();
    expect(fetchResults['doc3'].fields.title).toBe('updated_Product_3');
    expect(fetchResults['doc3'].fields.price).toBeCloseTo(3.99);

    collection.closeSync();
  });


  it('should handle upsert operations correctly', () => {
    const collection: ZVecCollection = ZVecOpen(testCollectionPath);
    expect(collection).toBeDefined();

    const upsertResult = collection.upsertSync({
      id: 'doc1',
      vectors: {
        'dense': Array.from({ length: dimension }, () => (0.7)),
        'sparse': { 9999: 0.5 }
      },
      fields: {
        'title': 'upserted_Product_1',
        'price': 99999
      }
    });
    expect(upsertResult.ok).toBe(true);
    expect(upsertResult.code).toBe('ZVEC_OK');

    const fetchResult = collection.fetchSync('doc1');
    expect(fetchResult['doc1']).toBeDefined();
    expect(fetchResult['doc1'].vectors.dense[0]).toBeCloseTo(0.7);
    expect(fetchResult['doc1'].vectors.sparse[9999]).toBeCloseTo(0.5);
    expect(fetchResult['doc1'].fields.title).toBe('upserted_Product_1');
    expect(fetchResult['doc1'].fields.price).toBeCloseTo(99999);

    collection.closeSync();
  });


  it('should handle delete operations correctly', () => {
    const collection: ZVecCollection = ZVecOpen(testCollectionPath);
    expect(collection).toBeDefined();

    const deleteSingleResult = collection.deleteSync('doc3');
    expect(deleteSingleResult.ok).toBe(true);
    expect(deleteSingleResult.code).toBe('ZVEC_OK');
    expect(collection.stats.docCount).toBe(500 - 1);
    const fetchResult = collection.fetchSync('doc3');
    expect('doc3' in fetchResult).toBe(false);

    const deleteByFilterResult = collection.deleteByFilterSync('price < 10');
    expect(deleteByFilterResult.ok).toBe(true);
    expect(deleteByFilterResult.code).toBe('ZVEC_OK');
    expect(collection.stats.docCount).toBe(500 - 8);
    const queryResult = collection.querySync({
      filter: 'price < 10'
    })
    expect(queryResult.length).toBe(0);

    collection.closeSync();
  });


});