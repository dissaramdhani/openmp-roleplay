# Runtime Configuration

Phase 3 provides versioned JSON configuration for the Pawn GameMode. The C++
Infrastructure plugin loads and validates the selected environment before
configuration values become available through its Pawn natives. It is not a
replacement for open.mp's own `config.json`.

## Select an environment

Set `OPENMP_ENV` explicitly to `development` or `production`. The plugin does
not silently choose an environment. By default, it reads `config/<environment>`
relative to the server's working directory. Set `OPENMP_CONFIG_ROOT` to an
absolute or relative directory containing the `development` and `production`
folders when the server runs elsewhere.

Example PowerShell setup for a local development server:

```powershell
$env:OPENMP_ENV = 'development'
$env:OPENMP_CONFIG_ROOT = Join-Path $PWD 'config'
```

Copy the build output `roleplay_config.dll` into the open.mp `plugins` directory
and add `roleplay_config` to `pawn.legacy_plugins` in open.mp's `config.json`.
The DLL must match the server architecture; the verified Windows server is
32-bit x86.

Missing files, invalid JSON, an unsupported schema version, missing required
fields, unknown keys, invalid types, and out-of-range values are logged with
the affected category/key. The plugin publishes no configuration snapshot on
invalid startup, and its typed getters fail closed. The GameMode then requests
an orderly open.mp server shutdown before application startup; it cannot start
with a partial snapshot.

## Files and schema

Each category file declares `"schema_version": 1`. All category files are
required in both environments. Optional fields receive the safe defaults listed
below; unknown keys are rejected.

| Category | Key | Type | Required / default | Constraints |
|---|---|---|---|---|
| `server` | `name` | string | Required | 1–80 characters |
| `server` | `language` | string | `id` | 1–16 characters |
| `database` | `host` | string | Required | 1–253 characters |
| `database` | `port` | integer | Required | 1–65535 |
| `database` | `database` | string | Required | 1–64 characters |
| `database` | `username` | string | Required | 1–64 characters |
| `database` | `password_env` | string | Required | 1–64 uppercase letters/digits/underscores; starts with a letter or underscore |
| `database` | `pool_size` | integer | `5` | 1–50 |
| `gameplay` | `starting_cash` | integer | `500` | 0–2,000,000,000 |
| `gameplay` | `max_characters` | integer | `3` | 1–10 |
| `economy` | `currency_symbol` | string | `$` | 1–4 characters |
| `economy` | `maximum_cash` | integer | `2,000,000,000` | 1–2,000,000,000 |
| `vehicle` | `max_personal_vehicles` | integer | `5` | 0–100 |
| `vehicle` | `max_vehicle_speed` | number | `200.0` | 1.0–400.0 |
| `jobs` | `max_active_jobs` | integer | `1` | 1–10 |
| `jobs` | `payout_multiplier` | number | `1.0` | 0.1–10.0 |
| `security` | `max_login_attempts` | integer | `5` | 1–20 |
| `security` | `login_lockout_seconds` | integer | `900` | 0–86400 |
| `authentication` | `bcrypt_cost` | integer | `12` | 4–16 |
| `authentication` | `password_min_length` | integer | `8` | 8–128 |
| `logging` | `level` | string | `info` | `debug`, `info`, `warn`, or `error` |
| `logging` | `log_chat` | boolean | `false` | — |
| `performance` | `timer_interval_ms` | integer | `1000` | 50–60000 |
| `performance` | `worker_threads` | integer | `2` | 1–32 |

The committed development and production files are safe examples, not
credentials. `database.password_env` stores the name of an environment
variable; the variable's value is never stored in JSON or logged. Database
connection behavior is not implemented by this phase.

## Pawn API

Application code uses the contract in
`gamemodes/roleplay/application/contracts/configuration.inc`. It exposes
`Roleplay_Config_GetInt`, `Roleplay_Config_GetFloat`, `Roleplay_Config_GetBool`,
`Roleplay_Config_GetString`, and `Roleplay_Config_Reload`. A getter returns
false when the key is missing, has a different type, or a string output buffer
is too small or startup configuration was rejected. String buffer length
includes the terminating null cell.

Reload validates every file into a candidate immutable snapshot. The active
snapshot is atomically replaced only after all files pass validation. A failed
reload logs the reason, returns false, and leaves the previous snapshot active.
Do not expose a reload call to clients; any future command must enforce
server-side authorization.

The example files include settings for future roleplay systems. At this phase,
only `server.name` is consumed during GameMode startup; the other categories
are available to later modules and do not imply those gameplay systems already
exist.

## Build and tests

```powershell
cmake -S native -B build\native -A Win32
cmake --build build\native --config Release
ctest --test-dir build\native -C Release --output-on-failure
```

The native project pins `guil2k7/samp-plugin-sdk` at
`c4a32be5a4155924d0b809c78466490869d9349a` and `nlohmann/json` v3.12.0 at
`55f93686c01528224f448c19128836e7df245f72`. Both dependencies are MIT-licensed.
The open.mp server source confirms support for both its component entry point
and legacy `Supports`/`Load` AMX plugins; this plugin uses the latter ABI to expose
fail-closed natives even when configuration validation fails, allowing the
GameMode to request an orderly server shutdown before application startup.
