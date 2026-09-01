/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/welcome_centre.h
 *
 * PURPOSE:
 *   Define Studio welcome/start centre for projects, recent workspaces and learning actions.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * Studio composes reusable Framework capabilities here. Generic models and
 * engines stay in Framework so the same mechanisms can serve future products.
 */
#ifndef APPLICATIONS_STUDIO_INCLUDE_UMICOM_STUDIO_WELCOME_CENTRE_H
#define APPLICATIONS_STUDIO_INCLUDE_UMICOM_STUDIO_WELCOME_CENTRE_H
#include <stddef.h>
#include <stdint.h>
#include "umicom/umicom.h"
#include "umicom/studio/services.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct UmiStudioWelcomeCentreSnapshot {
    uint32_t struct_size;
    uint32_t api_version;
    char area_id[128];
    char view_type[128];
    char title[256];
    char summary[512];
    /* The recommended layout comes from the canonical Framework experience. */
    char recommended_layout_id[128];
    uint64_t revision;
    size_t item_count;
    /* Each choice represents one user-selectable Framework layout recipe. */
    size_t workspace_choice_count;
    /* Readiness reports maturity without claiming visible means complete. */
    unsigned readiness_percent;
    /* Product readiness is copied from Framework adoption evidence. */
    int product_ready;
    int available;
} UmiStudioWelcomeCentreSnapshot;
/* Build the Studio first-screen model from shared Framework catalogues. */
UmiStatus umi_studio_welcome_centre_snapshot(
    UmiStudioServices *services,
    UmiStudioWelcomeCentreSnapshot *out_snapshot);
#ifdef __cplusplus
}
#endif
#endif
