#!/usr/bin/env node


const platform = process.platform;
const arch = process.arch;
const packageName = `@zvec/bindings-${platform}-${arch}`;


try {
    require.resolve(packageName);
} catch (error) {
    console.error(`❌ zvec Error: Failed to install prebuilt binary for ${platform}-${arch}`);
    process.exit(1);
}