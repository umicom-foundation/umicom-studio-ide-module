/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_workbench_document_contracts.c
 * PURPOSE:
 *   Compile the exact document boundary used by workbench_window.c. Missing
 *   types, declarations or incompatible callback signatures fail compilation.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
#include "umicom/ui/gtk4/document_commands.h"
#include <stdio.h>
#include <string.h>

/* _Generic inspects types without calling GTK or creating a native window. */
#define SIGNATURE(symbol, type) \
    _Static_assert(_Generic(&(symbol), type: 1, default: 0), #symbol " signature changed")
typedef UmiStatus (*CommandMap)(const char *, UmiDocumentEditCommand *);
typedef UmiStatus (*DocumentCommand)(UmiGtk4Adapter *, const char *);
typedef int (*Enabled)(UmiGtk4Adapter *, const char *);
typedef int (*HasCompletion)(UmiGtk4Adapter *);
typedef UmiStatus (*Prompt)(UmiGtk4Adapter *);
typedef UmiStatus (*Bind)(UmiGtk4Adapter *, UmiDocumentCoordinator *, UmiGtk4DocumentEditResultFn, void *);
typedef UmiStatus (*SaveAll)(UmiGtk4Adapter *, UmiGtk4DocumentSaveResultFn, void *);
typedef int (*SaveBusy)(const UmiGtk4Adapter *);
typedef UmiStatus (*SaveProgress)(const UmiGtk4Adapter *, UmiDocumentSaveProgress *);
typedef UmiStatus (*ProgressFormat)(const UmiDocumentSaveProgress *, char *, size_t);
SIGNATURE(UmiDocumentSaveProgressFormat, ProgressFormat);
SIGNATURE(UmiDocumentEditCommandFromId, CommandMap);
SIGNATURE(UmiGtk4AdapterDocumentCommand, DocumentCommand);
SIGNATURE(UmiGtk4AdapterDocumentNavigate, DocumentCommand);
SIGNATURE(UmiGtk4AdapterDocumentCommandEnabled, Enabled);
SIGNATURE(UmiGtk4AdapterDocumentHasCompletion, HasCompletion);
SIGNATURE(UmiGtk4AdapterPromptDocumentLocation, Prompt);
SIGNATURE(UmiGtk4AdapterBindDocumentEditing, Bind);
SIGNATURE(UmiGtk4AdapterDocumentSaveAll, SaveAll);
SIGNATURE(UmiGtk4AdapterCancelDocumentSaveAll, Prompt);
SIGNATURE(UmiGtk4AdapterDocumentSaveAllBusy, SaveBusy);
SIGNATURE(UmiGtk4AdapterDocumentSaveAllProgress, SaveProgress);

int main(void)
{
    UmiDocumentEditCommand command;
    if (UmiDocumentEditCommandFromId("edit.undo", &command) != UMI_STATUS_OK ||
        command != UMI_DOCUMENT_EDIT_UNDO) return 1;
    /* Retained previous completion message. The checks above only used the
     * type of the save formatter; this real call also verifies its definition.
     * GTK linkage is checked by Framework's separate native link consumer. */
    // puts("Studio document boundary: declarations and command mapping verified.");
    UmiDocumentSaveProgress progress = {0};
    progress.phase = UMI_DOCUMENT_SAVE_COMPLETE;
    progress.last_status = UMI_STATUS_OK;
    char message[128];
    if (UmiDocumentSaveProgressValidate(&progress) != UMI_STATUS_OK ||
        UmiDocumentSaveProgressFormat(&progress, message, sizeof(message)) != UMI_STATUS_OK ||
        strcmp(message, "Save All complete: 0 saved, 0 already saved.") != 0) return 1;
    puts("Studio document boundary: declarations, mapping and linked save formatter verified.");
    return 0;
}
