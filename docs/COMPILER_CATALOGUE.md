![Umicom](assets/umicom-logo.svg)

# Inspect compiler catalogue metadata

Studio projects Framework provider records into presentation rows containing a name, driver path, family, supported-language labels and selection state. Framework remains the owner of provider admission and argument planning. Studio's catalogue is a view, not an independent tool-discovery database.

## Publish a coherent view

1. Keep the provider registry stable while reading it.
2. Call **umi_studio_compiler_catalogue_project** with the selected **provider identifier** and a separate destination catalogue.
3. Check the status before showing the new rows.
4. Keep the prior catalogue on refusal and report the reason.

A compiler profile identifier such as **host-debug** is different from its provider identifier such as **gcc**. Resolve the active profile before choosing the selected row. The selected-provider text may borrow the old destination catalogue because it is consumed before publication.

Projection validates the complete source registry, then constructs a candidate catalogue. It refuses malformed later rows instead of hiding them behind a valid first row. It reports capacity/formatting failures without publishing a partial view. Registry storage and output catalogue storage must not overlap.

**umi_studio_compiler_catalogue_find** validates every active presentation row before returning a borrowed match. Keep the catalogue alive and unchanged while using that pointer. **umi_studio_compiler_catalogue_valid** can distinguish malformed view storage from an ordinary unsuccessful lookup.

## Inspect the command output

After an owner-run Studio build, the native command accepts optional driver metadata:

~~~powershell
.\umicom-studio-compiler-centre.exe
.\umicom-studio-compiler-centre.exe "C:/reviewed toolchain/bin/gcc.exe" "C:/reviewed toolchain/bin/clang.exe"
~~~

The command checks snapshot acquisition, catalogue projection and summary formatting before printing. Its selected row comes from the active profile's provider. Summary formatting delegates to **umi_compiler_centre_format_snapshot** through the existing Studio export interface.

These commands do not execute the named drivers. Availability labels report configured metadata. They cannot substitute for real compiler/target qualification.

Existing creation/registration/snapshot failures retain exit codes 1, 2 and 3. Catalogue refusal returns 4, summary refusal 5, allocation failure 6, stdout failure 7 and unexpected extra arguments 8. A failed output stream may already contain a prefix; check exit status before accepting it as a complete report.

## Review the priority languages

The Studio qualification list for 19 November 2026 starts with compiled workflows. Its order reflects the Studio roadmap.

| Priority | Languages in order |
| --- | --- |
| 1–4 | C23, C++, Umicom C, Assembly |
| 5–10 | Rust, Go, Zig, D, FreeBASIC, Pascal |
| 11–17 | Fortran, Ada, Swift, Java, C#, Kotlin, TypeScript |
| 18–20 | Python (CPython), JavaScript, PHP |

1. Use the Studio console or Umicom Command from a build that includes the language cohort report.

   ~~~powershell
   .\umicom-studio-console.exe --language-cohort
   .\umicom.exe project language-cohort
   ~~~

2. To inspect one tool installation, pass its absolute tool directory. For example, replace `C:/tools/jdk/bin` with the `bin` directory of your installed JDK.

   ~~~powershell
   .\umicom-studio-console.exe --language-cohort --tools 'C:/tools/jdk/bin' | ConvertFrom-Json
   ~~~

3. Check the process exit code before saving or using the report. A relative or missing directory is refused. If your Framework SDK lacks the report, rebuild Studio with an SDK that includes it.
4. Read `declared` for the features described by the language pack. Read `tools` for the tool names found in the selected scope. Without `--tools`, the scope is the console's inherited `PATH`. `located` means a regular file matched the registered name. The report performs no launches and contains no resolved paths. To qualify a toolchain, build a known project, run its tests, check a compiler diagnostic and stop the debugger at a known source location.

Focused Windows checks have exercised generated C and C++ build, test, run and GDB workflows, CPython build, test, run and `pdb` workflows, and Java JAR build, test, run and `jdb` workflows. The bounded Umicom C artifact workflow and the integer-only Java/C translation command have separate selected checks. D, FreeBASIC and Pascal have starter recipes. Verify each language and target platform with its own native tools before relying on those workflows.

## Compiler centre qualification

The authored checks are **studio.compiler_catalogue**, **studio.compiler_centre_output** and **framework.compiler.snapshot_format**. They cover malformed later records, bounded lookup, unchanged output on refusal, selected-provider identity and command failures before output. Native compilation, execution, SDK deployment and GUI rendering remain unverified.

For the shared rule behind these changes, continue with the [preparation and publication lesson](../../../../Umicom-Bits2Banking/content/courses/learning-build-systems/lessons/20-publish-toolchain-selection.html) in the ecosystem learning checkout. See also [compiler selection](COMPILER_SELECTION.md) and [Framework snapshot formatting](../../../framework/docs/development/compiler-snapshot-text.md).
