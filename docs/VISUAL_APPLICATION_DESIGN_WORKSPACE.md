<!--
Umicom Studio IDE
File: docs/VISUAL_APPLICATION_DESIGN_WORKSPACE.md

PURPOSE:
Explain how Studio's integrated visual-application-design workspace is composed
from reusable Umicom Framework services.

AUTHOR AND ORGANISATION:
Sammy Hegab
Umicom Foundation

LICENCE:
MIT
-->

# Visual Application Design Workspace

Select **Design** in the layout strip or **Visual Designer** from the Tools
menu. Studio opens seven synchronized surfaces:

- **Design** shows semantic component geometry with undo and redo.
- **Code** edits the canonical declarative definition. A short debounce avoids
  parsing incomplete keystrokes.
- **Mixed** places the same source and design canvas on opposite sides of a
  resizable splitter.
- **Preview** shows the latest valid visual result and the inspectable
  toolkit-neutral renderer output.
- **Components** searches the Framework component catalogue. Selecting a row
  adds that component below the selected container, or below the root when the
  current selection cannot contain children.
- **Structure** shows the component hierarchy. Selecting a row changes the
  active component for all designer surfaces.
- **Properties** shows the selected component's registered property schema.
  Apply validates the entered text before recording an undoable change.

The seven surfaces are normal Studio workbench windows. Unlock the layout to
move them between Left, Right, Bottom and Centre, detach them into native
windows, group them as tabs, hide them or restore them. Lock the layout when the
arrangement is ready. The named layout and existing workspace persistence
remain responsible for restoring the arrangement.

## How the pieces relate

`UmiDesignerDocument` owns components and revision state. `UmiDesignerHistory`
owns undo and redo. `UmiDesignerSelection` owns the current component.
`UmiDesignerWorkspaceModel` projects the catalogue, hierarchy, selection,
values and property schema for a single revision.
`UmiDesignerLiveSourceWorkspace` coordinates Design, Code, Mixed and Preview
state with bounded text, source and preview revisions, configurable debounce,
stale-result rejection and last-known-good preview retention. Studio's GTK code
renders these Framework records and sends user intent back through
`UmiStudioDesigner`.

This separation allows future desktop, web and headless tools to use the same
designer behavior without copying GTK code or creating a second component
model.

## Current boundaries

The integrated slice supports catalogue search, component insertion, tree
selection, typed property editing, undo, redo, semantic canvas rendering,
editable declarative source, mixed authoring, debounced validation and
last-known-good preview. Framework also classifies C, HTML, JavaScript, CSS and
script documents for future preview providers.

The Design and Mixed canvases support direct component manipulation. Press and
drag inside a component to move it. Drag near an edge or corner to resize it.
Selected components show four visible handles, and a quiet grid gives placement
feedback. **Snap to grid** is enabled by default and can be switched off from
the Design toolbar for fine positioning.

The canvas does not edit a GTK widget while the pointer moves. It asks the
Framework interaction service for a preview rectangle, then stores the final
rectangle as one semantic history operation when the pointer is released. This
means **Undo** reverses the complete drag in one step, Code and Preview receive
the same final geometry, and an interrupted gesture cannot leave a half-written
design. Minimum sizes and canvas boundaries also prevent a component from being
dragged into an unusable state.

Palette drag-and-drop insertion, peer alignment guides, event wiring and data
binding remain planned. Studio also has a separate Live Preview surface for
ordinary active editor documents. It safely presents HTML as readable content
and uses a plain-text fallback for other source families. Compiled and
interpreted output still requires sandboxed preview-provider adapters. Those
adapters belong behind Framework contracts first, with Studio remaining a thin
presentation layer.
