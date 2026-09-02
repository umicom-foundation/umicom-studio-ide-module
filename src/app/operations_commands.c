/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/operations_commands.c
 *
 * PURPOSE:
 *   Implement the operations commands behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE | Operations commands v2 | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/studio/operations_commands.h"
/*
 * Provide the studio operations seed operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_operations_seed(UmiStudioOperationsCentre *centre)
{
    UmiOperationsCentre *operations = umi_studio_operations_centre_engine(centre);
    UmiOperationsResourceSample resource_a = {"studio",100U,21.5,268435456U,14U,28U,3U};
    UmiOperationsResourceSample resource_b = {"studio",200U,34.0,285212672U,16U,31U,1U};
    UmiOperationsBenchmarkRun baseline = {"workspace-restore","umicom-studio","run-56",100U,14000000U,71.0,true};
    UmiOperationsBenchmarkRun current = {"workspace-restore","umicom-studio","run-57",200U,11000000U,90.0,true};
    UmiOperationsCrashEvidence crash = {"crash-studio-1","umicom-studio","frontend-preview","Renderer exited","reports/crash-studio-1",300U,5701U,false};
    UmiOperationsEvidenceCounts evidence = {24U,8U,15U,6U,36U,300U};
    UmiOperationsHealthSummary health = {4U,3U,1U,0U,5U,0U,UMI_OPERATIONS_HEALTH_DEGRADED};
    UmiOperationsResilienceTelemetry resilience = {5U,4U,1U,0U,2U,UMI_CIRCUIT_CLOSED,120U,3U};
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (operations == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_operations_centre_record_resource(operations,&resource_a); /* Preserve the original failure result so the caller can respond to the correct cause. */ if (status != UMI_STATUS_OK) return status;
    status = umi_operations_centre_record_resource(operations,&resource_b); /* Preserve the original failure result so the caller can respond to the correct cause. */ if (status != UMI_STATUS_OK) return status;
    status = umi_operations_centre_record_benchmark(operations,&baseline); /* Preserve the original failure result so the caller can respond to the correct cause. */ if (status != UMI_STATUS_OK) return status;
    status = umi_operations_centre_record_benchmark(operations,&current); /* Preserve the original failure result so the caller can respond to the correct cause. */ if (status != UMI_STATUS_OK) return status;
    status = umi_operations_centre_record_crash(operations,&crash); /* Preserve the original failure result so the caller can respond to the correct cause. */ if (status != UMI_STATUS_OK) return status;
    status = umi_operations_centre_update_evidence(operations,&evidence); /* Preserve the original failure result so the caller can respond to the correct cause. */ if (status != UMI_STATUS_OK) return status;
    status = umi_operations_centre_update_health(operations,&health); /* Preserve the original failure result so the caller can respond to the correct cause. */ if (status != UMI_STATUS_OK) return status;
    return umi_operations_centre_update_resilience(operations,&resilience);
}
/*
 * Perform studio operations through the module contract so client applications do not
 * duplicate its policy.
 */
UmiStatus umi_studio_operations_execute(UmiStudioOperationsCentre *centre,UmiStudioOperationsCommand command,const char *argument)
{
    UmiOperationsCentre *operations = umi_studio_operations_centre_engine(centre);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    /* Select the behaviour associated with the requested command or state value. */
    switch (command) {
        case UMI_STUDIO_OPERATIONS_COMMAND_SEED: return umi_studio_operations_seed(centre);
        case UMI_STUDIO_OPERATIONS_COMMAND_MARK_CRASH_RECOVERED:
            return argument == NULL || operations == NULL ? UMI_STATUS_INVALID_ARGUMENT : umi_operations_crash_registry_mark_recovered(&operations->crashes,argument);
        case UMI_STUDIO_OPERATIONS_COMMAND_OPEN_PROFILER: return umi_studio_operations_centre_activate(centre,"profiler");
        default: return UMI_STATUS_NOT_IMPLEMENTED;
    }
}
