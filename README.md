# Umicom Studio IDE application

Umicom Studio IDE 0.23.0 is an independent GTK4 and console application built on
Umicom Framework 0.9.0.

## Source ownership

- `src/app` contains the Studio composition root and thin product adapters over
  reusable Framework services.
- `src/console` contains the headless frontend.
- `src/tools` contains native Doctor, diagnostics, settings, and platform tools.
- `src/gtk` contains the GTK4 executable entry point.
- Product-specific editor, workbench, pane, theme, AI, and IDE workflows remain
  inside Studio.
- Reusable filesystem, process, toolchain, repository, scaffolding, capability,
  policy, and suite mechanisms are owned by Framework.

## Platform integration

`UmiStudioPlatformReport` exposes Framework discovery and capability information
to Studio without duplicating the implementation.  The GTK4 environment page
and future repository wizard must use the same Framework services as the native
`umicom` command.

## Application composition

Studio remains independently runnable while sharing the same Framework contracts
that can compose Studio, Designer, Trader, TMS, and future applications into a
suite.

Studio also starts Framework-owned learning, standard or focus presentation recipes as a live
application surface. It contributes IDE guidance and commands for ten reusable
panels while Framework owns their lifecycle, focus, state and portable layout.
See [the Studio application surface guide](docs/APPLICATION_SURFACE_GUIDE.md).
The [Studio runtime behavior guide](docs/RUNTIME_BEHAVIOR_ADOPTION.md) explains
how Studio uses shared refresh, background, context and checkpoint rules and
how a developer should connect them to a graphical event loop.
[Professional startup and workspace design](docs/PROFESSIONAL_STARTUP_AND_WORKSPACE_DESIGN.md)
explains the visible workbench hierarchy, layout lock and shared panel rules.
The [community extension development guide](docs/COMMUNITY_EXTENSION_DEVELOPMENT.md)
documents the Framework-owned extension starter, lifecycle, contribution,
permission, compatibility, package and testing model. The
[C development and learning workspace](docs/C_DEVELOPMENT_AND_LEARNING_WORKSPACE.md)
includes offline completion, quick documentation and the built-in safer C
reference.

Studio's public headers follow the same SDK contract audit as Framework. The
[catalogue include-order repair](docs/EXPERIENCE_CATALOGUE_INCLUDE_ORDER.md)
explains why two different catalogue families must have distinct guards.

The Windows suite installer exposes Studio as the optional **Studio**
component. Umicom Desk registers the real `umicom-studio-ide` executable and
can launch Studio together with other installed Umicom applications.

## Learn the everyday editor workflow

[Editing and saving files](docs/EDITING_FILES.html) follows an Umicom Notes
practice project through tabs, unsaved text, next/previous search, replacement,
Undo, Save All, builds and close confirmation. It explains the current editor
size limit and how to respond to a file conflict.

## Order your workspace layouts

Studio's shared Layout Library includes **Move up** and **Move down** controls.
[Arrange your workspace layouts](docs/ORDERING_WORKSPACE_LAYOUTS.md) explains
how to reorder the list, retain the active workspace and save the order.

## Copy reports for review

Export one selected test's retained results and output with [Copy selected-test evidence](docs/COPYING_TEST_EVIDENCE.md).

## Follow test failures into source

[Open a file mentioned by a test failure](docs/OPENING_TEST_FAILURES.md) explains source locations, retained runs and safe navigation.

Source breakpoint rows support explicit condition, log-message and enabled-state editing. Follow [Edit source breakpoints](docs/EDITING_BREAKPOINTS.md) for the workflow and adapter-confirmation limits.

Learn to [edit watch expressions and explicitly evaluate them](docs/EDITING_WATCHES.md) in the Debug workspace.

### Inspect local variables

The native Variables panel can read one level of an object at a time, refresh
its captured children and collapse a branch. Follow the beginner guide to
[inspect variables and their children](docs/INSPECTING_VARIABLES.md).

Use [Inspect a debugger scope](docs/INSPECTING_SCOPES.md) to open Locals,
Arguments, Globals or other adapter-provided groups, including expensive scopes
that remain unloaded until you choose **Inspect scope**.

## Review linked context values

Studio IDE exposes Framework's reviewed context changes through
`umi_studio_workspace_context_review`, `umi_studio_workspace_context_apply`
and `umi_studio_workspace_clear_context` in
`umicom/studio/workspace_commands.h`. A context group is a named value that
related panels can share; for example, a host could use `workspace.project` with the sample
value `sample-project`. The host must connect that name to its panel consumers.

1. Use a runtime initialised with this product's canonical experience. Prepare
   the requested changes with the review function; preparation changes no live state.
2. Display the copied Framework summary and rows. Keep the runtime and its
   workbench alive while the user reviews the proposed values. If the summary
   reports UI differences, show the captured UI value beside the cached value.
3. Apply only after acceptance, then destroy the review. If the workspace changed,
   prepare a fresh review. Cancelling only destroys the review.

This is a module API; native review screens are separate host work. Context edits
do not execute product commands or external operations. The shared guide at
`framework/docs/guides/REVIEWING_LINKED_CONTEXTS.html` in the Applications checkout
explains capacity, ownership, thread coordination and recovery in more detail.

[Run and debug with program arguments](docs/RUNNING_WITH_ARGUMENTS.html) explains the native workflow, retained settings and current limits.

Workspace recovery: [Review saved layouts while keeping open source drafts](../../framework/docs/learning/restore-saved-workspaces.html). The guide explains the complete comparison, explicit confirmation and recovery limits.

For an explicit runtime edit while debugging, see [assigning captured variables](docs/ASSIGNING_DEBUGGER_VALUES.html). The guide explains confirmation, adapter support and recovery after an uncertain reply.

