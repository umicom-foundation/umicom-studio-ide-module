/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/plugins.h
 *
 * PURPOSE:
 *   Provide Studio plug-in discovery and reporting as a thin product adapter
 *   over the Framework plug-in host.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_PLUGINS_H
#define UMICOM_STUDIO_PLUGINS_H

#include "umicom/studio/services.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio plugin report data shared with callers of this public contract.
 */
typedef struct UmiStudioPluginReport {
    size_t registered;
    size_t enabled;
    size_t failed;
    size_t contributions;
    size_t files_examined;
    size_t manifests_loaded;
    size_t invalid_manifests;
    size_t extension_points;
    size_t catalogue_entries;
    size_t audit_events;
    size_t permission_grants;
    char default_isolation[64];
} UmiStudioPluginReport;

/**
 * Provide the studio plugins discover operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_plugins_discover(UmiStudioServices *services,
                                      const char *root,
                                      int recursive,
                                      UmiStudioPluginReport *out_report);
/**
 * Provide the studio plugins report operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_plugins_report(UmiStudioServices *services,
                                    UmiStudioPluginReport *out_report);

#ifdef __cplusplus
}
#endif

#endif
