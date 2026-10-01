# Edit source breakpoints

A source breakpoint asks the debugger to stop at a line of your program.
A condition limits when it stops. A logpoint writes a message instead of
stopping. Studio sends these expressions to your selected debugger; it does
not interpret them itself.

## Add and inspect a breakpoint

1. Use **Run > Add Breakpoint** or find **Add Breakpoint** in the command search.
2. Enter the source path followed by a colon and its one-based line number,
   for example **C:/projects/notes/main.c:13**. Use your real source file.
3. Open the **Debug** workspace and select **Breakpoints**.
4. Find the row by its source path, line and column. The row states whether it
   is disabled, not verified, or verified by the adapter.

## Apply properties

1. Enter a **Condition**, such as **count > 3**. Empty means always.
2. To log instead of stopping, enter a **Log message**, such as
   **count={count}**. Empty means a stopping breakpoint.
3. Select or clear **Enabled**.
4. Choose **Apply**. Typing or changing the checkbox alone does not change
   the debugger's stored breakpoint. **Reset fields** discards these field edits.
5. Read the notification and the row's verification status.

Both text properties accept up to 511 UTF-8 bytes. Some characters use more
than one byte. Overlong input is rejected rather than shortened. The selected
adapter determines which expressions and interpolation syntax it accepts.
Use expressions appropriate for your program; expressions can have side effects.

When a native debug session is connected, Apply sends the complete breakpoint
set for that source. A successful exchange still permits individual rows to be
unverified when the adapter cannot install them. When no native session is
connected, properties remain in memory for the next launch. Applying a row
never launches a program.

## Understand refusal and recovery

Conditions and logpoints require advertised adapter support. Unsupported
properties are retained locally, but are not sent as if they were ordinary
breakpoints. Clear the unsupported property, disable that breakpoint, or select
a suitable adapter after stopping the session.

If a request fails or returns incomplete evidence, the source set is left
unconfirmed. Inspect the rows and the connection before choosing Apply again.
Do not assume that a timed-out request changed nothing in the adapter.

A source row captured before a breakpoint, session or launch configuration
changed cannot apply or remove a different current breakpoint. Refresh and
use the current row. A build queued to launch the debugger temporarily refuses
property edits. Try again when that transition finishes or is cancelled.

Unrelated watch or frame refreshes keep the current fields and keyboard focus.
Changing the breakpoint collection itself rebuilds the rows and discards their
unapplied fields. The original breakpoint remains unchanged until Apply.

## Remove and scope

Choose **Remove** on the intended row to remove that source breakpoint.
This is an explicit action even if the property fields contain unapplied edits.
With an active adapter, Studio sends the remaining source set, including an
empty set when the final breakpoint is removed. A failed exchange does not
guarantee that the adapter has removed it; stop or deliberately resynchronize.

This editor covers enabled state, conditions and log messages for source
breakpoints. It does not add hit-count expressions, function/data/instruction
breakpoint editors, automatic undo, or breakpoint persistence across application
restarts. Rebuild Framework and Studio together after updating the shared API.

For expression inspection, see [editing and evaluating watches](EDITING_WATCHES.md).
