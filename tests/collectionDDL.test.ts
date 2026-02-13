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
  ZVecQuantizeType
} from '../src/index';


ZVecInitialize({
  logType: ZVecLogType.CONSOLE,
  logLevel: ZVecLogLevel.WARN,
});


describe('Collection Data Definition Operations', () => {
  const testCollectionName = 'test_ddl_collection';
  const testCollectionPath = './test_ddl_collection';


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


  it('should create a simple collection successfully', () => {
    const schema = new ZVecCollectionSchema({
      name: testCollectionName,
      vectors: {
        name: 'vector',
        dataType: ZVecDataType.VECTOR_FP32,
        dimension: 10,
        indexParams: {
          indexType: ZVecIndexType.HNSW,
          metricType: ZVecMetricType.COSINE,
          m: 77,
          quantizeType: ZVecQuantizeType.FP16
        }
      },
      fields: {
        name: 'tag',
        dataType: ZVecDataType.ARRAY_STRING,
        nullable: true,
        indexParams: { indexType: ZVecIndexType.INVERT }
      }
    });

    const collection: ZVecCollection = ZVecCreateAndOpen(testCollectionPath, schema);
    expect(collection).toBeDefined();

    expect(collection.path).toBe(testCollectionPath);

    expect(collection.schema.name).toBe(testCollectionName);
    const vectors = collection.schema.vectors();
    expect(vectors.length).toBe(1);
    expect(vectors[0].name).toBe('vector');
    expect(vectors[0].dataType).toBe(ZVecDataType.VECTOR_FP32);
    expect(vectors[0].dimension).toBe(10);
    expect(vectors[0].indexParams!.indexType).toBe(ZVecIndexType.HNSW);
    expect(vectors[0].indexParams!.metricType).toBe(ZVecMetricType.COSINE);
    expect((vectors[0].indexParams as ZVecHnswIndexParams)["m"]).toBe(77);
    expect(vectors[0].indexParams!.quantizeType).toBe(ZVecQuantizeType.FP16);
    const fields = collection.schema.fields();
    expect(fields.length).toBe(1);
    expect(fields[0].name).toBe('tag');
    expect(fields[0].dataType).toBe(ZVecDataType.ARRAY_STRING);
    expect(fields[0].nullable).toBe(true);
    expect(fields[0].indexParams!.indexType).toBe(ZVecIndexType.INVERT);

    expect(collection.stats.docCount).toBe(0);

    collection.closeSync();
  });


  it('should open an existing collection successfully', () => {
    const collection: ZVecCollection = ZVecOpen(testCollectionPath);
    expect(collection).toBeDefined();

    expect(collection.path).toBe(testCollectionPath);

    expect(collection.schema.name).toBe(testCollectionName);
    const vectors = collection.schema.vectors();
    expect(vectors.length).toBe(1);
    expect(vectors[0].name).toBe('vector');
    expect(vectors[0].dataType).toBe(ZVecDataType.VECTOR_FP32);
    expect(vectors[0].dimension).toBe(10);
    expect(vectors[0].indexParams!.indexType).toBe(ZVecIndexType.HNSW);
    expect(vectors[0].indexParams!.metricType).toBe(ZVecMetricType.COSINE);
    expect((vectors[0].indexParams as ZVecHnswIndexParams)["m"]).toBe(77);
    expect(vectors[0].indexParams!.quantizeType).toBe(ZVecQuantizeType.FP16);
    const fields = collection.schema.fields();
    expect(fields.length).toBe(1);
    expect(fields[0].name).toBe('tag');
    expect(fields[0].dataType).toBe(ZVecDataType.ARRAY_STRING);
    expect(fields[0].nullable).toBe(true);
    expect(fields[0].indexParams!.indexType).toBe(ZVecIndexType.INVERT);

    expect(collection.stats.docCount).toBe(0);

    collection.closeSync();
  });


  it('should insert sample documents to the collection', () => {
    const collection: ZVecCollection = ZVecOpen(testCollectionPath);
    expect(collection).toBeDefined();

    const singleResult = collection.insertSync({
      id: 'doc1',
      vectors: { 'vector': Array.from({ length: 10 }, () => Math.random()) },
      fields: { 'tag': ['Music', 'Sports'] }
    });
    expect(singleResult.ok).toBe(true);
    expect(singleResult.code).toBe('ZVEC_OK');

    const sampleDocuments = [
      {
        id: 'doc2',
        vectors: { vector: Array.from({ length: 10 }, () => Math.random()) },
        fields: { tag: ['Music', 'Movie'] }
      },
      {
        id: 'doc3',
        vectors: { vector: Array.from({ length: 10 }, () => Math.random()) },
        fields: { tag: ['Literature', 'Sports'] }
      }
    ];

    const multiResults = collection.insertSync(sampleDocuments);
    expect(multiResults.length).toBe(2);
    expect(multiResults[0].ok).toBe(true);
    expect(multiResults[0].code).toBe('ZVEC_OK');
    expect(multiResults[1].ok).toBe(true);
    expect(multiResults[1].code).toBe('ZVEC_OK');

    expect(collection.stats.docCount).toBe(3);

    collection.closeSync();
  });


  it('should add more columns successfully', () => {
    const collection: ZVecCollection = ZVecOpen(testCollectionPath);
    expect(collection).toBeDefined();

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
    const tagField = collection.schema.field('tag');
    expect(tagField.name).toBe('tag');
    expect(tagField.dataType).toBe(ZVecDataType.ARRAY_STRING);
    expect(tagField.nullable).toBe(true);
    expect(tagField.indexParams!.indexType).toBe(ZVecIndexType.INVERT);
    const priceField = collection.schema.field('price');
    expect(priceField.name).toBe('price');
    expect(priceField.dataType).toBe(ZVecDataType.INT64);
    expect(priceField.nullable).toBe(true);
    expect(priceField.indexParams!.indexType).toBe(ZVecIndexType.INVERT);

    const insertResult = collection.insertSync({
      id: 'doc4',
      vectors: { 'vector': Array.from({ length: 10 }, () => Math.random()) },
      fields: {
        'tag': ['Technology'],
        'price': 199,
      }
    });
    expect(insertResult.ok).toBe(true);
    const fetchResult = collection.fetchSync('doc4');
    expect(fetchResult['doc4'].fields['price']).toBe(199);

    // Test adding a duplicate column
    let errorOccurred = false;
    let errorCode: string = '';
    try {
      collection.addColumnSync({
        fieldSchema: {
          name: 'price',
          dataType: ZVecDataType.UINT32,
          nullable: true
        }
      });
    } catch (error) {
      if (isZVecError(error)) {
        errorOccurred = true;
        errorCode = error.code;
      }
      else {
        throw (error);
      }
    }
    expect(errorOccurred).toBe(true);
    expect(errorCode).toBe('ZVEC_INVALID_ARGUMENT');

    collection.closeSync();
  });


  it('should rename a column successfully', () => {
    const collection: ZVecCollection = ZVecOpen(testCollectionPath);
    expect(collection).toBeDefined();

    collection.alterColumnSync({
      columnName: 'price',
      newColumnName: 'price-usd'
    });

    expect(collection.schema.fields().length).toBe(2);
    const priceUSD = collection.schema.field('price-usd');
    expect(priceUSD.name).toBe('price-usd');
    expect(priceUSD.dataType).toBe(ZVecDataType.INT64);

    // Test getting a non-existing column
    let errorOccurred = false;
    let errorCode: string = '';
    try {
      const field = collection.schema.field('price');
      console.log(field);
    } catch (error) {
      if (isZVecError(error)) {
        errorOccurred = true;
        errorCode = error.code;
      }
      else {
        throw (error);
      }
    }
    expect(errorOccurred).toBe(true);
    expect(errorCode).toBe('ZVEC_NOT_FOUND');

    collection.closeSync();
  });


  it('should drop a column successfully', () => {
    const collection: ZVecCollection = ZVecOpen(testCollectionPath);
    expect(collection).toBeDefined();

    let errorOccurred = false;
    let errorCode: string = '';

    collection.dropColumnSync('price-usd');

    expect(collection.schema.fields().length).toBe(1);
    try {
      collection.schema.field('price-usd');
    } catch (error) {
      if (isZVecError(error)) {
        errorOccurred = true;
        errorCode = error.code;
      }
      else {
        throw (error);
      }
    }
    expect(errorOccurred).toBe(true);
    expect(errorCode).toBe('ZVEC_NOT_FOUND');

    // Test dropping a non-existing column
    errorOccurred = false;
    errorCode = '';
    try {
      collection.dropColumnSync('price');
    } catch (error) {
      if (isZVecError(error)) {
        errorOccurred = true;
        errorCode = error.code;
      }
      else {
        throw (error);
      }
    }
    expect(errorOccurred).toBe(true);
    expect(errorCode).toBe('ZVEC_INVALID_ARGUMENT');


    collection.closeSync();
  });


  it('should destroy a collection successfully', () => {
    const collection: ZVecCollection = ZVecOpen(testCollectionPath);
    expect(collection).toBeDefined();
    collection.destroySync();
    expect(fs.existsSync(testCollectionPath)).toBe(false);
  });
});