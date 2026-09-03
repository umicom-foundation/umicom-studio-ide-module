/*-----------------------------------------------------------------------------
 * Umicom Studio IDE Tests
 * File: tests/test_developer_language_intelligence_centre.c
 *
 * PURPOSE:
 *   Verify Studio activates the Framework language service, offline C
 *   completion providers and trusted quick documentation as one thin product
 *   composition.
 *
 * AUTHOR AND ORGANISATION:
 *   Sammy Hegab
 *   Umicom Foundation
 *
 * LICENCE:
 *   MIT
 *---------------------------------------------------------------------------*/
#include <assert.h>
#include <string.h>

#include "umicom/studio/language_intelligence_centre.h"

/* Start the centre and exercise a real completion request without opening GTK. */
int main(void)
{
    UmiStudioLanguageIntelligenceCentre *centre = NULL;
    UmiStudioLanguageIntelligenceCentreSnapshot snapshot;
    UmiEditorCompletionOrchestrationSnapshot completion_snapshot;
    UmiEditorCompletionRequest request;
    UmiEditorCompletionRankingConfig ranking;

    assert(umi_studio_language_intelligence_centre_create(&centre) ==
           UMI_STATUS_OK);
    assert(umi_studio_language_intelligence_centre_snapshot(
               centre, &snapshot) == UMI_STATUS_OK);
    assert(snapshot.available);
    assert(snapshot.offline_c_assistance_available);
    assert(snapshot.completion_providers.provider_count == 2U);
    assert(snapshot.quick_documentation.section_count >= 20U);
    assert(snapshot.signatures.signature_count == 10U);
    assert(umi_studio_language_intelligence_centre_signatures(centre) != NULL);

    request = umi_editor_completion_request_default("lesson.c", "c", 1U);
    (void)memcpy(request.prefix, "memm", sizeof("memm"));
    request.cursor_offset = 4U;
    ranking = umi_editor_completion_ranking_config_default();
    assert(umi_editor_completion_orchestration_begin(
               umi_studio_language_intelligence_centre_completion(centre),
               &request, &ranking) == UMI_STATUS_OK);
    assert(umi_editor_completion_orchestration_snapshot(
               umi_studio_language_intelligence_centre_completion(centre),
               &completion_snapshot) == UMI_STATUS_OK);
    assert(completion_snapshot.result_count == 1U);

    umi_studio_language_intelligence_centre_destroy(centre);
    return 0;
}
