![Umicom](assets/umicom-logo.svg)

# Keep Studio's selected toolchain coherent

A failed toolchain change should leave Studio describing the previous usable selection. The toolchain centre now asks Framework to prepare the child environment before publishing a manual profile selection.

1. Register or discover the intended profile in the shared catalogue.
2. Call **umi_studio_toolchain_centre_select_profile** with its identifier.
3. Check the returned status before updating a selected-profile label.
4. Borrow the selected profile and environment only while the centre remains alive and stable.
5. Synchronize the separate compiler centre when a new compiler argument plan is needed.

For manual selection, failure preserves the catalogue's explicit selection, the Studio profile, its environment and its presentation flags. The same guarantee applies when environment construction fails after profile lookup. The service does not briefly publish the new profile and then attempt to undo it.

Project selection and discovery also prepare an environment before replacing Studio's profile/environment pair. Discovery can already have changed catalogue entries or produced probe evidence before a later adapter failure. Those earlier discovery effects are not rolled back. A project-specific profile can intentionally differ from the catalogue's global preferred profile.

The environment owns its strings. Borrowed process-entry pointers are rebound to that receiving owner after a successful change. Do not retain individual values across later selection calls or destroy the centre while a consumer is using them. All access must be serialized by the application.

An invalid environment value, unavailable selected compiler, incompatible profile record, allocation failure or exhausted selection revision can refuse an operation. Surface that status and correct the underlying metadata or lifecycle issue. Do not silently fall back to a different compiler while presenting the requested selection.

The authored **studio.toolchain_selection_publication** regression checks a newline-containing compiler path: the new selection is refused and the previous profile, environment, catalogue selection and borrowed pointer ownership remain intact. No compiler or GUI execution is claimed. See [compiler selection](COMPILER_SELECTION.md) for the separate argument-planning adapter.
