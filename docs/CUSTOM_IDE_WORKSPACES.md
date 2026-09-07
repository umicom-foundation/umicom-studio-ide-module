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

## Title bar and side tools

The topmost title bar contains the official Umicom SVG and application name.
The document and project appear beside that fixed identity. Menus and the
toolbar are separate rows beneath it.

In the current default layout, click Structure or Object Inspector on the left
edge. The selected tool opens over the workspace. Click another tab to switch,
or use X to collapse it. Its contents remain available when reopened. Escape
and clicking outside the panel also collapse it. Use the dock control to keep
the tool open as an ordinary docked panel.

Saved layouts are not silently replaced by a new product default. Choose
**New default layout**, give the new copy a name, and test it before saving.
Your other layouts remain available in the layout selector.

This behaviour is implemented in source and has native regression tests.
It has not yet been compiled or tested in a running application for this update.

Studio's main window now uses the shared Framework workspace host in source.
Its professional-workspace model owns panel placement and the edit baseline.
The native integration and new regression tests have not been compiled or run
during this update. Follow the acceptance steps below after building.

An arrangement controls which tools are visible and where they appear. It does
not own the files, document buffers or business services used by those tools.
The Editor panel contains Studio's original Framework editor, not a summary
card or another editor implementation.

## How to change a layout

1. Choose a preset in the layout strip, or use **New canvas** to create a named
   empty arrangement. The layout selector includes your new canvas.
2. Unlock a preset before moving its panels. A newly created canvas starts in
   edit mode so you can add its first tools immediately.
3. Select **New Window** and search for Editor, Explorer, Problems, Terminal,
   Source Control, Tests, Debug, Build or a design tool.
4. Use **Canvas** to place the selected tool as an independent rectangle.
   Its title handle moves it. Drag any edge or corner to resize it while keeping
   the opposite edge in place. Movement uses the shared edit grid and remains
   inside the available host area.
5. The same catalogue offers dock destinations, **Float** and **Hide**.
   A floating panel is a separate operating-system window. A canvas panel is
   still inside Studio. These are two different arrangements.
6. Select **Apply and Lock** to keep the current in-session arrangement, or
   **Cancel Edit** to restore the model captured when editing began.

Closing a detachable native panel is a guarded model action. Locked or pinned
panels cannot be removed accidentally. Hiding Editor hides its panel, not its
open documents. Reopen Editor from New Window to return to those documents.
Moving a panel does not create another instance of its service.

For keyboard control, use Tab to focus the panel title during Edit Layout.
Arrow keys preview movement by one grid step. Hold Shift with an arrow to
resize the bottom or right edge. Enter applies the preview to the current
layout edit. Escape or leaving the title cancels the preview. Apply and Lock
still finishes the layout edit; Save remains a separate storage operation.
These shortcuts belong to the title only, so typing in a tool or editor does
not move its panel. Protected panels cannot be moved, and fixed-size panels
cannot be resized.

The New Window search now reads the canonical Framework description and
plain-language aliases for each Studio surface. A user can therefore search by
an intended job such as building, diagnostics, source history, visual design
or model comparison without memorising the panel's exact title. Each catalogue
row also reports its compact policy so the result of moving to a smaller screen
is understandable before the layout changes.

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

The professional workspace uses `UmiUiWorkspaceCustomisation` for panel edits
and portable layout serialization. Studio has no private canvas parser or
second geometry store. Its older shell-session and document services are not
the same document format as this canvas. They must not overwrite the canvas
during periodic status updates.

An explicit canvas checkpoint is now connected to the Framework Data Server in
source. It stores the last layout you saved, not the whole named-layout library.
Monitor restoration, older-layout migration and the complete library remain
separate work. An in-session Apply action does not write a checkpoint.

## What Save and Restore mean

- **Apply and Lock** commits the current layout transaction in memory.
- **Cancel Edit** restores panel geometry, visibility and context membership
  from that transaction's baseline.
- **Save layout** writes the applied arrangement and retains a previous valid
  checkpoint. Save is unavailable during an unfinished layout edit.
