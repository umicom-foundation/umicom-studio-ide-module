# STU-02 close-lifecycle fixture matrix

These 256 files enumerate every combination of eight externally observable
boolean inputs to the Studio close planner. They are acceptance data, not
generated padding: the native fixture test reads each file and compares it with
the product close contract.

Bit order in the numeric mask:

0. services ready
1. Framework/Studio snapshot ready
2. Test run pending
3. Test discovery pending
4. Build busy
5. Debugger busy
6. Dirty documents present
7. User has confirmed discard

Running work always has precedence over discard confirmation. If Studio has to
stop owned work, the previous discard confirmation is invalidated and another
close request is required after the operation reaches a stable state.
