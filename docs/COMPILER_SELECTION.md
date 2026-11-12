![Umicom](assets/umicom-logo.svg)

# Understand Studio compiler selection

Studio's compiler centre is an application adapter over Umicom Framework. It holds view state and product defaults while Framework owns provider records, profiles, plan arguments and the compilation database. This service is distinct from evidence that the graphical Build panel or an installed compiler works.

## Follow a changed toolchain

1. Select the intended profile through the Studio toolchain centre.
2. Request compiler-centre synchronization.
3. Check its return status before showing the selection as successfully synchronized.
4. Read a checked compiler-centre snapshot and inspect its active profile identifier.
5. Create a new unit plan when you need a command reflecting the new selection.

The adapter now uses Framework's coherent provider/profile publication. Selecting another GCC path replaces the stored GCC provider and the named **toolchain-selected** profile. Selecting Clang changes that named profile to Clang while retaining GCC for other profiles. A failure keeps the prior compiler-centre configuration. It does not claim that every upstream selection or discovery operation is transactional.

The **toolchain_synchronised** flag records that synchronization succeeded at least once. It is not a background monitor: selecting another toolchain later still requires another synchronization call. A **FOUND** or **VALIDATED** tool state is supplied metadata, not a new executable probe. Failed tools are refused.

## Keep the project directory explicit

A product client can call:

~~~c
UmiStatus status = umi_studio_compiler_centre_plan_unit_in_directory(
    compiler_centre, "project", "main", project_root, plan);
~~~

Include **umicom/studio/compiler_centre.h**. The centre must own an opened manifest and an enabled active compiler profile; **plan** must point to separate, sufficiently sized caller storage. Both the returned plan and its compilation-database entry record **project_root**. No directory change, file read, compiler launch or build happens here.

The older **umi_studio_compiler_centre_plan_unit** keeps its relative "." convention for compatibility. Prefer the explicit-directory form when the workspace already knows its root. A relative directory remains relative metadata; supplying it does not resolve it against an installed application location.

Default registration also stages all supplied providers and the **host-debug** profile together. Empty executable arguments omit that provider update; they do not remove an existing provider. The first supplied C-capable provider is preferred, with an existing available C provider as fallback. An existing active profile stays active unless there was no selection.

## Read snapshots carefully

A malformed Framework owner makes the checked Studio snapshot fail without replacing the caller's previous snapshot. Overlapping output that would overwrite either owner is refused. The engine pointer stays stable across configuration changes, but its borrowed rows may be replaced; serialize access and refresh row pointers after a mutation.

The historical aggregate revision saturates at its maximum instead of wrapping. It is an observation hint, not a unique cache key. The nested Framework revision remains available. Do not treat a successful snapshot, a revision value, or a compiler preview as a successful build.

The authored **studio.compiler_centre** and **studio.compiler_synchronization** regressions exercise repeated selection, stable ownership, explicit directories, snapshot failure and saturation. Compiler, test, GUI and installed-consumer qualification remain to be executed. No new graphical layout or automatic execution control is introduced by these service changes.
