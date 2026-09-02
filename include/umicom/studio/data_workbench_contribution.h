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

/**
 * Represent the studio data workbench command contribution data shared with callers of
 * this public contract.
 */
typedef struct UmiStudioDataWorkbenchCommandContribution {
    uint32_t struct_size;
    uint32_t api_version;
    const char *framework_command_id;
    const char *menu_id;
    const char *menu_group;
    int32_t order;
    int show_in_command_centre;
} UmiStudioDataWorkbenchCommandContribution;

/**
 * Represent the studio data workbench view contribution data shared with callers of this
 * public contract.
 */
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

/**
 * Return the number of records represented by studio data workbench command without
 * changing their state.
 */
size_t umi_studio_data_workbench_command_count(void);
/**
 * Find studio data workbench command while leaving the underlying catalogue or model owned
 * by this module.
 */
const UmiStudioDataWorkbenchCommandContribution *
umi_studio_data_workbench_command_at(size_t index);
/**
 * Find studio data workbench command while leaving the underlying catalogue or model owned
 * by this module.
 */
const UmiStudioDataWorkbenchCommandContribution *
umi_studio_data_workbench_command_find(const char *framework_command_id);
/**
 * Return the number of records represented by studio data workbench view without changing
 * their state.
 */
size_t umi_studio_data_workbench_view_count(void);
/**
 * Find studio data workbench view while leaving the underlying catalogue or model owned by
 * this module.
 */
const UmiStudioDataWorkbenchViewContribution *
umi_studio_data_workbench_view_at(size_t index);
/**
 * Find studio data workbench view while leaving the underlying catalogue or model owned by
 * this module.
 */
const UmiStudioDataWorkbenchViewContribution *
umi_studio_data_workbench_view_find(const char *view_id);
/**
 * Initialise studio data workbench from caller-provided values so later operations receive
 * a known state.
 */
UmiStatus umi_studio_data_workbench_create(
    UmiDatabaseExplorer *explorer,
    UmiDataWorkbenchRuntime **out_runtime);

#ifdef __cplusplus
}
#endif

#endif /* UMICOM_STUDIO_DATA_WORKBENCH_CONTRIBUTION_H */
