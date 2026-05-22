import * as fs from 'fs';
import {
  ZVecCollection,
  ZVecCreateAndOpen,
  ZVecInitialize,
  ZVecLogLevel,
  ZVecLogType,
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
      const results = collection.querySync({
        fieldName: 'dense', vector: doc.vectors!.dense, topk: 1, includeVector: true
      });
      expectDoc(results[0], 42, 1, 1);
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
      batch(collection, 'upsert', 1001, 1500, 1, 1);
      expect(collection.stats.docCount).toBe(1500);
      verifyDocs(collection, 1001, 1500, 1, 1);
    });

  });

  // NOTE: update, delete, and re-optimize tests are blocked by an engine bug.
  // ReduceVectorIndex in segment_helper.cc uses MakeQuantizeVectorIndexPath
  // for the primary index when quantization is enabled, causing "Failed to open index"
  // on any re-optimize after new data is written to an already-optimized collection.
});
