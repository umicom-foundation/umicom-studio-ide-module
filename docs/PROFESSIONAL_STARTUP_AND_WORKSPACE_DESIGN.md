<!-- --------------------------------------------------------------------------
Umicom Studio IDE
File: docs/PROFESSIONAL_STARTUP_AND_WORKSPACE_DESIGN.md

PURPOSE:
Explain the professional startup and workspace design, the Framework contracts
behind it, and the rules junior developers should follow when extending it.

AUTHOR AND ORGANISATION:
Sammy Hegab
Umicom Foundation

LICENCE:
MIT
---------------------------------------------------------------------------- -->

# Professional Startup and Workspace Design

This guide explains how Umicom Studio should look, behave and grow. It is
written for a developer who may be seeing an IDE codebase for the first time.

The main idea is simple: **the editor is the work, and everything around it is
a tool**. Menus, toolbars, Explorer, Problems and AI panels must help the user
without competing with the source code for attention.

## What was wrong with the earlier screen

The earlier screen contained many useful functions, but their presentation made
them difficult to understand:

- every region had almost the same visual weight;
- large fixed panels left too little space for source code;
- explanatory paragraphs were permanently visible inside working panels;
- several rows of controls repeated similar actions;
- the welcome document looked like an ordinary source file;
- layout controls existed, but moving and resizing regions was not obvious;
- there was no calm transition between starting Studio and entering a project.

The problem was not the number of features. The problem was hierarchy. A
professional application can contain thousands of features if it reveals them
at the right time.

## What professional products teach us

The design is informed by official product documentation, not by copying a
screenshot pixel for pixel.

