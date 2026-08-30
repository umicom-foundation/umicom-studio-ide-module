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
   Debug, Build or an AI surface.
5. Move the panel to Left, Right or Bottom, or hide it.
6. Select **Editing Layout** again to lock the result.
7. If the experiment did not help, select **Cancel Edit**. Studio restores the
   panel locations, visibility and splitter sizes captured when editing began.

The editor remains the central document region. Tool panels may move around it;
ordinary source documents continue to use the Framework editor-group and
document-tab services.

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

The next integration step is to persist every move through the Framework
semantic layout service. That will let custom layouts be cloned, renamed,
restored on another monitor and recovered after an interrupted session without
creating Studio-only storage logic.

## Design references

Studio combines well-understood desktop IDE conventions with the explicit
workspace editing found in trading workstations. The relevant TWS behaviours
are documented in the official [Anchor Window guide](https://www.ibkrguides.com/traderworkstation/mosaic-anchor-window.htm),
[custom layout guide](https://www.ibkrguides.com/traderworkstation/custom-layouts.htm)
and [display configuration guide](https://www.ibkrguides.com/traderworkstation/display-configuration.htm).

Umicom uses those public behaviours as research. Studio remains an original IDE
with its own Framework contracts, visual language and developer workflows.
