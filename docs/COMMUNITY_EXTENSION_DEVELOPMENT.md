<!--
Umicom Studio IDE
File: docs/COMMUNITY_EXTENSION_DEVELOPMENT.md

PURPOSE:
Explain how community extensions integrate with the Framework and Studio
without placing reusable behaviour inside the application.

AUTHOR AND ORGANISATION:
Sammy Hegab
Umicom Foundation

LICENCE:
MIT
-->

# Community Extension Development

Umicom extensions add a focused capability without copying or modifying the
Studio application core. A project describes itself before its native library
is loaded, exports one stable query function, declares the Framework ABI it
understands, and participates in a supervised lifecycle.

Choose the **Umicom Extension** project starter to generate a C23 library,
manifest, public entry-point header, lifecycle implementation, descriptor test,
build presets, installation rules and package metadata. The starter is owned by
Umicom Framework, so other Framework-based development applications can offer
the same workflow.

## Extension package structure

The generated project contains:

- `extension.umi.in`, which declares identity, version, ABI, library and the
  first provided capability;
- `include/<extension>/extension.h`, which exports the stable module query;
- `src/extension.c`, which owns configure, initialise, start and stop work;
- `tests/test_descriptor.c`, which rejects incompatible ABI requests without
  opening a graphical application;
- build, install and package rules that keep the manifest and library together.

The manifest is inspected before native code is loaded. This allows the host to
reject an incompatible, disabled, untrusted or incorrectly packaged extension
without executing it.

## Lifecycle and ownership

The lifecycle is intentionally ordered:

1. **Query** reads the immutable descriptor and ABI.
2. **Configure** receives borrowed Framework services and configuration.
3. **Initialise** creates extension-owned state.
4. **Start** registers granted contributions and begins work.
5. **Stop** disconnects callbacks and active work before unload.
6. **Destroy** releases the extension's remaining owned state.

A callback must not keep a borrowed pointer beyond the lifetime promised by its
service. Stop must remove timers, subscriptions and background work before the
library is unloaded. Partial creation must be safe to destroy.

## Contribution points

An extension contributes through a named contract rather than reaching into a
window's private widgets. Current and planned contribution families include:

- commands, menus, key bindings and command-centre entries;
- movable panes, window catalogue entries and layout presets;
- project templates, language definitions and snippets;
- completion, hover, signature, diagnostic and code-action providers;
- debugger, compiler, build, test and source-control adapters;
- documentation, lesson and knowledge-source packs;
- visual designers, previews, charts and data providers;
- governed model tools and task agents.

Contribution identifiers must be globally unique. Registration should be
repeatable, and removing an extension must leave no stale commands, panes or
callbacks. Multiple providers may answer the same request; the Framework owns
selection, policy, ordering, cancellation and result aggregation.

## Permissions and trust

Extensions request only the capabilities they need. File access, processes,
network access, credentials, trading actions and model tools are separate
permissions. A visible panel does not imply permission to perform an operation.

The extension host owns publisher identity, signature evidence, checksum
validation, provenance, trust policy, resource budgets, isolation, heartbeat,
crash recording, quarantine and recovery. Studio presents those decisions but
does not bypass them.

## Compatibility and updates

The Framework ABI protects structure layout and calling conventions. Extension
versions describe package evolution; dependency constraints describe required
capabilities or other packages. An update catalogue can propose a newer
package, but installation remains a planned, validated transaction with a
rollback path.

Do not silently replace a loaded library. Stop its contributions, verify that
no borrowed callback remains active, install the accepted package, activate it
through the same lifecycle, and restore the previous package if activation
fails.

## Testing an extension

Every extension should test:

- descriptor identity and rejection of an unsupported ABI;
- each lifecycle transition, including partial failure and repeated cleanup;
- manifest and package agreement;
- missing permissions and untrusted-workspace behaviour;
- duplicate contribution identifiers and dependency cycles;
- cancellation, capacity limits and provider timeouts;
- enable, disable, update, rollback, crash and recovery paths;
- headless behaviour before graphical interaction tests.

The generated descriptor test is the first gate, not the complete acceptance
suite. A feature is complete only when its command or provider performs real
work, its view reports honest state, and its cleanup leaves the host reusable.

## Remaining product work

The Framework already has manifest, ABI, lifecycle, contribution, dependency,
permission, package, update, isolation, trust and recovery contracts. The new
project starter makes those contracts approachable to community developers.
Further batches should add a graphical project wizard, signed package
publishing and review, catalogue-source management, live contribution
inspection, and full unload testing for every contribution family.
