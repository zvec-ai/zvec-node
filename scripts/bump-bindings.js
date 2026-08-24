#!/usr/bin/env node

const { execSync } = require("child_process");
const path = require('path');
const {
  assertUniformBindingVersion,
  getBindingPackages,
} = require('./binding-packages');

const type = process.argv[2];

if (!["patch", "minor", "major"].includes(type)) {
  console.error("Usage: npm run bump:bindings <patch|minor|major>");
  process.exit(1);
}

const root = path.resolve(__dirname, "..");
const bindingPackages = getBindingPackages(root);
assertUniformBindingVersion(bindingPackages);

for (const { target, packageDir } of bindingPackages) {
  console.log(`\nBumping bindings-${target} (${type})`);

  execSync(`npm version ${type} --no-git-tag-version`, {
    cwd: packageDir,
    stdio: "inherit",
  });
}

const updatedBindingPackages = getBindingPackages(root);
const newVersion = assertUniformBindingVersion(updatedBindingPackages);

console.log("\n✅ Bindings version bump completed");
console.log("\n📝 Next steps:");
console.log("   1. git add .");
console.log(`   2. git commit -m 'chore(release): bump bindings to v${newVersion}'`);
console.log("   3. git push");
console.log("   4. Trigger the 'Publish to npm' workflow on GitHub Actions");
console.log(`   5. After bindings are published, run: npm run bump:main <patch|minor|major>`);
