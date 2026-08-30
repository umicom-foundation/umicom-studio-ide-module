<!--
Umicom Studio IDE
File: docs/EXPERIENCE_CATALOGUE_INCLUDE_ORDER.md

PURPOSE:
Explain the Studio workspace-layout compilation failure and its Framework fix.

AUTHOR AND ORGANISATION:
Sammy Hegab
Umicom Foundation

LICENCE:
MIT
-->

# Experience Catalogue Include-Order Repair

Studio's `workspace_layouts.c` needs the Framework application definition for
`org.umicom.studio`. It calls
`umi_application_experience_catalogue_find()` and projects one of that
definition's layouts into the Studio workbench.

The source already included the correct declaration header. The compiler error
was caused by a different Framework header using the same include guard. An
earlier indirect include activated that guard, so the preprocessor skipped the
declaration later in the file.

The Framework fix gives each catalogue family a unique guard. Studio now uses
the combined `experience_portfolio.h` include to make its dependency explicit.

Two Framework regression tests cover:

- the previously failing include order;
- the combined portfolio header and both catalogue functions.

The public-header governance audit also scans the complete Studio include tree.
This means a future duplicated guard is reported as a clear header-contract
problem instead of appearing later as an unrelated implicit function call.
