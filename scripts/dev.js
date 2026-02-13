#!/usr/bin/env node


const { execSync } = require('child_process');
const fs = require('fs');
const path = require('path');


try {
    const PACKAGE_ROOT = path.resolve(__dirname, '..');

    // Build the bindings
    execSync('npm run build', { stdio: 'inherit', cwd: PACKAGE_ROOT });

    // Verify the platform package exists
    const platform = process.platform;
    const arch = process.arch;
    const platformPackageDir = path.join(PACKAGE_ROOT, 'packages', `bindings-${platform}-${arch}`);
    const targetPath = path.join(platformPackageDir, 'zvec_node_binding.node');
    if (!fs.existsSync(targetPath)) {
        throw new Error(`Platform package does not exist: ${targetPath}`);
    }

    // Pack
    const packResult = execSync('npm pack', { cwd: platformPackageDir, encoding: 'utf8' });
    const lines = packResult.trim().split('\n');
    const tarballFilename = lines[lines.length - 1].trim();
    const tarballPath = path.join(platformPackageDir, tarballFilename);
    if (!fs.existsSync(tarballPath)) {
        throw new Error(`Tarball not found: ${tarballPath}`);
    }

    // Install from local package
    execSync(`npm install ${tarballPath}`, { stdio: 'inherit', cwd: PACKAGE_ROOT });
} catch (error) {
    console.error('❌ Error during build and packaging:', error.message);
    process.exit(1);
}