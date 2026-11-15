const fs = require("node:fs");
const path = require("node:path");

const configFiles = [
    ".vscode/extensions.json",
    ".vscode/settings.json",
    ".vscode/tasks.json",
];

for (const file of configFiles) {
    const fullPath = path.join(__dirname, "..", file);
    JSON.parse(fs.readFileSync(fullPath, "utf8"));
    console.log(`Valid JSON: ${file}`);
}
