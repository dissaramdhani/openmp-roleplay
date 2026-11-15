# Core Architecture

## Status and scope

Phase 2 is complete. The Pawn/open.mp architecture foundation was reviewed and
merged in PR #8; acceptance evidence is tracked in Issue #9. It does not add
gameplay, a database schema, credentials, a JavaScript backend, or a C++ plugin.
Concrete persistence and BCrypt adapters belong with the phases that introduce
those features.

## Pawn source layout

```text
gamemodes/
├── roleplay.pwn                         # open.mp entry point
└── roleplay/
    ├── modules.inc                      # composition root/static module registration
    ├── domain/                          # pure game-domain rules and types
    ├── application/
    │   ├── contracts/                   # ports consumed by use cases
    │   └── startup.inc                  # application startup use case
    ├── infrastructure/
    │   └── adapters/                    # implementations of application ports
    └── presentation/
        └── server_callbacks.inc         # open.mp lifecycle adapter
```

The Domain directory intentionally has no placeholder entity. Add domain types
when a real business concept is introduced.

The root `native/` project contains the C++ Configuration Infrastructure. It
uses the open.mp-compatible legacy AMX plugin ABI to validate versioned JSON
files before configuration values become available to Pawn. If validation
fails, its typed natives stay fail-closed so the GameMode can request an
orderly server shutdown before application startup. Pawn Application consumes
only the configuration contract; the native ABI and JSON parser remain in the
Infrastructure adapter.

## Dependency rule

The allowed source dependencies are:

```text
Presentation ──> Application ──> Domain
Infrastructure ──> Application contracts ──> Domain
Composition root ──> Presentation, Application, Infrastructure, Domain
```

- Domain has no dependency on another layer or on open.mp, plugins, databases,
  configuration, or I/O.
- Application may depend on Domain and its own contracts. It must not include
  Infrastructure or Presentation.
- Infrastructure implements ports declared by Application and may depend on
  Domain.
- Presentation adapts open.mp callbacks and player input to Application. It
  must not call Infrastructure directly.
- `gamemodes/roleplay/modules.inc` is the composition root. It explicitly
  includes/registers the modules in compile order; this is static registration,
  not runtime plugin discovery.
- The Application configuration contract exposes typed reads and reload. The
  Infrastructure adapter forwards those calls to the C++ plugin. Domain and
  Application do not include C++ or JSON implementation details.
- Application requests server shutdown through its server-lifecycle contract;
  the Infrastructure adapter translates that request to open.mp's console
  command rather than exposing the server native to the use case.
- The C++ plugin validates the complete environment snapshot before publishing
  it. Reload builds and validates a candidate first, then atomically swaps the
  active immutable snapshot; a failed reload leaves the previous snapshot
  active.
- Quoted Pawn include paths are rooted at the `gamemodes/` compiler include
  directory and start with `roleplay/`; the architecture check resolves them
  using the same rule.
- Domain and Application may not include external Pawn libraries. The
  infrastructure, presentation, and composition layers are the only places
  allowed to reference external includes.

## Pawn-specific contracts

Pawn does not provide conventional interfaces or dependency injection. Declare
an application port as a `forward` in `application/contracts/`, implement that
signature in an Infrastructure adapter, and assemble both in the composition
root. Application code calls only the declared port; it does not include or
name the concrete adapter. Keep include paths within their owning layer except
for the allowed inward dependencies above.

The initial `Roleplay_Log` port is deliberately small: the application startup
use case emits one message, and the Infrastructure console adapter implements
it with Pawn's `print`. The Presentation adapter exposes `OnGameModeInit` and
delegates startup to Application. This is wiring only, not a gameplay feature.

## Adding a module

1. Put domain rules and types in `domain/`; do not add server or persistence
   concerns there.
2. Put use cases and ports in `application/`; express external needs as
   contracts.
3. Implement those contracts under `infrastructure/adapters/`.
4. Adapt open.mp callbacks and input under `presentation/`.
5. Add module includes to `modules.inc`, which is the only composition root.
6. Run `node scripts/check-pawn-architecture.js` and
   `node --test tests/architecture.test.js`.
7. Compile with the open.mp-compatible Pawn compiler and its includes before
   treating the GameMode as build-verified. Build/test the C++ Configuration
   Infrastructure with CMake and CTest.

Database access, BCrypt, and client-controlled input must remain behind
application contracts and be validated server-side when those features are
introduced.

The configuration loader is not a gameplay system. Phase 3 makes the server
name available to startup and provides typed values for later modules; other
configuration categories do not become active gameplay behavior until those
modules consume them.
