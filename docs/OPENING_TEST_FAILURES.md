# Open a file mentioned by a test failure

Studio keeps a test's retained results and output in Test Explorer. When a
message includes a file and line, the **Source locations** tab provides a
button that opens that location in the editor.

## Follow a failure into its source

1. Open **Test Explorer** and select the test you want to inspect.
2. Read **Selected test** first. It contains the retained result messages and
   output, with run labels and counts showing how much evidence is available.
3. Open **Source locations**. Each button shows a path, a line and a column,
   along with the run and whether the location came from a result or output.
   A column of zero means the start of the line.
4. If the path is relative, read the current test working directory printed
   below it. It must be an absolute directory. Old results may have come from a
   different checkout: verify that this directory is the one you want before
   opening. If it is unavailable, the button is disabled.
5. Activate the button. Studio opens the file and moves to the recorded line.
   If the file is already open, its unsaved text stays in the editor. The action
   does not run the test or save the document.

The existing **Source** button on a test row remains available. For a CTest
test, it normally opens the CMake statement that registered the test. Use
**Source locations** for the file mentioned by an assertion or diagnostic.

## Understand an empty list

An empty list does not mean a test passed. The message may have no filename,
use an unsupported format, or belong to evidence that is no longer retained.
For example, `line 28: capture != NULL` does not identify a source file, so
Studio leaves it as text instead of guessing a destination.

The tab displays counts for duplicate locations, unparsed lines and omissions
caused by capacity limits. It can show up to 256 locations from the retained
evidence. The **Selected test**, **Results**, **Coverage** and **Output** tabs
remain available. [Copy selected-test evidence](COPYING_TEST_EVIDENCE.md) explains
how to copy the retained evidence as a CSV report for review.

## Respond to a location that cannot be opened

- If the file moved or was deleted, Studio reports that the location was not
  reached. A failed file open does not move the cursor in your previous tab.
- If the file exists but its old line no longer exists, the source tab stays
  open. Read the current file and the original failure message together.
- If discovery, evidence or the selected test changes, refresh Test Explorer
  and choose its current location again. Old buttons do not act on a new test.
- If a relative path belongs to an older checkout, open that checkout's source
  explicitly through the file-opening workflow. This view does not rewrite
  historical paths or recover a run's original working directory.

Locations are historical clues, not proof that the current source still has
the same assertion on the same line. Build and run the relevant test again
after editing to obtain new evidence.