[Follow and copy live build output](docs/LIVE_BUILD_OUTPUT.html) explains progress, paused inspection, retained history and output limits.

[Save build output to a new log file](docs/SAVING_BUILD_LOGS.html) explains file selection, capture status, retention and recovery.

[Edit provider connection settings](../../framework/docs/learning/editing-provider-connections.html)
explains the Connections entry in AI Coding, local persistence and explicit conflict review.
This settings editor does not change the active AI provider or authenticate with a service.

Within Connections, **Manage local keys…** opens Framework's profile-verified
credential panel for the same Studio settings scope.
[Manage local provider keys](../../framework/docs/learning/managing-local-provider-keys.html)
explains creating the local profile, saving or replacing a key, and checking or
removing it. This does not switch the active AI provider or sign in remotely.

[Check a saved provider connection](../../framework/docs/learning/checking-provider-connections.html)
explains the separate **Check saved connection** action in Connections. After
review and approval, it requests a model catalogue from the official OpenAI
endpoint or a supported local loopback server. Remote checks freshly verify
the local profile password before reading the stored key. A listed model does
not prove that chat or inference works, and this action does not change the
active AI provider. Editing and saving metadata still performs no sign-in.

[Chat with a saved connection](../../framework/docs/learning/chat-with-saved-connections.html)
explains **Connections → Chat with selected saved connection…**. Enter a prompt
and optional context, review the exact outgoing message, then approve one send.
The shared Framework workflow uses the selected OpenAI or local connection and
displays plain text. It does not change the provider used by the existing chat,
agent or patch controls, and it cannot run tools or apply code changes.

In AI Coding, **Chat about selected code…** copies the active editor selection into connection choice and request review. Read the [selected-code chat guide](../../framework/docs/learning/chat-about-selected-code.html) for the steps, limits and local context ownership.

For a longer discussion, see [Choose context for a chat follow-up](../../framework/docs/learning/reviewed-chat-follow-ups.html). The shared window keeps up to eight exchanges locally in memory and copies only the excerpt you explicitly select into a newly reviewed request.

To turn a suggestion into a deliberate source edit, follow [Review a replacement for selected code](../../framework/docs/learning/review-selected-code-replacements.html). The shared review captures the target, previews complete drafts and applies approved text through the document Undo owner; saving remains separate.

[Choose project presets and a program working folder](../../framework/docs/learning/project-stage-presets.html) explains separate configure, build and test selections, saved launch folders shared by Run and native Debug, and recovery from conflicting settings.

Project Settings can read the two standard preset files and offer their directly declared names in a searchable list. Choose a row, copy its name to the matching stage, then apply settings separately. Included files are flagged as unread; inherited values and conditions remain CMake’s responsibility. The guide explains limits and recovery from a failed read.

[Choose and review a program launch](../../framework/docs/learning/review-launch-settings.html) explains selecting a local executable and working folder, reviewing exact argument values, preserving manual edits during a chooser, and making Run and native Debug executable lookup explicit. Review does not start a process or save settings.

[Discover configured build targets](../../framework/docs/learning/discover-configured-targets.html) explains requesting CMake target metadata, reading the configured target list, selecting a build target or executable, and recovering from stale or mismatched build folders. Selection edits the form; applying settings and execution remain separate.

Native debugger choices can be saved locally and loaded for review. See [Choose and save a native debugger](docs/learning/native-debugger-preferences.html) for the supported workflow and recovery steps.

For an explicit native server handshake check, see [Check a language server in Studio](docs/native-language-connection.html).


### Saved debugger setups

Run and Debug now includes a saved-setup panel for breakpoint locations,
conditions, log messages and watch expressions. Save to a new private JSON
file, load it for a complete current/proposed comparison, and explicitly apply
the reviewed replacement while debugging is stopped. Loading starts no process
and evaluates no expression. The same panel can review the settings replaced
by its last successful application.

The format supports up to 64 breakpoints and 64 watches; larger collections are
refused without truncation. File paths are entered explicitly, and saved source
locations are not remapped when a project moves. Studio accepts one breakpoint
per source line through its existing compatibility projection. See Framework's
`docs/learning/saved-debug-setups.html` for the step-by-step workflow, recovery
guidance and reusable API ownership rules.

Paused native variables with adapter memory references can be inspected as bounded hex/ASCII captures. See [Inspect memory in Studio](docs/READING_DEBUGGER_MEMORY.html) for scope capture, offsets, short reads and recovery.

Installed programs can be reviewed from build settings after a successful full CMake install. Choose **Read installed files**, inspect a program candidate, then **Use installed program**. Apply Settings and Run remain separate actions. Framework's `docs/learning/review-installed-files.html` explains folder selection, stale manifests, supported paths and recovery.

### Find configured targets

Build Settings now offers an explicit text filter and original/A-Z/Z-A ordering
for captured CMake targets. Select a visible row to inspect its output before
copying its build target or program. A hidden selection is cleared; changes to
build inputs still require a fresh read. See Framework's installed learning
guide, **Find a configured target or captured position**, for the workflow and
text-matching limits.

Installed-file review also supports literal path/status filtering and original,
ascending or descending text order. The chosen file retains its source identity
when rows move; hiding it while validation runs refuses the late selection.
See Framework's installed-file review guide for the complete install-to-launch
workflow. Applying a filter neither runs a program nor accepts build settings.

For persistent build outcomes, see [Reviewing local build-job history](docs/LOCAL_JOB_HISTORY.html). Select a private SQLite file in Output; reopening it never restarts work.

## Private test history

Test Explorer can retain completed queued runs in an explicitly selected local SQLite database. Read [Private test history](docs/PRIVATE_TEST_HISTORY.html) for saving, reopening and inspecting evidence without changing current test results.
