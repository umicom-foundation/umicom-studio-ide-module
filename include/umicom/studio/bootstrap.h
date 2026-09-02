/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/bootstrap.h
 *
 * PURPOSE:
 *   Own the Umicom Framework composition root shared by the console and GTK4
 *   frontends.  Studio services, commands and Slave Controllers are registered
 *   here, including access to retained diagnostics and runtime registries.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_BOOTSTRAP_H
#define UMICOM_STUDIO_BOOTSTRAP_H

#include <stddef.h>

#include "umicom/umicom.h"
#include "umicom/studio/services.h"
#include "umicom/studio/ui.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio bootstrap data shared with callers of this public contract.
 */
typedef struct UmiStudioBootstrap UmiStudioBootstrap;

/**
 * Initialise studio bootstrap from caller-provided values so later operations receive a
 * known state.
 */
UmiStatus umi_studio_bootstrap_create(UmiStudioBootstrap **out_bootstrap);
/**
 * Provide the studio bootstrap start operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_bootstrap_start(UmiStudioBootstrap *bootstrap);
/**
 * Provide the studio bootstrap stop operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_bootstrap_stop(UmiStudioBootstrap *bootstrap);
/**
 * Release or reset state held by studio bootstrap so the same storage can be reused
 * safely.
 */
void umi_studio_bootstrap_destroy(UmiStudioBootstrap *bootstrap);

/**
 * Provide the studio bootstrap application name operation used by this module and its
 * client applications.
 */
const char *umi_studio_bootstrap_application_name(
    const UmiStudioBootstrap *bootstrap
);

/**
 * Return the number of records represented by studio bootstrap module without changing
 * their state.
 */
size_t umi_studio_bootstrap_module_count(
    const UmiStudioBootstrap *bootstrap
);

/**
 * Provide the studio bootstrap services operation used by this module and its client
 * applications.
 */
UmiStudioServices *umi_studio_bootstrap_services(
    UmiStudioBootstrap *bootstrap
);

/**
 * Provide the studio bootstrap service registry operation used by this module and its
 * client applications.
 */
UmiServiceRegistry *umi_studio_bootstrap_service_registry(
    UmiStudioBootstrap *bootstrap
);
/**
 * Provide the studio bootstrap command registry operation used by this module and its
 * client applications.
 */
UmiCommandRegistry *umi_studio_bootstrap_command_registry(
    UmiStudioBootstrap *bootstrap
);
/**
 * Provide the studio bootstrap health registry operation used by this module and its
 * client applications.
 */
UmiHealthRegistry *umi_studio_bootstrap_health_registry(
    UmiStudioBootstrap *bootstrap
);
/**
 * Provide the studio bootstrap ui operation used by this module and its client
 * applications.
 */
UmiStudioUi *umi_studio_bootstrap_ui(UmiStudioBootstrap *bootstrap);
/**
 * Provide the studio bootstrap desktop runtime operation used by this module and its
 * client applications.
 */
UmiDesktopRuntime *umi_studio_bootstrap_desktop_runtime(
    UmiStudioBootstrap *bootstrap);
/**
 * Provide the studio bootstrap desktop shell operation used by this module and its client
 * applications.
 */
UmiDesktopShellModel *umi_studio_bootstrap_desktop_shell(
    UmiStudioBootstrap *bootstrap);
/**
 * Provide the studio bootstrap desktop content operation used by this module and its
 * client applications.
 */
UmiDesktopContentRuntime *umi_studio_bootstrap_desktop_content(
    UmiStudioBootstrap *bootstrap);
/**
 * Provide the studio bootstrap component host operation used by this module and its client
 * applications.
 */
UmiUiComponentHostService *umi_studio_bootstrap_component_host(
    UmiStudioBootstrap *bootstrap);
/**
 * Provide the studio bootstrap component drag drop operation used by this module and its
 * client applications.
 */
UmiDesktopComponentDragDrop *umi_studio_bootstrap_component_drag_drop(
    UmiStudioBootstrap *bootstrap);
/**
 * Provide the studio bootstrap context synchronizer operation used by this module and its
 * client applications.
 */
UmiDesktopContextSynchronizer *umi_studio_bootstrap_context_synchronizer(
    UmiStudioBootstrap *bootstrap);
/**
 * Provide the studio bootstrap layout history operation used by this module and its client
 * applications.
 */
UmiDesktopLayoutHistory *umi_studio_bootstrap_layout_history(
    UmiStudioBootstrap *bootstrap);
/**
 * Provide the studio bootstrap session recovery operation used by this module and its
 * client applications.
 */
UmiDesktopSessionRecovery *umi_studio_bootstrap_session_recovery(
    UmiStudioBootstrap *bootstrap);
/**
 * Provide the studio bootstrap monitor interaction operation used by this module and its
 * client applications.
 */
UmiDesktopMonitorInteraction *umi_studio_bootstrap_monitor_interaction(
    UmiStudioBootstrap *bootstrap);
#ifdef __cplusplus
}
#endif

#endif
