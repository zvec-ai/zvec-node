import {
  ZVecCollectionSchema,
  ZVecDataType,
  ZVecHnswIndexParams,
  ZVecIndexType,
  ZVecInvertIndexParams,
  ZVecIVFIndexParams,
  ZVecMetricType,
  ZVecQuantizeType
} from '../src/index';


describe('CollectionSchema', () => {
  it('should create a simple schema correctly', () => {
    const schema = new ZVecCollectionSchema({
      name: 'test_collection',
      vectors: {
        name: 'vector',
        dataType: ZVecDataType.VECTOR_FP32,
        dimension: 10,
        indexParams: {
          indexType: ZVecIndexType.HNSW,
          metricType: ZVecMetricType.COSINE,
          m: 500,
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

    expect(schema).toBeInstanceOf(ZVecCollectionSchema);
    expect(schema.name).toBe('test_collection');

    const vectors = schema.vectors();
    expect(vectors.length).toBe(1);
    expect(vectors[0].name).toBe('vector');
    expect(vectors[0].dataType).toBe(ZVecDataType.VECTOR_FP32);
    expect(vectors[0].dimension).toBe(10);
    expect((vectors[0].indexParams as ZVecHnswIndexParams).indexType).toBe(ZVecIndexType.HNSW);
    expect((vectors[0].indexParams as ZVecHnswIndexParams).metricType).toBe(ZVecMetricType.COSINE);
    expect((vectors[0].indexParams as ZVecHnswIndexParams)["m"]).toBe(500);
    expect((vectors[0].indexParams as ZVecHnswIndexParams)["efConstruction"]).toBe(500);
    expect((vectors[0].indexParams as ZVecHnswIndexParams).quantizeType).toBe(ZVecQuantizeType.FP16);

    const fields = schema.fields();
    expect(fields.length).toBe(1);
    expect(fields[0].name).toBe('tag');
    expect(fields[0].dataType).toBe(ZVecDataType.ARRAY_STRING);
    expect(fields[0].nullable).toBe(true);
    expect((fields[0].indexParams as ZVecInvertIndexParams).indexType).toBe(ZVecIndexType.INVERT);

    expect(schema.vector('vector').name).toBe('vector');
    expect(schema.vector('vector').dataType).toBe(ZVecDataType.VECTOR_FP32);
    expect(schema.vector('vector').dimension).toBe(10);
    expect((schema.vector('vector').indexParams as ZVecHnswIndexParams).indexType).toBe(ZVecIndexType.HNSW);

    expect(schema.field('tag').name).toBe('tag');
    expect(schema.field('tag').dataType).toBe(ZVecDataType.ARRAY_STRING);
    expect((schema.field('tag').indexParams as ZVecInvertIndexParams).indexType).toBe(ZVecIndexType.INVERT);
  });


  it('should create a complex schema correctly', () => {
    const schema = new ZVecCollectionSchema({
      name: 'test_collection',
      vectors: [
        {
          name: 'vector1',
          dataType: ZVecDataType.VECTOR_FP32,
          dimension: 10,
          indexParams: {
            indexType: ZVecIndexType.HNSW,
            metricType: ZVecMetricType.COSINE,
            m: 1000,
            quantizeType: ZVecQuantizeType.FP16
          }
        },
        {
          name: 'vector2',
          dataType: ZVecDataType.VECTOR_FP16,
          dimension: 4,
          indexParams: {
            indexType: ZVecIndexType.IVF,
            nList: 101,
            nIters: 202
          }
        },
        {
          name: 'vector3',
          dataType: ZVecDataType.SPARSE_VECTOR_FP32,
          indexParams: {
            indexType: ZVecIndexType.HNSW,
            metricType: ZVecMetricType.IP
          }
        }
      ],
      fields: [
        {
          name: 'title',
          dataType: ZVecDataType.STRING
        },
        {
          name: 'price',
          dataType: ZVecDataType.INT32,
          indexParams: {
            indexType: ZVecIndexType.INVERT,
            enableRangeOptimization: true
          }
        },
        {
          name: 'tag',
          dataType: ZVecDataType.ARRAY_STRING,
          nullable: true,
          indexParams: { indexType: ZVecIndexType.INVERT }
        }
      ]
    });

    expect(schema).toBeInstanceOf(ZVecCollectionSchema);
    expect(schema.name).toBe('test_collection');

    const vectors = schema.vectors();
    expect(vectors.length).toBe(3);
    expect(vectors[0].name).toBe('vector1');
    expect(vectors[0].dataType).toBe(ZVecDataType.VECTOR_FP32);
    expect(vectors[0].dimension).toBe(10);
    expect(vectors[0].indexParams!.indexType).toBe(ZVecIndexType.HNSW);
    expect(vectors[0].indexParams!.metricType).toBe(ZVecMetricType.COSINE);
    expect((vectors[0].indexParams as ZVecHnswIndexParams).m).toBe(1000);
    expect((vectors[0].indexParams as ZVecHnswIndexParams).efConstruction).toBe(500);
    expect((vectors[0].indexParams as ZVecHnswIndexParams).quantizeType).toBe(ZVecQuantizeType.FP16);
    expect(vectors[1].name).toBe('vector2');
    expect(vectors[1].dataType).toBe(ZVecDataType.VECTOR_FP16);
    expect(vectors[1].dimension).toBe(4);
    expect(vectors[1].indexParams!.indexType).toBe(ZVecIndexType.IVF);
    expect(vectors[1].indexParams!.metricType).toBe(ZVecMetricType.IP);
    expect((vectors[1].indexParams as ZVecIVFIndexParams).nList).toBe(101);
    expect((vectors[1].indexParams as ZVecIVFIndexParams).nIters).toBe(202);
    expect((vectors[1].indexParams as ZVecIVFIndexParams).quantizeType).toBe(ZVecQuantizeType.UNDEFINED);
    expect(vectors[2].name).toBe('vector3');
    expect(vectors[2].dataType).toBe(ZVecDataType.SPARSE_VECTOR_FP32);
    expect(vectors[2].dimension).toBe(0);
    expect(vectors[2].indexParams!.indexType).toBe(ZVecIndexType.HNSW);
    expect(vectors[2].indexParams!.metricType).toBe(ZVecMetricType.IP);
    expect((vectors[2].indexParams as ZVecHnswIndexParams).m).toBe(50);
    expect((vectors[2].indexParams as ZVecHnswIndexParams).efConstruction).toBe(500);
    expect((vectors[2].indexParams as ZVecHnswIndexParams).quantizeType).toBe(ZVecQuantizeType.UNDEFINED);

    const fields = schema.fields();
    expect(fields.length).toBe(3);
    expect(fields[0].name).toBe('title');
    expect(fields[0].dataType).toBe(ZVecDataType.STRING);
    expect(fields[0].nullable).toBe(false);
    expect("indexParams" in fields[0]).toBe(false);
    expect(fields[1].name).toBe('price');
    expect(fields[1].dataType).toBe(ZVecDataType.INT32);
    expect(fields[1].nullable).toBe(false);
    expect(fields[1].indexParams!.indexType).toBe(ZVecIndexType.INVERT);
    expect((fields[1].indexParams as ZVecInvertIndexParams).enableRangeOptimization).toBe(true);
    expect((fields[1].indexParams as ZVecInvertIndexParams).enableExtendedWildcard).toBe(false);
    expect(fields[2].name).toBe('tag');
    expect(fields[2].dataType).toBe(ZVecDataType.ARRAY_STRING);
    expect(fields[2].nullable).toBe(true);
    expect(fields[2].indexParams!.indexType).toBe(ZVecIndexType.INVERT);

    expect(schema.vector('vector1').name).toBe('vector1');
    expect(schema.vector('vector1').dataType).toBe(ZVecDataType.VECTOR_FP32);
    expect(schema.vector('vector1').dimension).toBe(10);
    expect(schema.vector('vector1').indexParams!.indexType).toBe(ZVecIndexType.HNSW);
    expect(schema.vector('vector2').name).toBe('vector2');
    expect(schema.vector('vector2').dataType).toBe(ZVecDataType.VECTOR_FP16);
    expect(schema.vector('vector2').dimension).toBe(4);
    expect(schema.vector('vector2').indexParams!.indexType).toBe(ZVecIndexType.IVF);
    expect(schema.vector('vector3').name).toBe('vector3');
    expect(schema.vector('vector3').dataType).toBe(ZVecDataType.SPARSE_VECTOR_FP32);
    expect(schema.vector('vector3').indexParams!.indexType).toBe(ZVecIndexType.HNSW);

    expect(schema.field('title').name).toBe('title');
    expect(schema.field('title').dataType).toBe(ZVecDataType.STRING);
    expect("indexParams" in schema.field('title')).toBe(false);
    expect(schema.field('price').name).toBe('price');
    expect(schema.field('price').dataType).toBe(ZVecDataType.INT32);
    expect(schema.field('price').indexParams!.indexType).toBe(ZVecIndexType.INVERT);
    expect(schema.field('tag').name).toBe('tag');
    expect(schema.field('tag').dataType).toBe(ZVecDataType.ARRAY_STRING);
    expect(schema.field('tag').indexParams!.indexType).toBe(ZVecIndexType.INVERT);
  });
});