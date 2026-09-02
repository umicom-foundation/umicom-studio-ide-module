/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/quality_duplicates.c
 *
 * PURPOSE:
 *   Implement the quality duplicates behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE | Duplicate review evidence | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/studio/quality_duplicates.h"
/*
 * Add studio quality duplicate only after its inputs and available capacity have been
 * checked.
 */
UmiStatus umi_studio_quality_duplicate_add(UmiStudioQualityCentre *centre,const UmiCodeGuardDuplicateFile *first,const UmiCodeGuardDuplicateFile *second)
{
    UmiCodeGuardDuplicateCandidate candidate;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_codeguard_duplicate_candidate_build(first,second,&candidate);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = umi_codeguard_duplicate_review_add(&centre->session.duplicates,&candidate);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) { centre->session.revision += 1U; centre->revision += 1U; }
    return status;
}
/*
 * Provide the studio quality duplicate decide operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_quality_duplicate_decide(UmiStudioQualityCentre *centre,size_t index,UmiCodeGuardCandidateDisposition disposition)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_codeguard_duplicate_review_disposition(&centre->session.duplicates,index,disposition);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) { centre->session.revision += 1U; centre->revision += 1U; }
    return status;
}
/*
 * Find studio quality duplicate while leaving the underlying catalogue or model owned by
 * this module.
 */
const UmiCodeGuardDuplicateCandidate *umi_studio_quality_duplicate_at(const UmiStudioQualityCentre *centre,size_t index)
{
    return centre != NULL && index < centre->session.duplicates.count ? &centre->session.duplicates.items[index] : NULL;
}
