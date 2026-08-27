/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/production_commands.h
 *
 * PURPOSE:
 *   Project the Framework-owned application production control plane into this
 *   thin product without duplicating layout, readiness or acceptance logic.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_PRODUCTION_COMMANDS_H
#define UMICOM_STUDIO_PRODUCTION_COMMANDS_H
#include "umicom/studio/production_profile.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct UmiStudioProductionCommandContribution {
    const UmiApplicationProductionControlCommand *command;
    const char *menu_path;
    const char *view_id;
} UmiStudioProductionCommandContribution;
typedef struct UmiStudioProductionCommandCatalogue {
    UmiStudioProductionCommandContribution entries[32];
    size_t count;
    size_t mutating_count;
} UmiStudioProductionCommandCatalogue;
UmiStatus umi_studio_production_commands_build(
    UmiStudioProductionCommandCatalogue *out_catalogue);
#ifdef __cplusplus
}
#endif
#endif

