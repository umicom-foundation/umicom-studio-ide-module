/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/operations_workspace_fixture.h
 *
 * PURPOSE:
 *   Verify the operations workspace fixture behaviour and report a clear failure
 *   when its contract changes.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* Umicom Studio IDE Tests | Batch 57 fixtures | Sammy Hegab | Umicom Foundation | MIT */
#ifndef UMICOM_STUDIO_TEST_OPERATIONS_WORKSPACE_FIXTURE_H
#define UMICOM_STUDIO_TEST_OPERATIONS_WORKSPACE_FIXTURE_H
#include <assert.h>
#include "umicom/studio/operations_commands.h"
#include "umicom/studio/workspace_commands.h"
/**
 * Exercise test batch57 services and return a clear result when the behaviour no longer
 * matches its contract.
 */
static inline UmiStudioServices *umi_test_batch57_services(void)
{
    static int services_token;
    return (UmiStudioServices *)(void *)&services_token;
}
/**
 * Exercise test operations centre and return a clear result when the behaviour no longer
 * matches its contract.
 */
static inline UmiStudioOperationsCentre *umi_test_operations_centre(void)
{
    UmiStudioOperationsCentre *centre = NULL;
    assert(umi_studio_operations_centre_create(umi_test_batch57_services(),&centre) == UMI_STATUS_OK);
    return centre;
}
/**
 * Exercise test seeded operations centre and return a clear result when the behaviour no
 * longer matches its contract.
 */
static inline UmiStudioOperationsCentre *umi_test_seeded_operations_centre(void)
{
    UmiStudioOperationsCentre *centre = umi_test_operations_centre();
    assert(umi_studio_operations_seed(centre) == UMI_STATUS_OK);
    return centre;
}
/**
 * Exercise test professional workspace and return a clear result when the behaviour no
 * longer matches its contract.
 */
static inline UmiStudioProfessionalWorkspace *umi_test_professional_workspace(void)
{
    UmiStudioProfessionalWorkspace *workspace = NULL;
    assert(umi_studio_professional_workspace_create(umi_test_batch57_services(),&workspace) == UMI_STATUS_OK);
    return workspace;
}
/**
 * Exercise test seeded professional workspace and return a clear result when the behaviour
 * no longer matches its contract.
 */
static inline UmiStudioProfessionalWorkspace *umi_test_seeded_professional_workspace(void)
{
    UmiStudioProfessionalWorkspace *workspace = umi_test_professional_workspace();
    assert(umi_studio_workspace_seed(workspace) == UMI_STATUS_OK);
    return workspace;
}
#endif
