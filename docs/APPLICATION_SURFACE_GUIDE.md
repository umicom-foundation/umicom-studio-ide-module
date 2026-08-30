<!--
Umicom Studio IDE
File: docs/APPLICATION_SURFACE_GUIDE.md

PURPOSE:
Explain how Studio adopts the Framework application surface runtime without
replacing its existing workbench, services or GTK4 frontend.

AUTHOR AND ORGANISATION:
Sammy Hegab
Umicom Foundation

LICENCE:
MIT
-->

# Studio Application Surface

Studio now starts the Framework recipe
`org.umicom.workspace.studio.standard` as part of its normal UI composition.
The recipe supplies ten reusable components: editor, explorer, designer, build,
debug, tests, source control, terminal, AI chat and diagnostics.

Studio contributes only the messages and actions that are specific to an IDE.
For example, an empty editor explains how to open a file and an empty build
panel explains that output will appear after a build. Framework owns panel
visibility, focus, state, dispatch, checkpoints and the portable window plan.

This is additive. The existing `UmiUiWorkbench`, document coordinator,
headless adapter and GTK4 adapter are still active. The new surface is exposed
as the service `umicom.studio.application-surface`, which allows later frontend
work to adopt the shared runtime without another migration of product logic.

When adding a Studio feature:

1. Look for a suitable Framework component first.
2. Add the component to a Framework recipe instead of hard-coding a GTK panel.
3. Put reusable behavior in Framework.
4. Put only Studio data binding and commands in a Studio controller.
5. Add a headless surface test, then connect visual rendering.

The public Studio helpers in `application_surface_commands.h` use meaningful
component identifiers. They never depend on widget order or panel indexes.
