import * as fs from 'fs';
import {
  ZVecCollection,
  ZVecCreateAndOpen,
  ZVecInitialize,
  ZVecLogLevel,
  ZVecLogType,
  isZVecError
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

    it('should respect fetch projection controls', () => {
      const doc = makeDoc(42, 1, 1);
      const fetched = collection.fetchSync({
        ids: doc.id,
        outputFields: ['title'],
        includeVector: false
      })[doc.id];
      expect(fetched.fields.title).toBe(doc.fields!.title);
      expect(fetched.fields.price).toBeUndefined();
      expect(Object.keys(fetched.vectors)).toHaveLength(0);
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

    it('should resolve concurrent async queries with correct results', async () => {
      const targets = [550, 600, 700, 800, 900, 950];
      const denseQueries = targets.map(k => {
        const doc = makeDoc(k, 1, 1);
        return collection.query({
          fieldName: 'dense', vector: doc.vectors!.dense, topk: 10, includeVector: true,
        }).then(results => ({ k, results }));
      });
      const sparseQueries = targets.map(k => {
        const doc = makeDoc(k, 1, 1);
        return collection.query({
          fieldName: 'sparse', vector: doc.vectors!.sparse, topk: 10, includeVector: true,
          filter: `title = "Product_${k}_v1"`
        }).then(results => ({ k, results }));
      });

      const denseResults = await Promise.all(denseQueries);
      for (const { k, results } of denseResults) {
        expect(results.length).toBe(10);
        expectDoc(results[0], k, 1, 1);
      }
      const sparseResults = await Promise.all(sparseQueries);
      for (const { k, results } of sparseResults) {
        expect(results.length).toBe(1);
        expectDoc(results[0], k, 1, 1);
      }
    });
  });


  describe('update', () => {
    it('should update scalar fields without changing vectors', () => {
      batch(collection, 'update', 501, 1000, 3, 1);
      verifyDocs(collection, 501, 1000, 3, 1);
    });

    it('should not affect docs outside the update range', () => {
      verifyDocs(collection, 1, 500, 2, 2);
      verifyDocs(collection, 1001, 1500, 2, 2);
    });

    it('should return correct results from vector query after field update', () => {
      const doc = makeDoc(750, 3, 1);
      const denseResults = collection.querySync({
        fieldName: 'dense', vector: doc.vectors!.dense, topk: 1, includeVector: true
      });
      expectDoc(denseResults[0], 750, 3, 1);
      const sparseResults = collection.querySync({
        fieldName: 'sparse', vector: doc.vectors!.sparse, topk: 1, includeVector: true
      });
      expectDoc(sparseResults[0], 750, 3, 1);
    });
  });


  describe('delete', () => {
    it('should delete a single doc by id', () => {
      const doc = makeDoc(1001, 2, 2);
      const result = collection.deleteSync(doc.id);
      expect(result.ok).toBe(true);
      const fetched = collection.fetchSync(doc.id);
      expect(fetched[doc.id]).toBeUndefined();
    });

    it('should delete a batch of docs by ids', () => {
      const ids = Array.from({ length: 100 }, (_, i) => makeDoc(1002 + i, 2, 2).id);
      const results = collection.deleteSync(ids);
      for (const status of results) {
        expect(status.ok).toBe(true);
      }
      const fetched = collection.fetchSync(ids);
      for (const id of ids) {
        expect(fetched[id]).toBeUndefined();
      }
    });

    it('should reflect correct docCount after deletions', () => {
      expect(collection.stats.docCount).toBe(1500 - 101);
    });

    it('should not affect remaining docs', () => {
      verifyDocs(collection, 1, 500, 2, 2);
      verifyDocs(collection, 501, 1000, 3, 1);
      verifyDocs(collection, 1102, 1500, 2, 2);
    });

    it('should delete docs matching a filter expression', async () => {
      const result = await collection.deleteByFilter('price > 2800 AND title LIKE "%_v2"');
      expect(result.ok).toBe(true);
      const fetched = collection.fetchSync(['doc_1400', 'doc_1450', 'doc_1500']);
      expect(fetched['doc_1400']).toBeUndefined();
      expect(fetched['doc_1450']).toBeUndefined();
      expect(fetched['doc_1500']).toBeUndefined();
    });

    it('should not affect docs outside the filter criteria', () => {
      verifyDocs(collection, 1, 500, 2, 2);
      verifyDocs(collection, 501, 1000, 3, 1);
      verifyDocs(collection, 1102, 1399, 2, 2);
    });
  });


  describe('error handling', () => {
    it('should throw on insert with missing required vector field', () => {
      try {
        collection.insertSync({ id: 'bad_1', fields: { title: 'no vectors' } } as any);
        fail('expected to throw');
      } catch (e) {
        expect(isZVecError(e)).toBe(true);
      }
    });

    it('should throw on insert with wrong vector dimension', () => {
      try {
        collection.insertSync({
          id: 'bad_2',
          vectors: { dense: [1, 2, 3], sparse: { 0: 1.0 } },
          fields: { title: 'wrong dim' }
        });
        fail('expected to throw');
      } catch (e) {
        expect(isZVecError(e)).toBe(true);
      }
    });

    it('should fail to insert a duplicate doc id', () => {
      const doc = makeDoc(1, 1, 1);
      const result = collection.insertSync(doc);
      expect(result.ok).toBe(false);
    });

    it('should fail to update a non-existent doc', () => {
      const result = collection.updateSync({ id: 'nonexistent', fields: { title: 'ghost' } });
      expect(result.ok).toBe(false);
    });

    it('should throw on query with invalid filter syntax', () => {
      const doc = makeDoc(1, 1, 1);
      try {
        collection.querySync({
          fieldName: 'dense', vector: doc.vectors!.dense, topk: 1,
          filter: 'invalid @@@ syntax'
        });
        fail('expected to throw');
      } catch (e) {
        expect(isZVecError(e)).toBe(true);
      }
    });

    it('should throw on query with non-existent vector field', () => {
      try {
        collection.querySync({ fieldName: 'nonexistent', vector: [1, 2, 3], topk: 1 });
        fail('expected to throw');
      } catch (e) {
        expect(isZVecError(e)).toBe(true);
      }
    });
  });
});
