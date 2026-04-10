#!/usr/bin/env node

const path = require('path');
const fs = require('fs');
const { execSync } = require("child_process");

const type = process.argv[2];

if (!["patch", "minor", "major"].includes(type)) {
  console.error("Usage: npm run bump:bindings <patch|minor|major>");
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

// Read the new version from any binding package
const firstDir = dirs.find(dir => fs.existsSync(path.join(packagesDir, dir, "package.json")));
const newVersion = JSON.parse(fs.readFileSync(path.join(packagesDir, firstDir, "package.json"), "utf8")).version;

console.log("\n✅ Bindings version bump completed");
console.log("\n📝 Next steps:");
console.log("   1. git add .");
console.log(`   2. git commit -m 'chore(release): bump bindings to v${newVersion} [skip ci]'`);
console.log("   3. git push");
console.log("   4. Trigger the 'Publish to npm' workflow on GitHub Actions");
console.log(`   5. After bindings are published, run: npm run bump:main <patch|minor|major>`);
