# Arrange your Studio workspaces

Keep the workspaces you use most near the top of the layout selector. For
example, you can put your everyday editing layout before debugging and keep
special-purpose layouts later in the list.

## Move a named layout

1. Finish your panel arrangement with **Apply and Lock**, or cancel that edit.
2. Open **Layout Library** in Studio's workspace controls.
3. Clear the search box, then select the layout you want to move.
4. Click **Move up** or **Move down** once for each place you want to move it.
5. Check the layout selector: it follows the same order as the library.
6. Choose **Save library** to store the named list and its new order.

Moving a row does not open it or close the current workspace. Your current
editor, unsaved text and panel arrangements remain with their existing owners.
Save your documents separately using the normal editor save commands.

## Keep or restore the order

Read the storage message before leaving Studio. **Save library** can persist
all named layouts when disk storage is connected. A memory-only message means
the saved copy lasts only for that process. **Save layout** covers only the
active layout; it is not a backup of the complete list or its order.

To return to the last saved list, select the confirmation beside **Restore
library**, then click **Restore library**. This replaces the session's named
layout list and restores its saved active selection. Review unsaved layout
changes before doing so. Document drafts are separate from this archive.

## When a button is unavailable

- The top row cannot move up; the bottom row cannot move down.
- Clear search before changing order so that hidden neighbours cannot surprise you.
- Apply or cancel a panel edit before changing the library.
- Let a pending operation finish. If the list changed elsewhere, refresh it,
  check your selection and retry.

Studio delegates these operations to Framework. The same ordered library is
used by the selector, native panels and explicit library storage; Studio does
not maintain a second list of saved workspaces.

## Make your own copy of a layout

1. Finish or cancel a panel-layout edit, then open **Layout Library**.
2. Select an existing layout as your starting point.
3. Enter a useful **Layout name**, such as `My daily workspace`.
4. Leave **New layout ID (optional)** empty, then click **Duplicate**.
5. Studio opens the new copy. Its arrangement begins with the source layout.
6. Edit the copy's panels, apply and lock the arrangement, then choose
   **Save library** to retain the complete named list and active selection.

Framework chooses an unused internal ID automatically, checking all layouts,
including any hidden by search. You can still type a manual full ID when you
need one; keep the same application prefix and choose an unused identity.
The original layout stays in the library. Your editor drafts remain separate. Save source files using the editor save commands.

If another action changed the library after your click, refresh it, review
your selection and retry. A full library needs space before a copy can be
created. If a very long source ID leaves no room for a generated suffix, enter
a shorter manual ID with the same application prefix. Read the storage message
before exiting: a memory-only save does not survive restart.

## Review a saved list before replacing the current one

Choose **Preview saved** in Layout Library to read the saved names, order,
active selection and panel counts without restoring them. Follow
[Previewing saved layouts](PREVIEWING_SAVED_LAYOUTS.md) for a short walkthrough,
including how another window's later save affects Restore.

## Open a layout with the keyboard

1. Finish any active layout edit and open **Layout Library**.
2. Search if you want a shorter list, then move keyboard focus to a visible row.
3. Press **Enter** to open that row. You can also double-click it or select it
   and choose **Open**. Single clicks only select; they do not switch layouts.
4. If the list changed while the request was waiting, choose **Refresh**, check
   the visible selection, and try again. The application does not retry a stale
   request automatically.

Opening a layout changes the current panel arrangement. It does not save a
document, send an order or save the layout library. Use **Save library** when
you want to persist the list and its active selection.
