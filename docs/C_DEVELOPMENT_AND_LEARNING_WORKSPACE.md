# C Development and Learning Workspace

Umicom Studio IDE brings the tools needed for C development into one customizable workspace. A developer can edit source files, configure and build a project, inspect compiler messages, run tests, debug a program, read local HTML documentation, and follow a sequence of lessons without leaving the workbench.

## How the workspace is organised

The central editor remains the main working area. Explorer, Build, Problems, Terminal, Run and Debug, Guided Learning, and Documentation are separate surfaces. Each surface is registered by Umicom Framework, so it can be shown, hidden, moved, docked, floated, or restored through the same layout system.

Choose the **C Learning** layout when you want the editor, lessons, documentation, build results, problems, and terminal visible together. This is a starting arrangement rather than a restriction. Unlock the layout to move a panel, then lock it again when the arrangement is comfortable.

## Editing and building C

Open a folder to create a workspace, then open a `.c` or `.h` file in the editor. Studio uses the existing Framework document, language, compiler, build, diagnostic, test, terminal, and debugger services. The GTK workbench only presents those services; it does not contain a second compiler or debugger implementation.

The Build surface reports the active build profile, build directory, diagnostics, artifacts, and last status. Its commands configure, compile, test, clean, run, install, retry, cancel, and inspect build results through the registered Studio command service.

The Run and Debug surface works with the existing debugger service. It presents breakpoints, threads, stack frames, variables, watches, pause, continue, step over, step in, step out, and stop operations.

## Reading documentation

Open **Help > Documentation** to reveal the Documentation surface. Enter a local HTML file path such as `docs/index.html`, or a `file://` URL, and choose **Open**. The Framework browser records navigation history while the Framework document-preview service extracts the page title and readable content into a bounded buffer.

This readable preview is useful on systems where a native web rendering provider has not been installed. The browser contract deliberately remains provider-neutral, so a fully interactive renderer can be added later without changing Studio commands, saved layouts, or application code.

## Following lessons

Open **Help > AI Teacher** or select **Open Interactive Learning** on the welcome page. The Guided Learning surface reads the Framework-owned foundations curriculum and shows each lesson in sequence. Expand a lesson to read its explanation, practical exercise, expected score, estimated time, and required tools.

When a lesson supplies a resource path, **Open lesson resource** sends it to the same Documentation surface. Curriculum, progression, assessment, and resource identity remain in Umicom Framework, which means another Umicom application can present the same learning material without copying the lesson logic.

## Architecture summary

- Umicom Framework owns surface identity, command activation, semantic layouts, browser navigation, safe HTML text extraction, build/debug contracts, and lessons.
- Umicom Studio owns the thin GTK presentation that turns Framework snapshots into buttons, labels, lists, editors, and panels.
- The Application Shell catalogue keeps every surface discoverable and gives future frontends the same stable identifiers.
- Layout persistence remembers visibility, placement, docking, and floating state independently from the services shown inside each panel.
