# Inspect variables and their children in Studio

The Variables panel shows data from the program you are debugging. A simple
value such as an integer may be shown in one line. A structure, object or
array can have children that the debugger reads separately.

1. Open a project and start your usual native debug session. Pause at a
   breakpoint or use the existing pause command.
2. Open the **Debug** workspace. Choose a thread and a function in **Call Stack**.
   Selecting a function changes which local variables you can inspect.
3. Open **Variables**. Read each row as `name = value [type]`. The type can be
   empty when the adapter does not supply it. A bullet keeps its established
   meaning that the root value was marked changed by the debugger model.
4. Choose **Inspect children** on a structure or object. Studio asks the
   debugger for one level and displays the returned rows beneath that object.
   Adapter visualizers may execute code in the paused program to obtain values.
5. Choose **Inspect children** on a child to go further. Indentation shows which
   object owns the displayed branch. Children with the same name stay separate.
6. Choose the same **Inspect children** button again to refresh just that branch.
   A successful refresh replaces its earlier child controls. A failed refresh
   keeps the earlier capture and displays a notification.
7. Choose **Collapse** to discard the displayed child branch. You can inspect
   it again while the same debugger context remains current.
8. Select text in a value and use your usual copy shortcut when you need to
   paste it into notes. Copying the text does not edit the paused program.

The panel does not continually reread children. An unchanged refresh of the
Studio window retains expanded controls. Continuing, stepping, switching
frames or changing relevant debugger records discards old expanded branches.
Expand again from the current roots when the program next stops.

## If a button is unavailable

A scalar value usually has no child reference, so its button is disabled. A
reference to an ancestor is also disabled; for example, a linked structure
may point back to itself. Studio also stops nesting after 16 requests down
one branch. Hover over the button to read the reason.

<!-- The earlier child-only limitation is preserved for engineering review.
Explicit scope inspection now provides a separate control without changing the
root loader or removing the child inspector.
If no variables appear, check that the program is paused and a frame is
selected. The current root-loading workflow skips scopes marked expensive
by the adapter; this child inspector does not add an expensive-scope picker.
-->
If no root variables appear, check that the program is paused and a frame is
selected. The root loader still skips expensive scopes. Use **Scopes in this
frame** below the roots and choose **Inspect scope** to read those groups.
See [scope inspection](INSPECTING_SCOPES.md) for the full workflow.

## If inspection does not finish

- **The adapter returned no children:** this is a successful empty response.
  Collapse the branch or inspect it again when appropriate.
- **BUSY:** process pending debugger events and choose the current stopped
  frame again. Another stop can reuse the same numeric reference for a
  different object. Studio will not apply the stale click to it.
- **Capacity exceeded:** collapse another branch if the window already has
  many expanded values. The window allows 1024 child rows across all branches.
  Each response is limited to 128 children, and long text, deep nesting or
  the transport's message limit can also cause this result. This inspector
  does not fetch pages from larger arrays.
- **Refused, malformed or timed out:** review the notification and the adapter
  state before retrying. Earlier values remain earlier captures. A timeout
  does not prove that a visualizer did not execute code.

This panel currently reads child values. It does not change variables, retain
captures after closing Studio, or expand watch results. Use
[watches](EDITING_WATCHES.md) to evaluate an expression and
[breakpoint properties](EDITING_BREAKPOINTS.md) to configure conditions or log
messages.
