/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/watcher.c
 *
 * PURPOSE:
 *   Provide Studio-level lifecycle and inspection functions for the shared
 *   Framework workspace watcher.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/watcher.h"

/*
 * Provide the studio watcher start operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_watcher_start(UmiStudioServices *services)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    return umi_watcher_start(umi_studio_services_watcher(services));
}

/*
 * Provide the studio watcher stop operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_watcher_stop(UmiStudioServices *services)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    return umi_watcher_stop(umi_studio_services_watcher(services));
}

/*
 * Provide the studio watcher scan once operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_watcher_scan_once(UmiStudioServices *services)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    return umi_watcher_scan_once(umi_studio_services_watcher(services));
}

/*
 * Provide the studio watcher stats operation used by this module and its client
 * applications.
 */
UmiWatcherStats umi_studio_watcher_stats(UmiStudioServices *services)
{
    return services != NULL
        ? umi_watcher_stats(umi_studio_services_watcher(services))
        : (UmiWatcherStats){0};
}
