"use strict";

const js = require("@eslint/js");

module.exports = [
    js.configs.recommended,
    {
        files: ["**/*.cjs", "scripts/**/*.js", "tests/**/*.js"],
        languageOptions: {
            ecmaVersion: "latest",
            sourceType: "commonjs",
            globals: {
                __dirname: "readonly",
                console: "readonly",
                module: "readonly",
                process: "readonly",
                require: "readonly",
            },
        },
        rules: {
            "no-console": "off",
        },
    },
];
