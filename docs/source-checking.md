![Umicom](../../../framework/resources/brand/umicom-logo.svg)

# Check Umicom C source before a native build

The optional `umicom-studio-source-check` command checks the supported Umicom C subset. It compiles and validates an in-memory snapshot without running the program, saving it, invoking a shell or starting the Studio desktop. It is not a full C23 checker. Keep the normal compiler, linker and application tests in your development workflow.

## Build the optional command

Use a current Framework checkout and a compatible Umicom C SDK. The following source-composition example uses sibling repositories in a workspace; adjust the absolute compiler path to your checkout. Run each step separately and stop if it fails.

1. In the Applications repository, select the usual supported preset and add `-DUMICOM_STUDIO_CVM_CHECKS=ON`.
2. To select compiler source explicitly, also pass `-DUMICOM_C_PREVIEW_SOURCE=C:/your-workspace/umicom-c`. Otherwise the adapter looks for the installed Umicom C package.
3. Build target `umicom-studio-source-check`. The normal Applications runtime output directory controls where its executable is written.
4. With the compiler runtime directory on PATH, invoke the resulting executable with `--help`.
5. Pass one absolute path to an ordinary local source file. The reader refuses oversized files before allocating their content and rejects a final symlink/device. Trusted parent directories remain the caller's responsibility.

For isolated contributor checks, configure `applications/studio/tests/source_check` as its own CMake source directory with the same explicit compiler path. Build that directory and run CTest there. This composition also runs the actual Framework document-store and compiler tests. It does not start Studio or establish installed-SDK compatibility.

## Read the result

For `int main(void){int unused=7;return 42;}`, expect an unused-local warning and the phrase `instructions checked, not executed`. The command returns zero because warnings do not reject this source. It never prints the return value as a program result.

A compiler refusal returns 2 and a diagnostic location. Input, allocation and output failures return 1; incorrect command usage returns 64. Always inspect the process exit code. A partial output stream is not a complete diagnostic report. File limits and unsupported language constructs are refusals to investigate, not permission to truncate source silently.

## Use the controller with an editor

Create `UmiStudioSourceCheck` using the frontend's live `UmiDocumentStore`. Keep the store alive until after destroying the controller. Serialize controller calls on the frontend's owning thread.

Call `umi_studio_source_check_run` with the selected document ID. Framework captures text and revision under one mutex, enforces the smaller of the caller's bound and the compiler source limit, checks that copy, and releases it. Studio retains one result. It does not keep another permanent source copy.

Call `umi_studio_source_check_current` before displaying results. A valid result can contain a compiler error; inspect its `check_status` and `source.completed` fields. After an edit, close or path change, the old result is unavailable. A failed new request clears previous findings. Rechecking identical bytes after an edit still requires a new revision.

Use `umi_studio_source_check_warning` or `umi_studio_source_check_error` to obtain a shared `UmiDiagnosticSnapshot`. The record contains document correlation ID, revision, path, code and byte-based source location. Untitled documents have no path. Do not interpret a byte column as a GTK character index or an LSP UTF-16 column. Perform the appropriate conversion against the same current source before navigation.

Freshness is an observation, not a lock on future edits. Recheck immediately before navigation and serialize the UI action with editing. Results from a destroyed store must be discarded even if a later allocation reuses its address.

## Current boundary

The native controller and command are available through the opt-in composition. GTK commands, automatic background scheduling, editor markers and keyboard shortcuts are not connected to this controller yet. The compiler's unused-local analysis is lexical: a read in any source branch counts as a read. It does not prove arithmetic safety, data-flow correctness, memory ownership or complete C23 support. Large builds and application qualification remain necessary.
