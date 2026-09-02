/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/quality_navigation.c
 *
 * PURPOSE:
 *   Implement the quality navigation behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE | Quality source navigation | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/studio/quality_navigation.h"
#include "umicom/studio/quality_evidence.h"
#include <stdio.h>
#include <string.h>
/*
 * Provide the studio quality navigation resolve operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_quality_navigation_resolve(const UmiStudioQualityCentre *centre,const char *evidence_id,UmiStudioQualityNavigationTarget *out_target)
{
    const UmiCodeGuardEvidence *evidence;
    int length;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || evidence_id == NULL || out_target == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    evidence = umi_studio_quality_find_evidence(centre,evidence_id);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (evidence == NULL || evidence->path[0] == '\0') return UMI_STATUS_NOT_FOUND;
    (void)memset(out_target,0,sizeof(*out_target));
    (void)umi_codeguard_quality_copy(out_target->evidence_id,sizeof(out_target->evidence_id),evidence->id);
    (void)umi_codeguard_quality_copy(out_target->path,sizeof(out_target->path),evidence->path);
    out_target->line = evidence->line == 0U ? 1U : evidence->line;
    out_target->column = 1U;
    length = snprintf(out_target->label,sizeof(out_target->label),"%s at %s:%zu",evidence->summary,evidence->path,out_target->line);
    return length < 0 || (size_t)length >= sizeof(out_target->label) ? UMI_STATUS_CAPACITY_EXCEEDED : UMI_STATUS_OK;
}
