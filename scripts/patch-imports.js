const fs = require('fs');
const path = require('path');

const targetDir = process.argv[2];
const packageName = process.argv[3];

if (!targetDir || !packageName) {
  console.error("Usage: node patch-imports.js <directory> <package-name>");
  process.exit(1);
}

function walk(dir) {
  for (const entry of fs.readdirSync(dir, { withFileTypes: true })) {
    const p = path.join(dir, entry.name);

    if (entry.isDirectory()) {
      walk(p);
    } else if (p.endsWith(".ts") || p.endsWith(".js")) {
      let content = fs.readFileSync(p, "utf8");

      const patched = content.replace(
        /from\s+['"]\.\.\/src[^'"]*['"]/g,
        `from '${packageName}'`
      );

      if (patched !== content) {
        fs.writeFileSync(p, patched);
      }
    }
  }
}

walk(targetDir);
