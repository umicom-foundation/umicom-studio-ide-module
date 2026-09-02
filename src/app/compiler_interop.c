/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/compiler_interop.c
 *
 * PURPOSE:
 *   Implement the compiler interop behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE | Polyglot interoperability view v2 | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/studio/compiler_interop.h"
#include <stdio.h>
#include <string.h>
/* Provide the find node operation used by this module and its client applications. */
static const UmiInteropNode *find_node(const UmiInteropGraph *graph,const char *unit_id) { size_t index; /* Visit each bounded item once so every record receives the same rule. */ for (index = 0U; index < graph->node_count; ++index) /* Keep the operation inside its valid bounds before reading, writing or adding data. */ if (strcmp(graph->nodes[index].unit_id,unit_id) == 0) return &graph->nodes[index]; return NULL; }
/*
 * Provide the studio compiler interop summarise operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_compiler_interop_summarise(const UmiInteropGraph *graph,UmiStudioCompilerInteropSummary *out_summary)
{
    size_t index; char reason[UMI_COMPILER_TEXT_CAPACITY]; UmiStatus graph_status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (graph == NULL || out_summary == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    (void)memset(out_summary,0,sizeof(*out_summary));
    out_summary->units = graph->node_count;
    out_summary->boundaries = graph->edge_count;
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < graph->edge_count; ++index) { const UmiInteropNode *producer = find_node(graph,graph->edges[index].producer_id); const UmiInteropNode *consumer = find_node(graph,graph->edges[index].consumer_id); /* Protect caller-owned memory by checking that required state is available before it is used. */ if (graph->edges[index].required) out_summary->required_boundaries += 1U; /* Protect caller-owned memory by checking that required state is available before it is used. */ if (producer != NULL && consumer != NULL && umi_compiler_abi_compatible(&producer->abi,&consumer->abi)) out_summary->stable_boundaries += 1U; /* Use this fallback path when the earlier condition does not apply. */ else out_summary->incompatible_boundaries += 1U; }
    graph_status = umi_interop_graph_validate(graph,reason,sizeof(reason)); out_summary->buildable = graph_status == UMI_STATUS_OK && out_summary->incompatible_boundaries == 0U; (void)snprintf(out_summary->headline,sizeof(out_summary->headline),"%zu unit(s), %zu ABI boundary/boundaries, %zu incompatible",out_summary->units,out_summary->boundaries,out_summary->incompatible_boundaries); return UMI_STATUS_OK;
}
