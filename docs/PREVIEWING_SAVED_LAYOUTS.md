# Preview saved Studio layouts

Use **Preview saved** to inspect the last saved layout list before deciding
whether to restore it. The preview leaves your current workspace in place.

## Try it step by step

1. Apply or cancel any panel-layout edit, then open **Layout Library**.
2. If you want to save your current list first, choose **Save library**.
3. Rename, duplicate or reorder a layout without saving the library again.
4. Choose **Preview saved**. Read the saved names and order beneath the storage
   controls. The preview marks additions, removals and the saved active layout.
5. Close the library to keep working, or explicitly confirm **Replace this
   session's named layout list** and choose **Restore library**.

Keep an unsaved source-file draft open while trying the preview. The current editor and its draft remain in place. A layout-library checkpoint is not a source-file backup; save source files separately.

## Read the result correctly

An added row would return from the saved library. A removed row exists only in
the current session. Position numbers show the current and saved list order.
Counts include hidden panels. The summary does not compare individual panel
positions, sizes or document contents, so matching summaries can still describe
different arrangements.

**Memory-only** means the saved copy will not survive process exit. A persistent
source uses the application's connected durable storage. If the preview uses
an earlier valid recovery copy, it says so. Preview does not repair storage or
save anything. With no saved copy, it reports that nothing is available and
keeps the current session unchanged.

Restore reads storage again; another window may have saved newer layouts after
your preview. Preview again before restoring when sharing the same storage.
The preview clears earlier restore consent, so replacement always needs a new
confirmation. Refresh and layout/storage actions clear the old summary.

If preview reports a changed workspace or connection, select **Refresh**, check
the storage message, and preview again. A storage error keeps the current list.
Previewing after a Save conflict does not authorize overwriting another window's
saved library. Resolve the conflict through the existing Restore workflow.

See [Order and copy layouts](ORDERING_WORKSPACE_LAYOUTS.md) for the other library
controls. Both applications use Framework's shared comparison and storage code.
