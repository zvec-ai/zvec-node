#!/usr/bin/env node


try {
  const zvec = require('../src/index');
  console.log('✓ zvec loaded successfully');

  if (!zvec.ZVecDataType) {
    console.error('✗ ZVecDataType not found');
    process.exit(1);
  }

  if (!zvec.ZVecIndexType) {
    console.error('✗ ZVecIndexType not found');
    process.exit(1);
  }

  if (!zvec.ZVecMetricType) {
    console.error('✗ ZVecMetricType not found');
    process.exit(1);
  }

  if (!zvec.ZVecQuantizeType) {
    console.error('✗ ZVecQuantizeType not found');
    process.exit(1);
  }

  if (!zvec.ZVecCreateAndOpen) {
    console.error('✗ ZVecCreateAndOpen not found');
    process.exit(1);
  }

  if (!zvec.ZVecOpen) {
    console.error('✗ ZVecOpen not found');
    process.exit(1);
  }
} catch (error) {
  console.error('✗ Error loading zvec: ', error.message);
  process.exit(1);
}