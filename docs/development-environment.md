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
| MySQL Server | 8.4.9 | `MySQL84` service; `SELECT 1` passed over loopback |

The open.mp package includes the Pawn compiler and its compatible includes.
The server startup test used a temporary GameMode, disabled announcement and
RCON, and bound UDP to `127.0.0.1`; it was stopped after verification.

MSYS2's packaged CMake 4.5.0 crashed during project configuration on this
workstation (`0xC000001D`). Use the separately verified Kitware CMake 4.4.4
portable release for CMake builds instead.

MySQL Server 8.4.9 is installed under `%ProgramFiles%\MySQL\MySQL Server 8.4`
and configured as the Windows service `MySQL84`, which starts automatically.
The classic protocol (3306) and X Protocol (33060) listeners are both bound to
`127.0.0.1`; no enabled inbound Windows Firewall allow rules for these ports
were found. A local `SELECT 1` connection test succeeded. The root password
was entered locally and is not stored in this repository. For a later
connection check, enter the password only at the local prompt:

```powershell
& "$env:ProgramFiles\MySQL\MySQL Server 8.4\bin\mysql.exe" `
  --protocol=TCP --host=127.0.0.1 --port=3306 --user=root --password `
  --execute="SELECT 1;"
```

Phase 1 is complete. The environment checks, including the MySQL connection
test and repository validation, passed; the phase checklist and evidence are
recorded in Issue #5. No gameplay or application source was changed for these
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

The repository now has a minimal Pawn GameMode entry point at
`gamemodes/roleplay.pwn` and an architecture bootstrap. The Pawn compiler was
found in the local server installation at
`%USERPROFILE%\Downloads\Server\qawno\pawncc.exe` (it is not on `PATH`). The
bootstrap compiled with warnings treated as errors using that installation's
matching includes. A Pawn build task is not added because this workstation's
installation path is local-specific. No CMake plugin project exists yet.
See [Core Architecture](architecture.md) for the layer rules and module layout.
Phase 2 is complete; its review and verification evidence are tracked in
[Issue #9](https://github.com/dissaramdhani/openmp-roleplay/issues/9) and
[PR #8](https://github.com/dissaramdhani/openmp-roleplay/pull/8).

To repeat the verified Pawn build on this workstation:

```powershell
$serverRoot = Join-Path $env:USERPROFILE 'Downloads\Server'
$buildRoot = Join-Path $PWD 'build'
New-Item -ItemType Directory -Path $buildRoot -Force | Out-Null
& "$serverRoot\qawno\pawncc.exe" 'gamemodes\roleplay.pwn' `
  "-i$serverRoot\qawno\include" `
  "-i$PWD\gamemodes" `
  "-o$buildRoot\roleplay.amx" -E
```

The architecture dependency check and its automated tests run with Node.js:

```powershell
node scripts/check-pawn-architecture.js
node --test tests/architecture.test.js
```

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
