# Review Replace All before changing a document

Replace All in Studio's editor opens a comparison before it changes your text.
It works on the current document, including unsaved typing. It does not search
or replace every file in a project.

1. Open the document you want to edit and select its tab.
2. In the editor toolbar, enter the text in **Find** and its replacement in
   **Replace with**. Leave the replacement empty to delete the matches.
3. Choose **Replace All**. The review names the document, counts the matches
   and shows the complete current draft beside the proposed result. Both
   panes are read-only. Use the comparison's change navigation to inspect the
   edits, or select and copy text from either pane.
4. Choose **Cancel** to keep the draft unchanged. Cancel is the default action
   when the question opens. Choose **Apply replacements** only after reviewing
   the proposed text.
5. Inspect the edited document. **Undo** restores the draft as it was in the
   review. **Save** writes the edited document to its file separately.

For a small example, put `note one` and `note two` on separate lines. Find
`note` and replace it with `entry`. The review should report two matches and
show `entry one` and `entry two`. Cancel leaves both original lines in place.
Apply changes the draft; Undo restores the original lines.

## Understand what is matched

A lowercase search ignores ASCII letter case: `note` also matches `NOTE`.
A search containing an uppercase ASCII letter, such as `Note`, requests exact
case. Non-ASCII text compares exactly. Matches do not overlap. This toolbar
uses literal text, so `$1`, `[abc]` and backslashes have no special meaning.
It is not a symbol rename tool and does not parse C code.

If there are no matches, Studio tells you and keeps the draft. If the
replacement would produce identical text, it also keeps the draft and does
not create another Undo step. Single-occurrence Replace keeps its existing
immediate behavior.

## If the document changes during review

The comparison is a snapshot, meaning a copy of the text at the moment you
opened it. Typing, saving, renaming or reloading while a question is open can
make that question stale. Studio refuses the old proposal and asks you to
repeat the action. Review the new comparison before applying it.

Changing the Find or Replace fields cannot alter a question that is already
open. Cancel it and choose Replace All again to review different terms. If a
tab changes programmatically while a question is open, acceptance still
targets the named document. A closed document cannot be replaced by a newly
opened document with the same name.

This feature uses the existing editor limit of 8 MiB per draft and needs
memory for both complete versions. An oversized replacement is refused.
Studio does not save as part of Apply. The existing saved-file conflict checks
still apply when you later use Save. Keep a saved copy of important work;
Undo history is bounded and belongs to the open document session.
