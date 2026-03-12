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

// Read the new version from root package.json
const rootPkgJson = JSON.parse(fs.readFileSync(path.join(root, "package.json"), "utf8"));
const newVersion = rootPkgJson.version;

console.log("\n✅ Version bump completed");
console.log("\n📝 Next steps:");
console.log("   1. git add .");
console.log(`   2. git commit -m 'chore(release): bump version to v${newVersion} [skip ci]'`);
console.log("      ⚠️  IMPORTANT: Include [skip ci] to skip GitHub Actions");
console.log(`   3. git tag v${newVersion}`);
console.log("   4. git push && git push --tags");