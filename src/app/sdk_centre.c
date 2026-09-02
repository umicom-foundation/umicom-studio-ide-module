/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/sdk_centre.c
 *
 * PURPOSE:
 *   Present installed Framework SDK exports to Studio project consumers.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/sdk_centre.h"
#include <string.h>

/*
 * Provide the studio sdk centre refresh operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_sdk_centre_refresh(UmiStudioSdkCentre *centre)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    centre->consumer_cmake[0] = '\0';
    status = umi_sdk_export_plan_validate(&centre->export_plan);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        centre->ready = 0;
        return status;
    }
    status = umi_sdk_consumer_package_render_cmake(
        &centre->export_plan.package, &centre->export_plan.catalogue,
        centre->consumer_cmake, sizeof(centre->consumer_cmake));
    centre->ready = status == UMI_STATUS_OK;
    return status;
}

/*
 * Initialise studio sdk centre from caller-provided values so later operations receive a
 * known state.
 */
UmiStatus umi_studio_sdk_centre_init(UmiStudioSdkCentre *centre,
                                         const char *install_prefix)
{
    static const struct {
        const char *component;
        const char *target;
    } exports[] = {
        {"base", "Umicom::base"},
        {"runtime", "Umicom::runtime"},
        {"ui", "Umicom::ui"},
        {"compiler", "Umicom::compiler"},
        {"delivery", "Umicom::delivery"},
        {"sdk", "Umicom::sdk"},
        {"product", "Umicom::product"},
        {"distribution", "Umicom::distribution"}
    };
    UmiStatus status;
    size_t index;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL || install_prefix == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    (void)memset(centre, 0, sizeof(*centre));
    status = umi_sdk_export_plan_init(&centre->export_plan, install_prefix,
                                          "0.9");
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < sizeof(exports) / sizeof(exports[0]); ++index) {
        status = umi_sdk_export_plan_add_target(
            &centre->export_plan, exports[index].component,
            exports[index].target, 1);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;
    }
    return umi_studio_sdk_centre_refresh(centre);
}

/*
 * Provide the studio sdk centre consumer cmake operation used by this module and its
 * client applications.
 */
const char *umi_studio_sdk_centre_consumer_cmake(
    const UmiStudioSdkCentre *centre)
{
    return centre != NULL && centre->ready ? centre->consumer_cmake : NULL;
}
