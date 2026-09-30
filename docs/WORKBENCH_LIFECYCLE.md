# STU-02 — Workbench shell and window lifecycle

This batch changes the real Studio close path.

## Behaviour

1. Test Explorer run/discovery is asked to stop before broader close inspection.
2. Running builds are cancelled through the existing Studio/Framework build service.
3. Active native debugger sessions are stopped through the existing DAP-backed service.
4. A window is not destroyed merely because the user clicked **Close Anyway**.
5. The confirmation re-enters `close-request`; if new work appeared, Studio stops it first.
6. If Studio had to stop owned work, discard confirmation is cleared. The user closes again
   after the operation reaches a stable state.
7. Dirty-document policy remains Framework-owned through
   `UmiStudioCloseGuardEvaluateWithActivity()`.

The previous Test-Explorer-only close helper and direct window-destroy path remain in
`#if 0` blocks for engineering review, following the project preservation rule.
