/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_source_control_offline.c
 *
 * PURPOSE:
 *   Check explicit discovery-off source control without invoking a tool,
 *   inspecting a repository or treating missing evidence as a clean state.
 *
 * AUTHOR AND ORGANISATION:
 *   Sammy Hegab
 *   Umicom Foundation
 *
 * LICENCE:
 *   MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/source_control.h"
#include "umicom/studio/services.h"

#include <stdio.h>
#include <string.h>

/* Keep assertions and cleanup active in every build configuration. */
#define REQUIRE(expression) do { \
    if (!(expression)) { \
        (void)fprintf(stderr, "Line %d: %s\n", __LINE__, #expression); \
        failed = 1; \
        goto cleanup; \
    } \
} while (0)

/* The synthetic path need not exist: disabled discovery is a pure model
 * constructor, and all source-control operations must fail as unavailable. */
int main(void)
{
    UmiStudioSourceControlService *service = NULL;
    UmiStudioSourceControlSnapshot initial;
    UmiStudioSourceControlSnapshot after;
    UmiStudioServicesOptions defaults = umi_studio_services_options_default();
    int failed = 0;

    REQUIRE(defaults.discover_repository == 1);
    REQUIRE(umi_studio_source_control_service_create_with_discovery(
        "umicom-offline-fixture", 0, &service) == UMI_STATUS_OK);
    REQUIRE(umi_studio_source_control_service_snapshot(service, &initial) == UMI_STATUS_OK);
    REQUIRE(!initial.available);
    REQUIRE(strcmp(initial.provider_id, "umicom.vcs.unavailable") == 0);
    REQUIRE(initial.branch[0] == '\0');
    REQUIRE(umi_studio_source_control_service_refresh(service, 10U) == UMI_STATUS_UNAVAILABLE);
    REQUIRE(umi_studio_source_control_service_stage(service, "not-a-real-file.c") ==
        UMI_STATUS_UNAVAILABLE);
    REQUIRE(umi_studio_source_control_service_unstage_all(service) == UMI_STATUS_UNAVAILABLE);
    REQUIRE(umi_studio_source_control_service_push(service) == UMI_STATUS_UNAVAILABLE);
    REQUIRE(umi_studio_source_control_service_snapshot(service, &after) == UMI_STATUS_OK);
    REQUIRE(!after.available && after.revision == initial.revision);
    REQUIRE(after.operations == initial.operations);

cleanup:
    umi_studio_source_control_service_destroy(service);
    return failed;
}
