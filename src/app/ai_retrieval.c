/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/ai_retrieval.c
 *
 * PURPOSE:
 *   Chunk a Studio document and rank relevant pieces through Framework retrieval.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * Studio delegates chunking and ranking to Framework instead of owning a second retrieval implementation.
 */

#include "umicom/studio/ai_retrieval.h"

/*
 * Provide the studio ai retrieve operation used by this module and its client
 * applications.
 */
size_t umi_studio_ai_retrieve(const char *document_id,
                              const char *document_text,
                              const char *query,
                              UmiAiRetrievalResult *results,
                              size_t capacity)
{
    UmiAiChunk chunks[32];
    size_t count;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (document_id == NULL || document_text == NULL || query == NULL ||
        results == NULL || capacity == 0U) {
        return 0U;
    }
    count = umi_ai_chunk_text(document_id, document_text, 512U,
                              chunks, sizeof(chunks) / sizeof(chunks[0]));
    return umi_ai_retrieval_rank(query, chunks, count, results, capacity);
}
