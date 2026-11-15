# Development Environment

## Editor

Use Visual Studio Code with the project recommendations in
`.vscode/extensions.json`. Workspace settings and tasks are defined in
`.vscode/settings.json` and `.vscode/tasks.json`.

The Pawn extension provides language support, not the Pawn compiler or open.mp
server. Install and verify each toolchain separately; do not assume that an
editor extension installs its compiler or runtime.

## Verified workstation

The following versions were verified on the current Windows workstation:

| Component | Version | Verification |
|---|---:|---|
| open.mp server | 1.6.7.3218 | Started and stopped on loopback only |
| Pawn compiler | 3.10.11 | Compiled a smoke test with `<open.mp>`; warnings treated as errors |
| GCC/G++ (MSYS2 UCRT64) | 16.2.0 | Built and ran a C++ executable; built a native DLL |
| CMake (Kitware portable) | 4.4.4 | Configured and built the C++ smoke project with Ninja |
| Ninja | 1.13.2 | Used by the CMake smoke build |
| Node.js / npm | 26.13.0 / 11.20.0 | JavaScript runtime smoke test passed |
| Git | 2.56.0.windows.4 | Version check passed |
| MySQL Server | 8.4.9 | Binaries installed; server configuration and connection test are pending |

The open.mp package includes the Pawn compiler and its compatible includes.
The server startup test used a temporary GameMode, disabled announcement and
RCON, and bound UDP to `127.0.0.1`; it was stopped after verification.

MSYS2's packaged CMake 4.5.0 crashed during project configuration on this
workstation (`0xC000001D`). Use the separately verified Kitware CMake 4.4.4
portable release for CMake builds instead.

MySQL Server is installed under `%ProgramFiles%\MySQL\MySQL Server 8.4`, but no
MySQL service is configured and port 3306 is not listening. The MySQL
Configurator requires administrator elevation. Run
`%ProgramFiles%\MySQL\MySQL Server 8.4\bin\mysql_configurator.exe` as
administrator, configure a local development instance bound to loopback, and
set a strong private password directly in the configurator. Do not send the
password in chat or commit it to the repository. After setup, verify the local
connection with:

```powershell
& "$env:ProgramFiles\MySQL\MySQL Server 8.4\bin\mysql.exe" `
  --host=127.0.0.1 --user=root --password --execute="SELECT 1;"
```

Phase 1 remains in progress until the MySQL connection test and the repository
checks have passed. No gameplay or application source was changed for these
toolchain smoke tests.

## Required toolchains

Install the tools needed for the phase you are working on:

- open.mp server and Pawn compiler for GameMode development.
- C/C++ compiler and build tools for native plugins.
- MySQL server/client for database integration.
- Node.js for backend and repository tooling.
- Git for version control.

On Windows, the verified C/C++ setup uses MSYS2 UCRT64 with GCC and Ninja, plus
the portable CMake release from Kitware. Install MSYS2 in a user-writable,
short ASCII path, then install the UCRT64 compiler and Ninja packages from
MSYS2's package manager. Use the open.mp release's bundled `qawno\pawncc.exe`
and `qawno\include` directory for Pawn compilation.

The repository currently has no GameMode source or CMake project to build.
Keep the existing JavaScript and workspace-validation tasks; add Pawn and
native build tasks when the project source layout and build targets are defined
rather than hard-coding this workstation's tool paths.

Never commit credentials or production secrets; use local environment
configuration and document safe examples.

## Repository checks

Run the workspace JSON validator from the repository root:

```sh
node scripts/validate-workspace-config.js
```

GitHub Actions runs this check on pushes and pull requests. Compiler, runtime,
database, and application tests will be added to CI when their toolchains and
project build/test commands are established.
