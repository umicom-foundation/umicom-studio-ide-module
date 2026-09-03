/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/language_intelligence_centre.c
 *
 * PURPOSE:
 *   Implement the Language Intelligence Centre composition.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/language_intelligence_centre.h"

#include <stdlib.h>
#include <string.h>

#include "umicom/editor/providers/c_language_assistance.h"

struct UmiStudioLanguageIntelligenceCentre {
    UmiLanguageService *service;
    UmiEditorCLanguageAssistance *c_assistance;
    uint64_t revision;
};

/* Copy display text into a bounded snapshot without allowing a missing source
 * to escape as an invalid C string. */
static void copy_text(char *destination,
                      size_t capacity,
                      const char *source)
{
    size_t length;

    if (destination == NULL || capacity == 0U) return;
    if (source == NULL) source = "";
    length = strlen(source);
    if (length >= capacity) length = capacity - 1U;
    if (length > 0U) (void)memcpy(destination, source, length);
    destination[length] = '\0';
}
/*
 * Initialise studio language intelligence centre from caller-provided values so later
 * operations receive a known state.
 */
UmiStatus umi_studio_language_intelligence_centre_create(
    UmiStudioLanguageIntelligenceCentre **out_centre)
{
    UmiStudioLanguageIntelligenceCentre *centre;
    UmiStatus status;

    if (out_centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    *out_centre = NULL;
    centre = (UmiStudioLanguageIntelligenceCentre *)calloc(
        1U, sizeof(*centre));
    if (centre == NULL) return UMI_STATUS_OUT_OF_MEMORY;

    status = umi_language_service_create(&centre->service);
    if (status == UMI_STATUS_OK) {
        status = umi_editor_c_language_assistance_create(
            &centre->c_assistance);
    }
    /* One cleanup path prevents partially created services from leaking. */
    if (status != UMI_STATUS_OK) {
        umi_studio_language_intelligence_centre_destroy(centre);
        return status;
    }
    centre->revision = 1U;
    *out_centre = centre;
    return UMI_STATUS_OK;
}
/*
 * Release or reset state held by studio language intelligence centre so the same storage
 * can be reused safely.
 */
void umi_studio_language_intelligence_centre_destroy(
    UmiStudioLanguageIntelligenceCentre *centre)
{
    if (centre == NULL) return;
    umi_editor_c_language_assistance_destroy(centre->c_assistance);
    umi_language_service_destroy(centre->service);
    free(centre);
}
/*
 * Provide the studio language intelligence centre snapshot operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_language_intelligence_centre_snapshot(
    UmiStudioLanguageIntelligenceCentre *centre,
    UmiStudioLanguageIntelligenceCentreSnapshot *out_snapshot)
{
    UmiEditorCLanguageAssistanceSnapshot assistance_snapshot;
    UmiStatus status;

    if (centre == NULL || out_snapshot == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    (void)memset(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->struct_size = (uint32_t)sizeof(*out_snapshot);
    out_snapshot->api_version = 2U;
    copy_text(out_snapshot->area_id, sizeof(out_snapshot->area_id),
              "studio.language-intelligence-centre");
    copy_text(out_snapshot->title, sizeof(out_snapshot->title),
              "Language Intelligence Centre");
    copy_text(
        out_snapshot->summary, sizeof(out_snapshot->summary),
        "Completion, signatures, documentation, symbols, navigation, diagnostics and refactoring from interchangeable providers.");

    status = umi_language_service_snapshot(
        centre->service, &out_snapshot->service);
    if (status == UMI_STATUS_OK) {
        status = umi_editor_c_language_assistance_snapshot(
            centre->c_assistance, &assistance_snapshot);
    }
    if (status != UMI_STATUS_OK) return status;

    out_snapshot->completion_providers =
        assistance_snapshot.completion_providers;
    out_snapshot->quick_documentation = assistance_snapshot.documentation;
    out_snapshot->signatures = assistance_snapshot.signatures;

    out_snapshot->revision = centre->revision;
    out_snapshot->offline_c_assistance_available =
        out_snapshot->completion_providers.provider_count >= 2U &&
        out_snapshot->quick_documentation.section_count > 0U &&
        out_snapshot->signatures.signature_count > 0U;
    out_snapshot->available = 1;
    return UMI_STATUS_OK;
}
/*
 * Provide the studio language intelligence centre service operation used by this module
 * and its client applications.
 */
UmiLanguageService *umi_studio_language_intelligence_centre_service(
    UmiStudioLanguageIntelligenceCentre *centre)
{
    return centre != NULL ? centre->service : NULL;
}

/* Expose the caller-owned registry so plugins can contribute providers. */
UmiEditorCompletionProviderRegistry *
umi_studio_language_intelligence_centre_completion_providers(
    UmiStudioLanguageIntelligenceCentre *centre)
{
    return centre != NULL
        ? umi_editor_c_language_assistance_providers(centre->c_assistance)
        : NULL;
}

/* Expose the shared completion coordinator without transferring ownership. */
UmiEditorCompletionOrchestration *
umi_studio_language_intelligence_centre_completion(
    UmiStudioLanguageIntelligenceCentre *centre)
{
    return centre != NULL
        ? umi_editor_c_language_assistance_completion(centre->c_assistance)
        : NULL;
}

/* Expose trusted quick documentation without transferring ownership. */
UmiEditorQuickDocumentation *
umi_studio_language_intelligence_centre_documentation(
    UmiStudioLanguageIntelligenceCentre *centre)
{
    return centre != NULL
        ? umi_editor_c_language_assistance_documentation(centre->c_assistance)
        : NULL;
}

/* Expose callable signatures without transferring ownership to a view. */
UmiEditorSignatureHelpModel *
umi_studio_language_intelligence_centre_signatures(
    UmiStudioLanguageIntelligenceCentre *centre)
{
    return centre != NULL
        ? umi_editor_c_language_assistance_signatures(centre->c_assistance)
        : NULL;
}
