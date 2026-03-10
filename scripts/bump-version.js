#!/usr/bin/env node

const path = require('path');
const fs = require('fs');
const { execSync } = require("child_process");

const type = process.argv[2];

if (!["patch", "minor", "major"].includes(type)) {
  console.error("Usage: npm run version <patch|minor|major>");
  process.exit(1);
}

const root = path.resolve(__dirname, "..");
const packagesDir = path.join(root, "packages");

const dirs = fs.readdirSync(packagesDir);

for (const dir of dirs) {
  const pkgDir = path.join(packagesDir, dir);
  const pkgJson = path.join(pkgDir, "package.json");

  if (!fs.existsSync(pkgJson)) continue;

  console.log(`\nBumping ${dir} (${type})`);

  execSync(`npm version ${type} --no-git-tag-version`, {
    cwd: pkgDir,
    stdio: "inherit",
  });
}

execSync(`npm version ${type} --no-git-tag-version`, {
  cwd: root,
  stdio: "inherit",
});

console.log("\n✅ Version bump completed");