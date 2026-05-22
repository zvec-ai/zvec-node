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
    it('should reach full index completeness', async () => {
      await collection.optimize();
      expect(collection.stats.indexCompleteness['dense']).toBeCloseTo(1);
      expect(collection.stats.indexCompleteness['sparse']).toBeCloseTo(1);
    });

    it('should still return correct data after optimize', () => {
      verifyDocs(collection, 1, 1000, 1, 1);
    });

    it('should return correct results from async query', async () => {
      const doc = makeDoc(42, 1, 1);
      const results = await collection.query({
        fieldName: 'dense', vector: doc.vectors!.dense, topk: 1, includeVector: true
      });
      expectDoc(results[0], 42, 1, 1);
    });
  });
});
