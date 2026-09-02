<!--
Umicom Studio IDE
File: docs/CUSTOM_IDE_WORKSPACES.md

PURPOSE:
Teach new Studio contributors how editable layouts, movable tool panels and
layout recovery work.

AUTHOR AND ORGANISATION:
Sammy Hegab
Umicom Foundation

LICENCE:
MIT
-->

# Custom IDE Workspaces

Umicom Studio uses the same workspace ideas as the rest of the Umicom suite,
adapted for software development. The current GTK workbench now exposes a
searchable **New Window** manager for its real Framework-backed surfaces.

## How to change a layout

1. Select **Layout Locked** at the bottom of Studio. The label changes to
   **Editing Layout**.
2. Resize the left, right and bottom regions with their splitters.
3. Select **New Window** in the application bar.
4. Search for Explorer, Problems, Output, Terminal, Source Control, Tests,
   Debug, Build, an AI surface, or one of the visual-design surfaces.
5. Move the panel to Left, Right, Bottom or Centre. Select **Float** to place it
   in a separate native window, or select **Hide** when it is not needed.
6. Select **Editing Layout** again to lock the result.
7. If the experiment did not help, select **Cancel Edit**. Studio restores the
   panel locations, floating state, visibility and splitter sizes captured when
   editing began.

A floating panel remembers its previous dock region. Close its native window
while the layout is unlocked and Studio returns the panel to that region. A
locked layout refuses this structural change, preventing an accidental window
close from silently rearranging the workspace.

The editor remains the normal document region. A tool panel can temporarily
join the Centre tab stack, while source documents continue to use the Framework
editor-group and document-tab services.

## Why locking matters

Large IDEs contain many small surfaces. An accidental drag can make a familiar
tool disappear during important work. Studio therefore separates normal use
from layout editing. Locked mode is the everyday mode. Editing mode is a clear,
temporary workspace-design task.

## Relationship to named layouts

The layout strip contains task-oriented arrangements such as Default,
Debugging, Testing, Source Control, AI Development, Review and Zen. The Layout
Browser explains each preset. The New Window manager changes which real panels
are shown in the active shell.

The **Design** layout groups Design, Code, Mixed and Preview with Components,
Structure and Properties. These panels use the same docking, floating,
grouping and lock rules as the rest of Studio; any mode can be closed, reopened
from the window catalogue or detached to another monitor.

The **Code + Preview** layout is for ordinary editor documents rather than the
visual designer model. It places the active editor beside the Live Preview
panel with Problems and Output below. Live Preview can still be moved, grouped,
floated or hidden after the layout is unlocked.

The Framework now provides two related persistence paths. Studio's semantic
runtime commands save and restore the shell session. The professional workspace
API also delegates portable active-layout export and validated import to
`UmiUiWorkspaceCustomisation`, so Studio has no private layout parser. Import
rejects an existing identifier unless the caller explicitly asks to replace it.

The next integration step is to connect every native detached-window position
and monitor identity to the same semantic session. This will let a complete
multi-monitor arrangement recover after a monitor is removed without creating
Studio-only storage rules.

## What Save and Restore mean

- **Save** records the current committed semantic Studio session.
- **Restore** loads the last saved semantic session and refreshes the shell.
- Portable export produces text suitable for a trusted settings store or a
  reviewed layout-sharing workflow.
- Portable import checks panel and context dependencies before accepting a
  layout; it never accepts half of a layout.
- Save and import wait until layout editing is applied or cancelled, preserving
  the meaning of **Cancel Edit**.

## Design principles

Studio combines familiar desktop IDE conventions with explicit workspace
editing commonly found in professional workstation applications. It remains
an original Umicom IDE with Framework-owned contracts, an Umicom visual
language and developer workflows designed for this project.
