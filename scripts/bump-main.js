#!/usr/bin/env node

const path = require('path');
const fs = require('fs');
const { execSync } = require("child_process");
const {
  assertUniformBindingVersion,
  getBindingPackages,
  syncBindingOptionalDependencies,
} = require('./binding-packages');

const type = process.argv[2];

if (!["patch", "minor", "major"].includes(type)) {
  console.error("Usage: npm run bump:main <patch|minor|major>");
  process.exit(1);
}

const root = path.resolve(__dirname, "..");
const rootPkgPath = path.join(root, "package.json");

const rootPkg = JSON.parse(fs.readFileSync(rootPkgPath, "utf8"));
const bindingPackages = getBindingPackages(root);
assertUniformBindingVersion(bindingPackages);

for (const { packageJson } of bindingPackages) {
  const oldVersion = rootPkg.optionalDependencies?.[packageJson.name];
  console.log(`Updating ${packageJson.name}: ${oldVersion ?? '(missing)'} → ${packageJson.version}`);
}

syncBindingOptionalDependencies(rootPkg, bindingPackages);
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
