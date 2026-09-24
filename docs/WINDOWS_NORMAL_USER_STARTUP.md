# Umicom Studio IDE normal-user startup

Umicom Studio IDE should open by double-clicking `umicom-studio-ide.exe` or a Windows shortcut. It should not need **Run as administrator** and it should not treat `C:\Windows\System32` as a project merely because Windows supplied that process working directory.

## Startup behaviour in this batch

Before Studio constructs its service graph, it asks Umicom Framework for the current user's application-state root and prepares it. On Windows the normal location is:

```text
%LOCALAPPDATA%\Umicom\Studio
```

Studio then uses that writable directory for its established relative settings and session files. This keeps the existing file formats and service contracts while removing the dependency on the folder that launched the process.

The executable path is resolved before the working directory changes. Icons and packaged Framework/GTK resources therefore continue to resolve beside the executable rather than from the user-state directory.

## Workspace restoration

Current Studio services still require an initial directory while their service graph is being constructed. The per-user state root is used only as that safe startup location.

After the Framework lifecycle has started, Studio:

1. restores an explicitly saved workspace when it still exists;
2. otherwise consults the existing recent-work catalogue;
3. otherwise closes the temporary startup workspace and shows the welcome experience.

A restored workspace starts untrusted and follows the same watcher and workspace service used by an explicit **Open Workspace Folder** action. When the user opens a workspace through the GTK4 folder/recent-work route, Studio also saves that selected root through its existing session store so the next normal launch can restore it. A session-write problem does not close an otherwise valid workspace; Studio reports that the choice could not be remembered.

The change does not grant workspace trust or elevated permissions.

## What this batch does not change

This batch does not replace the Studio editor, workbench, build service, debugger, Test Explorer, source-control services or legacy comparison frontend. It also does not change Windows DLL staging or the installer selection UI. Those existing Framework deployment mechanisms remain in place.

## Manual acceptance check

After building `windows-ucrt64-all-debug` and refreshing the Windows runtime:

1. Close every running Umicom application.
2. Open Windows Explorer.
3. Browse to `build\windows-ucrt64-all-debug\bin`.
4. Double-click `umicom-studio-ide.exe` as a normal user.
5. Confirm Studio opens without a UAC prompt.
6. Confirm the title does not identify `system32` as the workspace.
7. Open a real workspace, close Studio normally and start Studio again.
8. Confirm the real workspace is restored when session restoration is enabled.
9. Open a C file and confirm GtkSourceView loads its language data without the earlier RelaxNG warning after the runtime staging target has been refreshed.
