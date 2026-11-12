![Umicom](assets/umicom-logo.svg)

# Polyglot workspace metadata and compiler previews

A polyglot workspace can describe source files in several languages. Studio stores the current selection and uses Framework for manifest admission, language projection and compiler planning. These values do not by themselves prove that the project can compile or link across languages.

## Initialize and select safely

Allocate **UmiStudioPolyglotWorkspace** explicitly: its manifest and graphs contain large inline collections. Call **umi_studio_polyglot_workspace_init** with a nonempty bounded workspace identifier and a complete manifest. Valid empty drafts remain supported.

Initialization prepares a new value before replacing the destination. Its inputs may borrow the destination's current identifier and manifest. Malformed manifest storage, an overlong identifier or allocation failure preserves the complete previous workspace. The resulting language mask comes from **umi_compiler_manifest_languages**.

Call **umi_studio_polyglot_workspace_select_unit** to select an existing unit. The operation checks the projection fields it consumes, refuses revision exhaustion and leaves the workspace unchanged on failure. Every successful selection advances once, including selection of the current unit. The count accessor returns zero when the projection is malformed; zero alone therefore cannot distinguish an empty draft from invalid state.

Serialize readers and writers. These operations do not supply locks, run interop/build graphs or replace the shared services' validation contracts.

## Inspect the native preview

After building the Studio compiler tools in your own qualified environment:

~~~powershell
.\umicom-studio-polyglot-compiler.exe
.\umicom-studio-polyglot-compiler.exe --directory "C:/projects/reviewed application"
~~~

The first form preserves the existing relative-directory example. The second records an explicit intended directory in its plan and compilation-database JSON. It does not change the process directory, create project files or run GCC. Driver names and the example source **src/main.c** are metadata.

The command owns its large manifest, plan and JSON buffer on the heap and releases them together with its centre on success or failure. Existing setup/planning stages retain exit codes 1–8; allocation failure returns 9 and standard-output failure returns 10. Invalid command arguments return 2 with usage text. If writing stdout fails, a partial document may already have reached the receiver: check the exit status before accepting the JSON.

## Qualification and troubleshooting

The authored regressions are:

- **studio.umicc_polyglot**: setup checks remain active with **NDEBUG** and large values are explicitly owned.
- **studio.polyglot_workspace_publication**: empty drafts, multiple languages, borrowed inputs, malformed records and saturated selection revisions.
- **studio.polyglot_command_lifetime**: allocation and setup/output failures release the command's acquired owners.
- **studio.polyglot_command_json**: the executable emits complete JSON with the requested directory and refuses unknown arguments before preview.

These checks are unexecuted until owner qualification. The failure-injection harness observes only the command's own allocations and centre lifecycle; it is not a universal memory-leak detector. A successful metadata preview is separate from actual compiler execution, project builds, UI selection, toolchain discovery and runtime loading.

For a missing API at compile or link time, first compare the installed/linked Framework identity with the source headers. For a refused workspace, correct its identifier, manifest or selection state; do not truncate identifiers or reset a live revision counter to hide the refusal.

See [compiler selection](COMPILER_SELECTION.md) and the shared [manifest language projection](../../../framework/docs/development/compiler-manifest-languages.md).

For a guided explanation of preparation and publication, follow the [toolchain publication lesson](../../../../Umicom-Bits2Banking/content/courses/learning-build-systems/lessons/20-publish-toolchain-selection.html) and [borrowed-pointer ownership lesson](../../../../Umicom-Bits2Banking/content/courses/learning-build-systems/lessons/21-environment-pointer-ownership.html) in the ecosystem learning checkout.
