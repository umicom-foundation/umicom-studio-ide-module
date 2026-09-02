/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/retrieval_main.c
 *
 * PURPOSE:
 *   Demonstrate local Framework retrieval from a small Studio document.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This native command exercises the same Framework-backed service that future Studio GUI views will call.
 */

#include "umicom/studio/ai_retrieval.h"

#include <stdio.h>

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(int argc, char **argv)
{
    UmiAiRetrievalResult results[4];
    const char *query = argc > 1 ? argv[1] : "Framework";
    const char *text =
        "Umicom Framework owns reusable platform services. "
        "Umicom Studio IDE consumes Framework services. "
        "Umicom Trader and Umicom TMS will reuse the same contracts.";
    size_t count = umi_studio_ai_retrieve("studio-reference", text, query,
                                          results, 4U);
    size_t index;
    printf("Results: %zu\n", count);
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < count; ++index) {
        printf("[%zu] score=%.3f chunk=%s text=%s\n",
               index + 1U, results[index].score,
               results[index].chunk.chunk_id,
               results[index].chunk.text);
    }
    return count > 0U ? 0 : 1;
}