- [RAD Studio's IDE tour](https://docwiki.embarcadero.com/RADStudio/Athens/en/Tour_of_the_IDE)
  separates its menu, toolbars, central editor and docked tool windows.
- [RAD Studio's Welcome Page](https://docwiki.embarcadero.com/RADStudio/Alexandria/en/Welcome_Page)
  gives new projects, recent work and learning material a dedicated start
  surface.
- [RAD Studio desktop layouts](https://docwiki.embarcadero.com/RADStudio/Sydney/en/Saving_Desktop_Layouts)
  let a developer save arrangements for different kinds of work.
- [Visual Studio window layouts](https://learn.microsoft.com/en-us/visualstudio/ide/customizing-window-layouts-in-visual-studio?view=visualstudio)
  support docking, floating, hiding, tab groups and named saved layouts.
- [IntelliJ IDEA tool-window layouts](https://www.jetbrains.com/help/idea/tool-window-layouts.html)
  treat tool windows as a reusable arrangement rather than a collection of
  fixed boxes.
- [IntelliJ IDEA tool-window controls](https://www.jetbrains.com/help/idea/manipulating-the-tool-windows.html)
  keep common move, resize, hide and restore actions close to the panel.
- [IBKR Mosaic layouts](https://www.ibkrguides.com/traderworkstation/mosaic-layout.htm)
  use named workspaces, an explicit edit mode and a lock to stop accidental
  movement.
- [IBKR colour-linked panels](https://www.ibkrguides.com/traderworkstation/color-grouping-linked-panels.htm)
  show how several tools can follow the same selected instrument or context.

These products differ in appearance, but they share the same structural lesson:
layout is user-owned state and commands are product-owned behaviour.

## The new startup sequence

Studio now follows a clear sequence:

1. A small branded splash appears immediately.
2. The progress message names the real startup stage.
3. Umicom Framework creates and starts its services.
4. Studio restores the saved workspace and layout state.
5. The main workbench is presented.
6. The splash closes only after the workbench has been created.

The splash is intentionally simple. It is not a second application window and
does not contain menus, advertisements or fake progress. Its implementation is
isolated behind the `UmiSplash` API so it cannot take ownership of Framework or
Studio services.

## The welcome page

When no normal document is active, the centre of Studio shows a dedicated
welcome page. It has three jobs:

- **Start** creates or opens work;
- **Recent Work** gives a home to future locally stored recent projects;
- **Learn and Explore** opens lessons and documentation.

Extensions, toolchains and settings are available as small secondary actions.
Every button dispatches an existing Framework command. The welcome page does
not implement project creation, file loading, extension management or teaching
logic by itself.

## The workbench hierarchy

The visible hierarchy is:

```text
Application window
├── Menu and command bar
├── Main toolbar
├── Resizable workspace
│   ├── Activity rail
│   ├── Primary tool region
│   ├── Centre
│   │   ├── Welcome page, or
│   │   └── Framework editor workbench
│   ├── Secondary tool region
│   └── Bottom tool region
├── Named layout strip and layout lock
└── Status bar
```

The centre always receives the remaining space. Primary, secondary and bottom
regions have sensible minimum sizes, but they are not allowed to dominate the
window. Explanations that are useful to a beginner but not needed every minute
are placed in collapsed sections.

## Resize, save, restore and lock

The GTK splitters are presentation objects. Their dimensions are stored in the
toolkit-neutral `UmiUiWorkbenchState`:

| Visible region | Framework state |
|---|---|
| Primary tools | `sidebar_size` |
| Secondary tools | `auxiliary_sidebar_size` |
| Problems, Output and Terminal | `bottom_panel_size` |

When **Editing Layout** is active, moving a splitter updates Framework state.
When **Layout Locked** is active, an accidental drag restores the saved
position. Save and Restore are routed through `studio.layout.save` and
`studio.layout.restore`.

Umicom Framework already owns the deeper docking behaviour. Its GTK adapter
uses grouped notebooks whose tabs can be reordered and detached. A moved panel
updates the Framework pane placement model, so a later Qt, web or headless
frontend can understand the same layout. Studio must extend that path instead
of building a second private docking database.

## Panel grouping and shared context

There are two different kinds of grouping. They must not be confused:

1. **Visual grouping** places panels together as tabs or split regions.
2. **Context grouping** makes panels follow the same selected project, file,
   test, debugger frame, financial instrument or other subject.

Visual grouping belongs to the Framework pane and workspace-profile models.
Context grouping belongs to the Framework Workbench Context Host and Context
Channel contracts. The existing colour groups provide the reusable foundation
for the same idea that a trading workstation calls linked panels.

A future Studio panel header should render those existing context groups. It
must not invent Studio-only red, green or blue link state.

## Visual rules for future panels

Use these rules whenever a panel is added:

1. Give the panel one short title.
2. Put its most common actions in one compact action row.
3. Use icons only when their meaning is familiar; always add a tooltip.
4. Put rare actions in a menu or command palette.
5. Put educational explanation in a collapsed section or Help page.
6. Allow the main list, tree or editor to expand.
7. Never use a fixed width when a minimum width is enough.
8. Use Framework commands and state instead of private GTK business logic.
9. Make keyboard operation and accessible labels part of the first version.
10. Test empty, loading, ready and error states—not only the happy path.

## How to add a feature safely

Suppose you want to add a Package Manager panel.

1. Search Framework for an existing package service, command and view model.
2. Add missing reusable contracts to Framework only if other applications can
   benefit from them.
3. Register the panel and commands through the Application Shell catalogue.
4. Add a small GTK presenter that reads the view model and dispatches commands.
5. Add the panel to a Framework workspace profile.
6. Check that save, restore, close, keyboard and accessibility behaviour work.
7. Add a beginner-friendly guide explaining the purpose, not every line of C.

This is the LEGO principle in practice: services are the working pieces,
contracts are the connectors, layout profiles are the instruction sheet, and
the GTK code is only the visible surface.

## Remaining professionalisation work

The current change establishes the startup and visual hierarchy. The following
work should continue through existing Framework contracts:

- render Framework-owned move/close/pin actions in every Studio panel header;
- render Context Host colour groups in relevant panel headers;
- provide drag previews and keyboard docking commands;
- support floating tool windows and multiple monitors;
- make recent projects a local persistent service rather than placeholder text;
- add responsive two-column and one-column welcome layouts for narrow windows;
- add screenshot-based visual regression checks for light, dark and high
  contrast themes;
- add an accessibility pass for focus order, names and contrast.

These items are intentionally explicit. A polished IDE is created by finishing
interaction details, not by adding more panels faster than users can understand
them.
