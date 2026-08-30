<!--
Umicom Studio IDE
File: docs/RUNTIME_BEHAVIOR_ADOPTION.md

PURPOSE:
Explain how Studio adopts Framework-owned panel and workspace runtime behavior.

AUTHOR AND ORGANISATION:
Sammy Hegab
Umicom Foundation

LICENCE:
MIT
-->

# Studio Runtime Behavior

Studio uses the Framework standard recipe instead of inventing a second panel
lifecycle. The product supplies helpful IDE messages and commands, while the
Framework decides when reusable panels refresh, what context they understand
and when the workspace should be saved.

## What Studio now exposes

`application_surface_policy.h` provides a small Studio-friendly API:

- read a plain-language policy snapshot;
- advance scheduled panel work;
- tell the runtime when Studio enters or leaves the background;
- pass file or source context to compatible panels;
- ask whether a workspace checkpoint is due.

The Studio UI forwards the same operations through `umi_studio_ui_advance()`,
`umi_studio_ui_set_background()`, `umi_studio_ui_context_changed()` and
`umi_studio_ui_checkpoint_due()`.

## Connecting a GTK4 frontend

A future GTK4 timer should call `umi_studio_ui_advance()` with real elapsed
seconds. A window activation handler should call
`umi_studio_ui_set_background()`. Opening or selecting a file can call
`umi_studio_ui_context_changed()` for a source-aware panel.

The standard Studio policy reduces timed work while the application is behind
another window. It requests a checkpoint every 30 seconds. This keeps the IDE
responsive without wasting resources.

## Adding an IDE panel

Do not begin by copying another Studio panel. First ask whether the panel can be
useful to another application. If it can, add the contract, panel specification
and behavior profile to Framework. Studio should then add only its controller,
commands and product-specific presentation.

For example, a reusable terminal belongs to Framework. A Studio command that
opens that terminal beside the active source file belongs to Studio.

## What this does not do

The scheduler does not create threads and does not choose a GTK timer. It also
does not save files by itself. It produces portable decisions that the Studio
frontend and persistence services can carry out safely.
