# Development Environment

## Editor

Use Visual Studio Code with the project recommendations in
`.vscode/extensions.json`. Workspace settings and tasks are defined in
`.vscode/settings.json` and `.vscode/tasks.json`.

The Pawn extension provides language support, not the Pawn compiler or open.mp
server. Install and verify each toolchain separately; do not assume that an
editor extension installs its compiler or runtime.

## Required toolchains

Install the tools needed for the phase you are working on:

- open.mp server and Pawn compiler for GameMode development.
- C/C++ compiler and build tools for native plugins.
- MySQL server/client for database integration.
- Node.js for backend and repository tooling.
- Git for version control.

Record supported versions and reproducible setup steps here as the toolchain is
selected and verified in Phase 1. Never commit credentials or production
secrets; use local environment configuration and document safe examples.

## Repository checks

Run the workspace JSON validator from the repository root:

```sh
node scripts/validate-workspace-config.js
```

GitHub Actions runs this check on pushes and pull requests. Compiler, runtime,
database, and application tests will be added to CI when their toolchains and
project build/test commands are established.
