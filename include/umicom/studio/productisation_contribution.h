/*-----------------------------------------------------------------------------
 * Umicom Studio IDE Module
 * File: include/umicom/studio/productisation_contribution.h
 *
 * PURPOSE:
 *   Declare this thin module's adoption of Framework-owned application
 *   experience, components, layouts and productisation evidence.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_PRODUCTISATION_CONTRIBUTION_H
#define UMICOM_STUDIO_PRODUCTISATION_CONTRIBUTION_H

#include "umicom/application/productisation/session.h"
#include "umicom/application/productisation/workspace_guide.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Return Studio's immutable identity and availability contribution. */
const UmiProductApplicationAdoption *
umi_studio_productisation_contribution(void);
/** Rebuild Studio readiness from the current Framework catalogues. */
UmiStatus umi_studio_productisation_snapshot(
    UmiProductApplicationAdoptionSnapshot *out_snapshot);
/** Initialise a Framework-owned product session for Studio. */
UmiStatus umi_studio_product_session_init(
    UmiProductApplicationSession *out_session);
/** Build welcome-screen workspace choices from the canonical Framework profile. */
UmiStatus umi_studio_product_workspace_guide(
    UmiProductWorkspaceGuide *out_guide);

#ifdef __cplusplus
}
#endif

#endif
