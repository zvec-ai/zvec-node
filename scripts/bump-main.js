#!/usr/bin/env node

const path = require('path');
const fs = require('fs');
const { execSync } = require("child_process");

const type = process.argv[2];

if (!["patch", "minor", "major"].includes(type)) {
  console.error("Usage: npm run bump:main <patch|minor|major>");
  process.exit(1);
}

const root = path.resolve(__dirname, "..");
const packagesDir = path.join(root, "packages");
const rootPkgPath = path.join(root, "package.json");

const rootPkg = JSON.parse(fs.readFileSync(rootPkgPath, "utf8"));
const optDeps = rootPkg.optionalDependencies || {};

const dirs = fs.readdirSync(packagesDir);

for (const dir of dirs) {
  const pkgJsonPath = path.join(packagesDir, dir, "package.json");
  if (!fs.existsSync(pkgJsonPath)) continue;

  const pkg = JSON.parse(fs.readFileSync(pkgJsonPath, "utf8"));
  if (optDeps[pkg.name] !== undefined) {
    console.log(`Updating ${pkg.name}: ${optDeps[pkg.name]} → ${pkg.version}`);
    optDeps[pkg.name] = pkg.version;
  }
}

rootPkg.optionalDependencies = optDeps;
fs.writeFileSync(rootPkgPath, JSON.stringify(rootPkg, null, 2) + "\n");

console.log(`\nBumping main package (${type})`);

execSync(`npm version ${type} --no-git-tag-version`, {
  cwd: root,
  stdio: "inherit",
});

// Read the new version
const newVersion = JSON.parse(fs.readFileSync(rootPkgPath, "utf8")).version;

console.log("\n✅ Main package version bump completed");
console.log("\n📝 Next steps:");
console.log("   1. npm install --ignore-scripts");
console.log("   2. git add .");
console.log(`   3. git commit -m 'chore(release): bump main to v${newVersion}'`);
console.log(`   4. git tag v${newVersion}`);
console.log("   5. git push && git push --tags");
console.log("   6. npm run publish:main");
