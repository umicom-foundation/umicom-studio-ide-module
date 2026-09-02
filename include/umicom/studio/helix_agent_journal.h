/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/helix_agent_journal.h
 * PURPOSE: Inspect Helix v2 plan, action and evidence records in Studio.
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* The reporting helper formats copies of journal entries; it
 * never provides a mutation path into Framework's append-only record. */
#ifndef UMICOM_STUDIO_HELIX_AGENT_JOURNAL_H
#define UMICOM_STUDIO_HELIX_AGENT_JOURNAL_H

#include "umicom/studio/helix_agent_centre.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Return the number of records represented by studio helix agent journal without changing
 * their state.
 */
size_t umi_studio_helix_agent_journal_count(
    UmiStudioHelixAgentCentre *centre);
/**
 * Check that studio helix agent journal satisfies its contract before another service
 * relies on it.
 */
int umi_studio_helix_agent_journal_valid(
    UmiStudioHelixAgentCentre *centre);
/**
 * Provide the studio helix agent journal entry operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_helix_agent_journal_entry(
    UmiStudioHelixAgentCentre *centre,
    size_t index,
    char *out_text,
    size_t capacity);

#ifdef __cplusplus
}
#endif

#endif
