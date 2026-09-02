/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/knowledge_views.h
 * PURPOSE: Expose Studio view factories over Framework Knowledge Centre views.
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Studio supplies view IDs while Framework supplies view data. */
#ifndef UMICOM_STUDIO_KNOWLEDGE_VIEWS_H
#define UMICOM_STUDIO_KNOWLEDGE_VIEWS_H

#include "umicom/studio/knowledge_centre.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Provide the studio knowledge overview view operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_knowledge_overview_view(
    UmiStudioAiPlatform *platform, const char *view_id,
    UmiUiViewModel **out_view);
/**
 * Provide the studio knowledge collections view operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_knowledge_collections_view(
    UmiStudioAiPlatform *platform, const char *view_id,
    UmiUiViewModel **out_view);
/**
 * Provide the studio knowledge sources view operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_knowledge_sources_view(
    UmiStudioAiPlatform *platform, const char *view_id,
    UmiUiViewModel **out_view);
/**
 * Provide the studio knowledge search view operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_knowledge_search_view(
    UmiStudioAiPlatform *platform, const char *view_id,
    const char *query_text, UmiUiViewModel **out_view);
/**
 * Provide the studio knowledge source view operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_knowledge_source_view(
    const char *view_id, const UmiKnowledgeMatch *match,
    UmiUiViewModel **out_view);

#ifdef __cplusplus
}
#endif
#endif