- **Restore saved** loads the last explicit checkpoint. If it cannot be used,
  Framework may recover its previous valid copy. Failed validation leaves the
  visible layout intact, and does not delete the damaged record.
- Document saving remains an independent feature. Save source files normally;
  a layout edit must never replace or discard their unsaved buffers.
- The status label distinguishes a checkpoint on disk from a memory-only copy.
  A memory checkpoint does not survive restart. The label describes the stored
  checkpoint; it does not claim that later panel changes were also saved.

Normal native startup uses Framework to open a database in the operating
system's user configuration directory, under
`umicom/org.umicom.studio/workspace-layouts.sqlite3`. If Studio services already
have a configured SQLite Data Server, the canvas reuses it. When storage cannot
be opened, Studio keeps its useful default layout and reports the unavailable
state. The native offline test mode does not open this user directory.

The saved arrangement is application-wide for the current operating-system
user. It is not tied to one source file or project. Restart restores that last
explicitly saved layout when session restoration is enabled. Source files,
terminal processes, connections and business data are outside this checkpoint.

Two running Studio windows can both read the same checkpoint. Save compares the
storage revision with the revision that window last read. If another window
saved first, the older window reports a conflict instead of overwriting it.
Restore explicitly before saving again. This replaces the current arrangement,
so first decide whether you want to keep that arrangement elsewhere.

If storage metadata is damaged and its revision cannot be trusted, Save stays
unavailable. Recovery does not silently repair or overwrite unknown records.
An explicit repair workflow remains to be implemented. Integrity hashes detect
damaged content; they are not a signature or proof of who wrote it.

## Check the real editor and panel contents

Type an unsaved draft in Editor and leave text in another tool's input field.
Move those panels, change the layout lock and switch to a blank canvas. Reopen
the tools. The existing provider bodies should return with their text intact.
Normal status refreshes should not replace unchanged editor buffers, steal the
cursor or reopen a deliberately hidden panel.

The current GTK editor uses a bounded document snapshot: its text limit is
16,383 UTF-8 bytes, not 16,383 characters. Some characters use more than one
byte. An insertion that exceeds the limit is rejected with an explanation;
replacing selected text must preserve the earlier draft if insertion fails.
This safety guard is not large-file support. Connecting the editor directly to
the full document store remains work to complete. Cursor and selection values
are translated between GTK characters and Framework byte offsets.

The shared host keeps at most 64 retained panel instances. A full cache reports
a capacity error; it does not silently evict a hidden draft. A deliberate
provider refresh can replace a body, such as after a design-model change.
Provider refresh is distinct from rearranging a panel and needs its own draft
policy. Eligible internal panels now offer temporary maximisation through the
shared host. Restore returns the same provider body to its original frame;
saved geometry and the layout lock do not change. Protected, fixed-size,
detached and auto-hide panels do not use this mode. Movable menus, visual
docking targets and full monitor recovery remain outstanding. The new controls
still need native interaction acceptance.

## Find and focus a tool

Use command search to find a registered panel, including Editor. Open/Focus
selects its existing view; it does not reposition it. A hidden ordinary tool is
reopened through the workspace model, while an auto-hide tool is revealed
temporarily. The Window catalogue also provides Open/Focus alongside its
existing placement actions. Choosing a new position still requires Edit Layout.

Maximise an eligible panel to use the available workspace. Choose Restore panel
to return to the earlier view. Escape restores when the panel has not consumed
that key itself. Selecting another tool also restores the normal arrangement.
This is a view operation, not Save, Apply or an operating-system window change.

The new native test uses Studio's actual constructor with explicit offline,
unpresented options and a temporary service directory. Production constructors
keep their existing defaults. Skipped or unexecuted tests are not passes.

## Design principles

Studio combines familiar desktop IDE conventions with explicit workspace
editing commonly found in professional workstation applications. It remains
an original Umicom IDE with Framework-owned contracts, an Umicom visual
language and developer workflows designed for this project.
