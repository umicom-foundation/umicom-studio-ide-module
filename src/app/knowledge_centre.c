/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/knowledge_centre.c
 * PURPOSE: Implement Studio collection, ingestion, search and archive adapters.
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/knowledge_centre.h"

#include <stdio.h>
#include <string.h>

/*
 * Provide the studio knowledge centre add collection operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_knowledge_centre_add_collection(
    UmiStudioAiPlatform *platform,
    const char *collection_id,
    const char *display_name,
    const char *description)
{
    UmiKnowledgeCollection collection;
    UmiKnowledgeService *service =
        umi_studio_ai_platform_knowledge(platform);
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_knowledge_collection_init(
        &collection, collection_id, display_name, description);
    return status == UMI_STATUS_OK
        ? umi_knowledge_service_add_collection(service, &collection) : status;
}

/*
 * Provide the studio knowledge centre ingest text operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_knowledge_centre_ingest_text(
    UmiStudioAiPlatform *platform,
    const UmiStudioKnowledgeSourceInput *input,
    const char *text,
    UmiKnowledgeIngestionReport *out_report)
{
    UmiKnowledgeService *service =
        umi_studio_ai_platform_knowledge(platform);
    UmiKnowledgeSource source;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL || input == NULL || text == NULL ||
        out_report == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_knowledge_source_init(
        &source, input->source_id, input->collection_id, input->title,
        input->uri, input->kind);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    /* Apply this branch only when its contract condition is satisfied. */
    if (snprintf(source.language, sizeof(source.language), "%s",
                 input->language[0] != '\0' ? input->language : "und") < 0 ||
        snprintf(source.media_type, sizeof(source.media_type), "%s",
                 input->media_type[0] != '\0'
                    ? input->media_type : "text/plain") < 0) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }
    source.classification = input->classification != 0
        ? input->classification : UMI_AI_DATA_INTERNAL;
    source.modified_ns = input->modified_ns;
    return umi_knowledge_service_ingest_text(
        service, &source, text, out_report);
}

/*
 * Provide the studio knowledge centre search operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_knowledge_centre_search(
    UmiStudioAiPlatform *platform,
    const char *query_text,
    const UmiKnowledgeFilter *filter,
    UmiKnowledgeMatch *matches,
    size_t capacity,
    size_t *out_count)
{
    UmiKnowledgeService *service =
        umi_studio_ai_platform_knowledge(platform);
    UmiKnowledgeQuery query;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_knowledge_query_init(&query, query_text);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (filter != NULL) query.filter = *filter;
    query.limit = umi_studio_ai_platform_knowledge_result_limit(platform);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (query.limit > capacity) query.limit = capacity;
    return umi_knowledge_service_query(
        service, &query, matches, capacity, out_count);
}

/*
 * Write studio knowledge centre in its stable representation and report capacity or input
 * failures to the caller.
 */
UmiStatus umi_studio_knowledge_centre_save(
    UmiStudioAiPlatform *platform,
    const char *path)
{
    UmiKnowledgeService *service =
        umi_studio_ai_platform_knowledge(platform);
    const char *selected = path != NULL && path[0] != '\0'
        ? path : umi_studio_ai_platform_knowledge_archive_path(platform);
    return service != NULL
        ? umi_knowledge_archive_save(service, selected)
        : UMI_STATUS_INVALID_ARGUMENT;
}

/*
 * Read studio knowledge centre into validated module state and return a status when input
 * cannot be used.
 */
UmiStatus umi_studio_knowledge_centre_load(
    UmiStudioAiPlatform *platform,
    const char *path)
{
    UmiKnowledgeService *service =
        umi_studio_ai_platform_knowledge(platform);
    const char *selected = path != NULL && path[0] != '\0'
        ? path : umi_studio_ai_platform_knowledge_archive_path(platform);
    return service != NULL
        ? umi_knowledge_archive_load(service, selected)
        : UMI_STATUS_INVALID_ARGUMENT;
}
