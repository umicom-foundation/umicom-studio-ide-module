/*-----------------------------------------------------------------------------
 * Umicom Studio IDE Tests
 * File: tests/test_quality_documentation.c
 *
 * PURPOSE:
 *   Verify Studio scans source guidance through Framework, aggregates the
 *   report and presents it through the toolkit-neutral Quality Centre view.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include <assert.h>
#include <string.h>

#include "quality_centre_fixture.h"
#include "umicom/studio/quality_views.h"

/* Exercise the complete selected-file-to-Quality-Centre presentation journey. */
int main(void)
{
    UmiStudioQualityCentre *centre = umi_test_quality_centre();
    UmiCodeGuardDocumentationPolicy policy;
    UmiCodeGuardDocumentationReport report;
    UmiStudioQualitySnapshot snapshot;
    UmiStudioQualityView view;
    /* Framework owns thresholds and scanning; Studio only selects this source file. */
    umi_codeguard_documentation_policy_default(&policy);
    assert(umi_studio_quality_centre_review_documentation(
        centre, __FILE__, &policy, &report) == UMI_STATUS_OK);
    assert(umi_studio_quality_centre_snapshot(
        centre, &snapshot) == UMI_STATUS_OK);
    assert(snapshot.documentation_files == 1U);
    assert(snapshot.documentation_failures == 0U);
    assert(snapshot.documentation_function_percent == 100U);
    /* The view exposes stable navigation identity and learner-readable evidence. */
    assert(umi_studio_quality_view_build(
        centre,
        UMI_STUDIO_QUALITY_VIEW_DOCUMENTATION,
        &view) == UMI_STATUS_OK);
    assert(strcmp(view.id, "documentation") == 0);
    assert(view.item_count == 1U);
    assert(strstr(view.summary, "functions 100%") != NULL);
    umi_studio_quality_centre_destroy(centre);
    return 0;
}
