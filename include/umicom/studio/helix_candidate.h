/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/helix_candidate.h
 *
 * PURPOSE:
 *   Create isolated Studio Helix candidate metadata.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* BEGINNER NOTE:
 * Candidate metadata names an isolated workspace so future Builder agents do not modify the active Studio checkout in place.
 */

#ifndef APPLICATIONS_STUDIO_INCLUDE_UMICOM_STUDIO_HELIX_CANDIDATE_H
#define APPLICATIONS_STUDIO_INCLUDE_UMICOM_STUDIO_HELIX_CANDIDATE_H

#include "umicom/umicom.h"

#ifdef __cplusplus
extern "C" {
#endif

UmiStatus umi_studio_helix_candidate(const char *candidate_id,
                                     const char *plan_id,
                                     const char *workspace,
                                     UmiHelixCandidate *candidate);

#ifdef __cplusplus
}
#endif

#endif
