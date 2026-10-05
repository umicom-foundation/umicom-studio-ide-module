/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_build_log.c
 * PURPOSE: Verify one-job log selection, unchanged drafts on refusal and startup isolation.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/build.h"
#include "umicom/platform/filesystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); return 1; } } while (0)
/* No job is accepted in this fixture, so no compiler or worker needs to run. */
int main(void)
{
    UmiStudioBuildService *service = NULL; UmiClock clock = umi_clock_system();
    UmiStudioBuildLogState *state = calloc(1U, sizeof(*state)); CHECK(state != NULL);
    CHECK(umi_studio_build_service_create(".", &clock, &service) == UMI_STATUS_OK);
    CHECK(UmiStudioBuildReadLog(service, state) == UMI_STATUS_OK && state->next_path[0] == '\0' && !state->captured.enabled);
    char root[UMI_PATH_CAPACITY], path[UMI_PATH_CAPACITY];
    CHECK(umi_fs_current_directory(root, sizeof(root)) == UMI_STATUS_OK);
    CHECK(umi_path_join(root, "selected-build.log", path, sizeof(path)) == UMI_STATUS_OK);
    CHECK(UmiStudioBuildArmLog(service, path) == UMI_STATUS_OK);
    CHECK(UmiStudioBuildReadLog(service, state) == UMI_STATUS_OK && strcmp(state->next_path, path) == 0);
    CHECK(UmiStudioBuildArmLog(service, "relative.log") == UMI_STATUS_INVALID_ARGUMENT);
    CHECK(UmiStudioBuildSubmit(service, UMI_BUILD_PHASE_BUILD, 0) == UMI_STATUS_PERMISSION_DENIED);
    CHECK(UmiStudioBuildReadLog(service, state) == UMI_STATUS_OK && strcmp(state->next_path, path) == 0 && !state->captured.enabled);
    CHECK(!UmiStudioBuildBusy(service));
    CHECK(UmiStudioBuildArmLog(service, NULL) == UMI_STATUS_OK);
    CHECK(UmiStudioBuildReadLog(service, state) == UMI_STATUS_OK && state->next_path[0] == '\0');
    CHECK(UmiStudioBuildArmLog(NULL, path) == UMI_STATUS_INVALID_ARGUMENT);
    CHECK(UmiStudioBuildReadLog(service, NULL) == UMI_STATUS_INVALID_ARGUMENT);
    CHECK(UmiStudioBuildReadLog(NULL, state) == UMI_STATUS_INVALID_ARGUMENT);
    umi_studio_build_service_destroy(service); umi_clock_dispose(&clock); free(state); return 0;
}
