# Copy selected-test evidence

Test evidence records what the test service retained from previous runs. A CSV
copy lets you inspect the selected test's results and output without launching
another test.

1. Open **Test Explorer** and select a test using its **Select** action.
2. Open the **Selected test** details tab. Confirm the test identity and review
   its retained and matching counts.
3. Select **Copy selected evidence CSV**. Studio captures the selected test's
   currently retained evidence and offers it to the clipboard.
4. Paste into a text editor and save as UTF-8 with a `.csv` extension. For a
   spreadsheet, import with comma separators and quoted fields.
5. Read the `record` column: `evidence-summary` describes the capture, `result`
   rows contain outcomes and diagnostic details, and `output` rows contain
   captured stdout/stderr text. Columns that do not apply to a row are empty.

The selection matches the exact test ID, so a similarly named test is not
included. The native action includes all retained sessions for that test; an
empty session-filter field describes this scope. Results are newest first,
followed by output records newest first. At most 32 results and 64 output records
are retained in one capture. Matching counts disclose when older records were
omitted. These counts describe the service's retained data, not every test run
that has ever occurred.

A header and summary without results means there is no retained result for the
selection. It does not mean the test passed. Skipped, cancelled and timed-out
outcomes stay distinct. Copying does not discover, build, run or debug a test.

Potential spreadsheet formula text receives a leading apostrophe. Quotation
marks and line breaks remain inside their original cells. If evidence contains
invalid UTF-8 or terminal control bytes, the export is rejected instead of
silently changing diagnostics. The notification explains the failure and the
clipboard remains unchanged; use the original Output view to review that data.
Import IDs as text so a spreadsheet does not reinterpret them.
