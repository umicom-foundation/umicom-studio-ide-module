/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/watcher.h
 *
 * PURPOSE:
 *   Control Studio's shared Framework watcher and expose watcher statistics to
 *   headless tools, tests and future GTK4 workbench surfaces.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_WATCHER_H
#define UMICOM_STUDIO_WATCHER_H

#include "umicom/studio/services.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Provide the studio watcher start operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_watcher_start(UmiStudioServices *services);
/**
 * Provide the studio watcher stop operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_watcher_stop(UmiStudioServices *services);
/**
 * Provide the studio watcher scan once operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_watcher_scan_once(UmiStudioServices *services);
/**
 * Provide the studio watcher stats operation used by this module and its client
 * applications.
 */
UmiWatcherStats umi_studio_watcher_stats(UmiStudioServices *services);

#ifdef __cplusplus
}
#endif

#endif
