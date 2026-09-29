# Umicom Studio IDE — STU-01 Stable Release Contract

**Checked remote baseline**

- `umicom-applications/main`: `6b997b0424c367173403d1550cba9e403e5a50f0`
- `umicom-framework/main`: `9547695350655f6e8a8b1e2440af1ebaf8dbb24a`
- `umicom-studio-ide-module/main`: `b8b1ca6757701ab58d9cf62cebb586546ccdb1a0`

The Applications parent is behind the latest Framework main; update the Framework
checkout first and let `umicom repo lock .` write the reviewed pin after validation.

## First stable release

A daily-use native IDE for C23, Assembly and C++ toolchains, with an explicitly
qualified CPython profile. The release must complete the real editing, saving,
build, test, diagnostics, debugging, Git, designer, recovery and installation
journeys through Studio's actual interface.

## Release rule

A feature is not complete merely because its library target exists. It must have
a real Studio entry point, an executable acceptance test, failure behaviour and
installed-product evidence for the supported platform.

The twenty release areas are defined in
`include/umicom/studio/release_baseline.h` and exposed by the native
`umicom-studio-release-baseline` command.

## Current checkpoint

STU-01 remains open. This checkpoint makes the release contract executable and
does not mark any of STU-02..STU-20 as passed.
