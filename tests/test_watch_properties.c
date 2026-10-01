/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_watch_properties.c
 * PURPOSE: Verify product watch editing does not launch or evaluate a program.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/debugger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%d: %s\n", __LINE__, #x); exit(1); } } while (0)
#define OK(x) CHECK((x) == UMI_STATUS_OK)
int main(int argc, char **argv)
{
    CHECK(argc == 2); const char *name = argv[1]; UmiStudioDebuggerService *service = NULL;
    OK(umi_studio_debugger_service_create(&service)); char id[128];
    OK(umi_studio_debugger_service_add_watch(service, "savedNotes", id, sizeof id));
    UmiDebugWorkspace *workspace = umi_studio_debugger_service_workspace(service);
    UmiDebugWatchEdit *edit = NULL; OK(UmiDebugWatchEditCapture(workspace, 0U, &edit));
    UmiDebugWatchSettings settings; OK(UmiDebugWatchSettingsInit(&settings, 1, "savedNotes + 1"));
    UmiDebugWatchChange change; UmiDebugWatchSnapshot before, after;
    OK(umi_debug_workspace_watch_at(workspace, 0U, &before));
    if (strcmp(name, "evaluate-idle") == 0 || strcmp(name, "evaluate-memory") == 0) {
        if (strcmp(name, "evaluate-memory") == 0) OK(umi_studio_debugger_service_initialize(service, "memory", NULL));
        CHECK(UmiStudioDebuggerEvaluateWatch(service, edit) == UMI_STATUS_INVALID_STATE);
        OK(umi_debug_workspace_watch_at(workspace, 0, &after)); CHECK(after.revision == before.revision);
    } else if (strcmp(name, "stale") == 0) {
        OK(umi_studio_debugger_service_add_watch(service, "other", id, sizeof id));
        CHECK(UmiStudioDebuggerEditWatch(service, edit, &settings, 0, &change) == UMI_STATUS_BUSY && !change.changed);
    } else if (strcmp(name, "remove") == 0) {
        OK(UmiStudioDebuggerEditWatch(service, edit, NULL, 1, &change));
        CHECK(change.removed && umi_debug_workspace_watch_at(workspace, 0U, &after) == UMI_STATUS_NOT_FOUND);
    } else {
        CHECK(strcmp(name, "edit") == 0);
        OK(UmiStudioDebuggerEditWatch(service, edit, &settings, 0, &change));
        OK(umi_debug_workspace_watch_at(workspace, 0U, &after));
        CHECK(change.changed && !after.valid && strcmp(after.expression, settings.expression) == 0);
    }
    CHECK(!UmiStudioDebuggerNativeBusy(service));
    UmiDebugWatchEditDestroy(edit); umi_studio_debugger_service_destroy(service); return 0;
}
