import * as fs from 'fs';
import {
  ZVecCollection,
  ZVecCreateAndOpen,
  ZVecInitialize,
  ZVecLogLevel,
  ZVecLogType
} from '../../src/index';
import { batch, createTestSchema, expectDoc, makeDoc, verifyDocs } from './helpers';


const COLLECTION_PATH = './test_operations_collection';


ZVecInitialize({ logType: ZVecLogType.CONSOLE, logLevel: ZVecLogLevel.WARN });


describe('Data Operations Pipeline', () => {
  let collection: ZVecCollection;

  beforeAll(() => {
    if (fs.existsSync(COLLECTION_PATH)) {
      fs.rmSync(COLLECTION_PATH, { recursive: true, force: true });
    }
    collection = ZVecCreateAndOpen(COLLECTION_PATH, createTestSchema('operations_test'));
  });

  afterAll(() => {
    collection?.destroySync();
    if (fs.existsSync(COLLECTION_PATH)) {
      fs.rmSync(COLLECTION_PATH, { recursive: true, force: true });
    }
  });


  describe('insert', () => {
    it('should insert 1000 docs and verify all', () => {
      batch(collection, 'insert', 1, 1000, 1, 1);
      expect(collection.stats.docCount).toBe(1000);
      verifyDocs(collection, 1, 1000, 1, 1);
    });

    it('should return correct results from vector query', () => {
      const doc = makeDoc(42, 1, 1);
      const results = collection.querySync({
        fieldName: 'dense', vector: doc.vectors!.dense, topk: 1, includeVector: true
      });
      expectDoc(results[0], 42, 1, 1);
    });
  });


  describe('optimize', () => {
    it('should reach full index completeness', () => {
      collection.optimizeSync();
      expect(collection.stats.indexCompleteness['dense']).toBeCloseTo(1);
      expect(collection.stats.indexCompleteness['sparse']).toBeCloseTo(1);
    });

    it('should still return correct data after optimize', () => {
      verifyDocs(collection, 1, 1000, 1, 1);
    });

    it('should return correct results from vector query', () => {
      const doc = makeDoc(42, 1, 1);
      const denseResults = collection.querySync({
        fieldName: 'dense', vector: doc.vectors!.dense, topk: 1, includeVector: true
      });
      expectDoc(denseResults[0], 42, 1, 1);
      const sparseResults = collection.querySync({
        fieldName: 'sparse', vector: doc.vectors!.sparse, topk: 1, includeVector: true
      });
      expectDoc(sparseResults[0], 42, 1, 1);
    });

    it('should respect outputFields selection', () => {
      const doc = makeDoc(42, 1, 1);
      const results = collection.querySync({
        fieldName: 'dense', vector: doc.vectors!.dense, topk: 1,
        outputFields: ['title']
      });
      expect(results[0].fields.title).toBeDefined();
      expect(results[0].fields.price).toBeUndefined();
    });
  });


  describe('upsert', () => {
    it('should upsert existing docs with new versions', () => {
      batch(collection, 'upsert', 1, 500, 2, 2);
      verifyDocs(collection, 1, 500, 2, 2);
    });

    it('should not affect other docs', () => {
      verifyDocs(collection, 501, 1000, 1, 1);
    });

    it('should upsert new docs beyond the original range', () => {
      batch(collection, 'upsert', 1001, 1500, 2, 2);
      expect(collection.stats.docCount).toBe(1500);
      verifyDocs(collection, 1001, 1500, 2, 2);
    });

  });


  describe('async operations', () => {
    it('should optimize asynchronously without blocking the event loop', async () => {
      let eventLoopRanDuring = false;
      const promise = collection.optimize();
      setImmediate(() => { eventLoopRanDuring = true; });
      await promise;

      expect(eventLoopRanDuring).toBe(true);
      expect(collection.stats.indexCompleteness['dense']).toBeCloseTo(1);
      expect(collection.stats.indexCompleteness['sparse']).toBeCloseTo(1);
    });

    // it('should resolve concurrent async queries with correct results', async () => {
    //   const targets = [550, 600, 700, 800, 900, 950];
    //   const queries = targets.map(k => {
    //     const doc = makeDoc(k, 1, 1);
    //     return collection.query({
    //       fieldName: 'dense', vector: doc.vectors!.dense, topk: 10, includeVector: true,
    //     }).then(results => ({ k, results }));
    //   });

    //   const allResults = await Promise.all(queries);
    //   for (const { k, results } of allResults) {
    //     expectDoc(results[0], k, 1, 1);
    //   }
    // });
  });
});
