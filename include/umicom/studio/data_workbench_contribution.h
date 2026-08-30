/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/data_workbench_contribution.h
 *
 * PURPOSE:
 *   Place Framework-owned Database, SQL and modelling workbench contracts in
 *   Studio without duplicating data services or database policy.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_DATA_WORKBENCH_CONTRIBUTION_H
#define UMICOM_STUDIO_DATA_WORKBENCH_CONTRIBUTION_H

#include <stddef.h>
#include <stdint.h>

#include "umicom/data/workbench/workbench.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UMI_STUDIO_DATA_WORKBENCH_CONTRIBUTION_API_VERSION 1U

typedef struct UmiStudioDataWorkbenchCommandContribution {
    uint32_t struct_size;
    uint32_t api_version;
    const char *framework_command_id;
    const char *menu_id;
    const char *menu_group;
    int32_t order;
    int show_in_command_centre;
} UmiStudioDataWorkbenchCommandContribution;

typedef struct UmiStudioDataWorkbenchViewContribution {
    uint32_t struct_size;
    uint32_t api_version;
    const char *view_id;
    const char *title;
    const char *framework_contract;
    const char *default_region;
    int32_t order;
    int closable;
    int movable;
} UmiStudioDataWorkbenchViewContribution;

size_t umi_studio_data_workbench_command_count(void);
const UmiStudioDataWorkbenchCommandContribution *
umi_studio_data_workbench_command_at(size_t index);
const UmiStudioDataWorkbenchCommandContribution *
umi_studio_data_workbench_command_find(const char *framework_command_id);
size_t umi_studio_data_workbench_view_count(void);
const UmiStudioDataWorkbenchViewContribution *
umi_studio_data_workbench_view_at(size_t index);
const UmiStudioDataWorkbenchViewContribution *
umi_studio_data_workbench_view_find(const char *view_id);
UmiStatus umi_studio_data_workbench_create(
    UmiDatabaseExplorer *explorer,
    UmiDataWorkbenchRuntime **out_runtime);

#ifdef __cplusplus
}
#endif

#endif /* UMICOM_STUDIO_DATA_WORKBENCH_CONTRIBUTION_H */
