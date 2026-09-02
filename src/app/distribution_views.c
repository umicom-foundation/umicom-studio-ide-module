/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/distribution_views.c
 *
 * PURPOSE:
 *   Catalogue the views shown by Studio's SDK and Distribution Centre.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/distribution_views.h"
#include <string.h>

/*
 * Initialise studio distribution views from caller-provided values so later operations
 * receive a known state.
 */
void umi_studio_distribution_views_init(UmiStudioDistributionViews *views)
{
    static const UmiStudioDistributionView defaults[] = {
        {"sdk.exports", "SDK Exports", "left"},
        {"packages.manifest", "Package Manifest", "centre"},
        {"packages.dependencies", "Runtime Dependencies", "bottom"},
        {"packages.bundle", "Bundle Designer", "centre"},
        {"installers", "Installer Centre", "centre"},
        {"supply-chain", "Supply Chain", "bottom"},
        {"release.channels", "Release Channels", "right"},
        {"updates", "Updates and Rollback", "right"}
    };
    size_t index;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (views == NULL) return;
    (void)memset(views, 0, sizeof(*views));
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < sizeof(defaults) / sizeof(defaults[0]); ++index) {
        views->views[index] = defaults[index];
    }
    views->count = sizeof(defaults) / sizeof(defaults[0]);
}

/*
 * Find studio distribution views while leaving the underlying catalogue or model owned by
 * this module.
 */
const UmiStudioDistributionView *umi_studio_distribution_views_find(
    const UmiStudioDistributionViews *views,
    const char *view_id)
{
    size_t index;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (views == NULL || view_id == NULL) return NULL;
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < views->count; ++index) {
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (strcmp(views->views[index].view_id, view_id) == 0) {
            return &views->views[index];
        }
    }
    return NULL;
}
