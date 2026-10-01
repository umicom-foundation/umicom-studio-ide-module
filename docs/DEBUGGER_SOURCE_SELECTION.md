# Opening source from the debugger

The Call Stack lists the functions active when your program paused. Each row
contains the frame name, source path, line and column supplied by the debugger.
A frame is one function invocation, so the same function can appear more than
once when calls are nested or recursive.

1. Open Run and Debug and start the project's configured debugger.
2. Wait for the program to pause at entry, a breakpoint or a requested step.
3. Use Threads to select the thread you want to inspect.
4. Open Call Stack and select a frame. Studio selects the frame through its
   regular debugger command and then opens its source location if selection
   succeeds. Variables and Watches continue using the existing debugger model.
5. Read the status notification if the source could not be reached. Your text
   remains unchanged. A missing file will not move the previous editor's caret.

Rows belong to the debugger state that produced them. If the program has moved
on, Studio asks you to select a current row. A retained control from a removed
panel or closed window cannot act on a later window. Normal refresh ticks keep
unchanged row controls, preserving their identity for keyboard navigation.

Relative source paths use the captured session launch directory. They do not
depend on which directory Studio was started from. If that launch information
is unavailable, correct the configuration before retrying. Studio does not
open web or other external URI handlers for debugger frame paths.

Your unsaved source stays in the editor. If you changed it since compiling, a
debugger location may no longer identify the same statement. A missing line
leaves the source tab open for inspection and reports the failure. Review your
changes and rebuild when appropriate; Studio does not silently discard a draft
to make it resemble the executable.

Native frame, thread and scope inspection requires an active paused session.
An inactive or running session is refused before changing the selection. A
frame identifier alone is not evidence that a process is still paused.
