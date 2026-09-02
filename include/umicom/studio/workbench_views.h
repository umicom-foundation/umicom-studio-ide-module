/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/workbench_views.h
 *
 * PURPOSE:
 *   Register Studio-specific workbench view factories against the reusable
 *   Framework view-factory and presentation contracts.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * Studio decides what data belongs in Explorer, Source Control, Testing,
 * Designer, Applications, Framework and AI views. Framework owns the view-model
 * contract and frontend adapters decide how those models are drawn.
 */

#ifndef UMICOM_STUDIO_WORKBENCH_VIEWS_H
#define UMICOM_STUDIO_WORKBENCH_VIEWS_H

#include <stddef.h>

#include "umicom/studio/services.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Add studio workbench views only after its inputs and available capacity have been
 * checked.
 */
UmiStatus umi_studio_workbench_views_register(
    UmiUiWorkbench *workbench,
    UmiStudioServices *services
);
/**
 * Provide the studio workbench views register registry operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_workbench_views_register_registry(
    UmiUiViewFactoryRegistry *registry,
    UmiStudioServices *services
);

/**
 * Return the number of records represented by studio workbench view definition without
 * changing their state.
 */
size_t umi_studio_workbench_view_definition_count(void);

#ifdef __cplusplus
}
#endif

#endif
