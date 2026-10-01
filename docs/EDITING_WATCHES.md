# Edit and evaluate watches in Studio

A watch lets you inspect an expression while debugging. For example, if your
program has an integer called `savedNotes`, adding that expression lets you
inspect its value in a paused frame.

1. Open the Debug workspace and add a watch with **Add Watch**. The existing Add
   Watch command may immediately evaluate it if a native session is already
   paused.
2. Open **Watches**. Each row has an expression, an **Enabled** control and its
   last stored value.
3. Edit the expression and choose **Apply**. Applying does not run the
   expression. It clears the old value when the expression or enabled flag
   changes, so the old result cannot be mistaken for the new expression.
4. Use **Reset fields** to restore the applied expression and enabled state.
   Use **Remove** to remove that watch from the current list.
5. Start your normal native debug session and pause at a breakpoint. Select a
   frame in **Call Stack**. This chooses the function's local-variable context.
6. Choose **Evaluate** on the applied, enabled watch. The action may execute
   functions in the paused program. Studio will not evaluate unapplied text;
   apply or reset your fields first.
7. Read the displayed result as the last stored capture. Continuing, stepping
   and switching frames do not automatically evaluate watches again.

Expressions are nonempty and can contain at most 1023 UTF-8 bytes. Some
characters use more than one byte. Unrelated variable and frame refreshes keep
your draft fields; a change to watches, sessions or launch configurations
replaces stale rows. Watches currently last only for this workspace session.

If evaluation fails, the earlier value remains. Check the notification and
current debugger state. A disabled watch, running program, uninspected stack,
changed row, queued debugger event or rejected expression can prevent a new
value. Studio does not retry automatically. A timeout does not prove that the
expression did not execute, so inspect the program before trying again.

An empty result is allowed. Oversized or malformed adapter results are rejected
rather than presented as shortened or successful values. Values can be selected
and copied as text, but child objects, result history and target-variable
editing are not part of this watch editor.

See [editing breakpoints](EDITING_BREAKPOINTS.md) for conditions and log messages.

To explore a local structure or array in the Variables panel, see
[inspecting variables](INSPECTING_VARIABLES.md). This does not expand watch results.
