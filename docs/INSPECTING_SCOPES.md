# Inspect a debugger scope

A scope is a named group of variables in a function call. Debuggers commonly
provide groups such as Locals, Arguments, Registers or Globals. Your adapter
chooses the names and contents. A group marked **expensive** may contain many
values or require more work to read, so Studio leaves it unloaded until you ask.

1. Start your usual native debugging session and pause at a breakpoint.
2. Open the **Debug** workspace. Choose the required thread and function in
   **Call Stack**. A function call is also called a frame.
3. Open **Variables** and find **Scopes in this frame** beneath the ordinary
   variable rows. Each group has an **Inspect scope** button. Two groups with
   the same name are still separate containers.
4. Choose **Inspect scope** on the group you want. Studio requests that group's
   values and displays them underneath it. Reading values can run the adapter's
   visualizers in the paused program; use the action deliberately.
5. Choose **Inspect children** on a returned object to see its nested values.
   A simple number normally has no children. Select a value's text to copy it.
6. Choose **Inspect scope** again to refresh that group. A successful response
   replaces its earlier branch. A failed response keeps the earlier capture.
7. Choose **Collapse** to release the displayed branch. This does not change
   the program or send a debugger request.

Opening the panel or refreshing the window does not read an expensive scope.
Inspecting a group does not change the selected scope or the ordinary root
rows above it. Those rows can therefore show an earlier capture than the group
you just inspected. Read each as a capture from its own request, not a live view.

If a frame contains only expensive scopes, the ordinary root list can be empty.
Use the scope controls below it. There is no need to evaluate a watch merely
to open the group.

## Recover when inspection cannot finish

- If a button is disabled, hover over it. The group may lack an expandable
  reference, or the selected thread may no longer be paused.
- If the notification says **BUSY**, allow the ordinary debugger event loop
  to process pending events. Select the current stopped frame again, then use
  its new controls. Old references cannot be reused after another stop.
- If the group is empty, Studio displays **The adapter returned no children**.
  This is a successful response; it is different from a refused request.
- If inspection is refused, malformed or times out, review the adapter state
  before another attempt. A timeout does not prove that a visualizer did no
  work. There is no automatic retry.
- If the child-row budget is full, collapse another branch. Scopes and ordinary
  variables share a limit of 1024 displayed child rows. One response must fit
  within 128 values and the existing message/text bounds. A larger response is
  rejected as a whole; this version does not request paged slices.

Continuing, stepping, selecting another frame or changing debugger detail
records discards expanded branches when the panel refreshes. An old retained
button cannot send a request after its context changes or its window closes.
The maximum nesting is 16 requests, counting the initial scope request, and
references back to an ancestor are stopped sooner.

This view reads values. It does not assign them, expand watch results, save
captures between sessions or change which scopes the adapter provides.
See [variable inspection](INSPECTING_VARIABLES.md) for nested objects and
[watch properties](EDITING_WATCHES.md) for explicit expression evaluation.
