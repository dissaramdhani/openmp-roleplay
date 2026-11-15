"use strict";

const fs = require("node:fs");
const path = require("node:path");

const repositoryRoot = path.resolve(__dirname, "..");
const gamemodeRoot = path.join(repositoryRoot, "gamemodes");
const layerDependencies = {
    domain: new Set(["domain"]),
    application: new Set(["domain", "application"]),
    infrastructure: new Set(["domain", "application", "infrastructure"]),
    presentation: new Set(["domain", "application", "presentation"]),
    composition: new Set([
        "composition",
        "domain",
        "application",
        "infrastructure",
        "presentation",
    ]),
};
const externalDependencyLayers = new Set([
    "composition",
    "infrastructure",
    "presentation",
]);
const registeredModules = [
    "roleplay/application/startup.inc",
    "roleplay/infrastructure/adapters/console_logging.inc",
    "roleplay/presentation/server_callbacks.inc",
];

function isDependencyAllowed(sourceLayer, targetLayer) {
    return (
        Object.hasOwn(layerDependencies, sourceLayer) &&
        layerDependencies[sourceLayer].has(targetLayer)
    );
}

function isExternalDependencyAllowed(sourceLayer) {
    return externalDependencyLayers.has(sourceLayer);
}

function collectPawnFiles(directory) {
    return fs.readdirSync(directory, { withFileTypes: true }).flatMap((entry) => {
        const entryPath = path.join(directory, entry.name);
        if (entry.isDirectory()) {
            return collectPawnFiles(entryPath);
        }

        return /\.(inc|pwn)$/i.test(entry.name) ? [entryPath] : [];
    });
}

function classifyFile(filePath) {
    const relativePath = path
        .relative(gamemodeRoot, filePath)
        .split(path.sep)
        .join("/");

    if (relativePath === "roleplay.pwn" || relativePath === "roleplay/modules.inc") {
        return "composition";
    }

    const match = relativePath.match(
        /^roleplay\/(domain|application|infrastructure|presentation)\//,
    );
    return match ? match[1] : null;
}

function getLocalIncludes(source) {
    return [...source.matchAll(/^\s*#include\s+"([^"]+)"/gm)].map(
        (match) => match[1],
    );
}

function getExternalIncludes(source) {
    return [...source.matchAll(/^\s*#include\s+<([^>]+)>/gm)].map(
        (match) => match[1],
    );
}

function checkPawnArchitecture() {
    const errors = [];
    const pawnFiles = collectPawnFiles(gamemodeRoot);
    const registryPath = path.join(gamemodeRoot, "roleplay", "modules.inc");
    const registry = fs.readFileSync(registryPath, "utf8");

    for (const modulePath of registeredModules) {
        if (!registry.includes(`"${modulePath}"`)) {
            errors.push(`Composition root does not register "${modulePath}".`);
        }
    }

    for (const filePath of pawnFiles) {
        const sourceLayer = classifyFile(filePath);
        const relativeFile = path.relative(repositoryRoot, filePath);

        if (!sourceLayer) {
            errors.push(`Pawn file is outside the defined layers: ${relativeFile}`);
            continue;
        }

        const source = fs.readFileSync(filePath, "utf8");
        for (const includePath of getExternalIncludes(source)) {
            if (!isExternalDependencyAllowed(sourceLayer)) {
                errors.push(
                    `${relativeFile} (${sourceLayer}) cannot depend on external include <${includePath}>.`,
                );
            }
        }

        for (const includePath of getLocalIncludes(source)) {
            const resolvedPath = path.resolve(gamemodeRoot, includePath);
            if (!resolvedPath.startsWith(`${gamemodeRoot}${path.sep}`)) {
                errors.push(
                    `${relativeFile} includes a path outside gamemodes: ${includePath}`,
                );
                continue;
            }

            if (!fs.existsSync(resolvedPath)) {
                errors.push(`${relativeFile} includes a missing file: ${includePath}`);
                continue;
            }

            const targetLayer = classifyFile(resolvedPath);
            if (!targetLayer) {
                errors.push(
                    `${relativeFile} includes a file outside the defined layers: ${includePath}`,
                );
                continue;
            }

            if (!isDependencyAllowed(sourceLayer, targetLayer)) {
                errors.push(
                    `${relativeFile} (${sourceLayer}) cannot depend on ${includePath} (${targetLayer}).`,
                );
            }
        }
    }

    return { checkedFiles: pawnFiles.length, errors };
}

if (require.main === module) {
    const result = checkPawnArchitecture();
    if (result.errors.length > 0) {
        for (const error of result.errors) {
            console.error(`Architecture violation: ${error}`);
        }
        process.exitCode = 1;
    } else {
        console.log(
            `Pawn architecture valid: ${result.checkedFiles} source files checked.`,
        );
    }
}

module.exports = {
    checkPawnArchitecture,
    isDependencyAllowed,
    isExternalDependencyAllowed,
};
