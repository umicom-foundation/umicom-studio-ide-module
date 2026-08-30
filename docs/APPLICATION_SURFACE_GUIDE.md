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

Studio can now start the Framework learning, standard or focus recipe. The
standard recipe supplies ten reusable components: editor, explorer, designer,
build, debug, tests, source control, terminal, AI chat and diagnostics. Learning
reduces the number of panels for a new developer. Focus keeps the immediate
editing task visible with less surrounding information.

Studio contributes only the messages and actions that are specific to an IDE.
For example, an empty editor explains how to open a file and an empty build
panel explains that output will appear after a build. Framework owns panel
visibility, focus, state, dispatch, checkpoints and the portable window plan.
The Studio API delegates allocation, headless-host binding and lifecycle to
`UmiApplicationPresentationProductSurface`, so another product does not need to
copy Studio startup code.

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

Use `umi_studio_application_surface_create()` for the standard layout. Use
`umi_studio_application_surface_create_for_audience()` when a setting, first-run
journey or accessibility choice asks for learning or focus.

The public Studio helpers in `application_surface_commands.h` use meaningful
component identifiers. They never depend on widget order or panel indexes.
