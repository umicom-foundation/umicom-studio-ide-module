/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/helix_agent_journal.c
 * PURPOSE: Implement read-only Studio access to the Helix v2 journal.
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/helix_agent_journal.h"

#include <inttypes.h>
#include <stdio.h>

/*
 * Return the number of records represented by studio helix agent journal without changing
 * their state.
 */
size_t umi_studio_helix_agent_journal_count(
    UmiStudioHelixAgentCentre *centre)
{
    UmiHelixOrchestrator *runtime =
        umi_studio_helix_agent_centre_runtime(centre);
    return runtime != NULL ? runtime->journal.count : 0U;
}

/*
 * Check that studio helix agent journal satisfies its contract before another service
 * relies on it.
 */
int umi_studio_helix_agent_journal_valid(UmiStudioHelixAgentCentre *centre)
{
    UmiHelixOrchestrator *runtime =
        umi_studio_helix_agent_centre_runtime(centre);
    return runtime != NULL && umi_helix_journal_verify(&runtime->journal);
}

/*
 * Provide the studio helix agent journal entry operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_helix_agent_journal_entry(
    UmiStudioHelixAgentCentre *centre,
    size_t index,
    char *out_text,
    size_t capacity)
{
    UmiHelixOrchestrator *runtime =
        umi_studio_helix_agent_centre_runtime(centre);
    const UmiHelixJournalEntry *entry;
    int written;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || out_text == NULL || capacity == 0U) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    entry = umi_helix_journal_at(&runtime->journal, index);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (entry == NULL) return UMI_STATUS_NOT_FOUND;
    written = snprintf(out_text, capacity, "#%" PRIu64 " kind=%d %s",
                       entry->sequence, (int)entry->kind, entry->summary);
    return written >= 0 && (size_t)written < capacity
        ? UMI_STATUS_OK : UMI_STATUS_CAPACITY_EXCEEDED;
}
