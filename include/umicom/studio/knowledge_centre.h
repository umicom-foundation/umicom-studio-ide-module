/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/knowledge_centre.h
 * PURPOSE: Compose Framework Native RAG into Studio source and search workflows.
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * Studio chooses product defaults and user-facing inputs.  Embedding, vector
 * indexing, incremental refresh and citations remain Framework services.
 */
#ifndef UMICOM_STUDIO_KNOWLEDGE_CENTRE_H
#define UMICOM_STUDIO_KNOWLEDGE_CENTRE_H

#include "umicom/studio/ai_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio knowledge source input data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioKnowledgeSourceInput {
    char collection_id[UMI_KNOWLEDGE_ID_CAPACITY];
    char source_id[UMI_KNOWLEDGE_ID_CAPACITY];
    char title[UMI_KNOWLEDGE_NAME_CAPACITY];
    char uri[UMI_KNOWLEDGE_URI_CAPACITY];
    char language[UMI_KNOWLEDGE_LANGUAGE_CAPACITY];
    char media_type[UMI_KNOWLEDGE_MEDIA_TYPE_CAPACITY];
    UmiKnowledgeSourceKind kind;
    UmiAiDataClassification classification;
    uint64_t modified_ns;
} UmiStudioKnowledgeSourceInput;

/**
 * Provide the studio knowledge centre add collection operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_knowledge_centre_add_collection(
    UmiStudioAiPlatform *platform,
    const char *collection_id,
    const char *display_name,
    const char *description);
/**
 * Provide the studio knowledge centre ingest text operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_knowledge_centre_ingest_text(
    UmiStudioAiPlatform *platform,
    const UmiStudioKnowledgeSourceInput *input,
    const char *text,
    UmiKnowledgeIngestionReport *out_report);
/**
 * Provide the studio knowledge centre search operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_knowledge_centre_search(
    UmiStudioAiPlatform *platform,
    const char *query_text,
    const UmiKnowledgeFilter *filter,
    UmiKnowledgeMatch *matches,
    size_t capacity,
    size_t *out_count);
/**
 * Write studio knowledge centre in its stable representation and report capacity or input
 * failures to the caller.
 */
UmiStatus umi_studio_knowledge_centre_save(
    UmiStudioAiPlatform *platform,
    const char *path);
/**
 * Read studio knowledge centre into validated module state and return a status when input
 * cannot be used.
 */
UmiStatus umi_studio_knowledge_centre_load(
    UmiStudioAiPlatform *platform,
    const char *path);

#ifdef __cplusplus
}
#endif
#endif
