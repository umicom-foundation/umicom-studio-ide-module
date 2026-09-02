/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/workbench.h
 *
 * PURPOSE:
 *   Declare Studio workbench identity, default layout composition and reset
 *   operations without exposing GTK4 widget types.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_WORKBENCH_H
#define UMICOM_STUDIO_WORKBENCH_H

#include "umicom/umicom.h"
#include "umicom/studio/services.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UMI_STUDIO_WORKBENCH_ID "org.umicom.studio.workbench"
#define UMI_STUDIO_APPLICATION_ID "org.umicom.studio"
#define UMI_STUDIO_DEFAULT_PERSPECTIVE "studio.perspective.develop"

/**
 * Provide the studio workbench populate operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_workbench_populate(UmiUiWorkbench *workbench,
                                        UmiStudioServices *services);
/**
 * Provide the studio workbench reset layout operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_workbench_reset_layout(UmiUiWorkbench *workbench);
/**
 * Provide the studio workbench restore session operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_workbench_restore_session(UmiUiWorkbench *workbench,
                                               UmiSessionStore *session);
/**
 * Provide the studio workbench save session operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_workbench_save_session(UmiUiWorkbench *workbench,
                                            UmiSessionStore *session);

#ifdef __cplusplus
}
#endif

#endif
