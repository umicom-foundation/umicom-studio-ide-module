/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/integration_designer_commands.c
 *
 * PURPOSE:
 *   Implement the integration designer commands behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE | Integration Designer commands | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/studio/integration_designer_commands.h"
#include "umicom/studio/integration_designer_api.h"
#include "umicom/studio/integration_designer_connections.h"
#include "umicom/studio/integration_designer_mapping.h"
#include "umicom/studio/integration_designer_workflow.h"

/* Provide the set text operation used by this module and its client applications. */
static void set_text(char *destination,size_t capacity,const char *source)
{
    (void)umi_integration_designer_copy(destination,capacity,source);
}

/*
 * Provide the studio integration designer seed sample operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_integration_designer_seed_sample(UmiStudioIntegrationDesignerCentre *centre)
{
    UmiIntegrationConnectionProfile profile = {0};
    UmiIntegrationSchemaDescriptor schema = {0};
    UmiIntegrationApiContract contract;
    UmiIntegrationApiOperation operation = {0};
    UmiIntegrationFieldMapping mapping;
    UmiIntegrationFieldRule rule = {0};
    UmiIntegrationWorkflow workflow;
    UmiIntegrationWorkflowNode trigger = {0};
    UmiIntegrationWorkflowNode request = {0};
    UmiIntegrationWorkflowNode response = {0};
    UmiIntegrationWorkflowEdge first = {0};
    UmiIntegrationWorkflowEdge second = {0};
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    set_text(profile.id,sizeof(profile.id),"orders-api"); set_text(profile.name,sizeof(profile.name),"Orders API");
    profile.transport = UMI_INTEGRATION_DESIGNER_REST; set_text(profile.base_uri,sizeof(profile.base_uri),"https://api.example.test");
    profile.auth = UMI_INTEGRATION_DESIGNER_AUTH_API_KEY; set_text(profile.secret_reference,sizeof(profile.secret_reference),"vault://integration/orders-key");
    profile.timeout_ms = 5000U; profile.enabled = true;
    status = umi_studio_integration_designer_add_connection(centre,&profile); /* Preserve the original failure result so the caller can respond to the correct cause. */ if (status != UMI_STATUS_OK) return status;
    set_text(schema.id,sizeof(schema.id),"order-v1"); schema.kind = UMI_INTEGRATION_DESIGNER_JSON_SCHEMA; schema.major = 1U; set_text(schema.source_uri,sizeof(schema.source_uri),"workspace://schemas/order.json");
    status = umi_studio_integration_designer_add_schema(centre,&schema); /* Preserve the original failure result so the caller can respond to the correct cause. */ if (status != UMI_STATUS_OK) return status;
    status = umi_integration_api_contract_init(&contract,"orders","Orders","1.0","orders-api"); /* Preserve the original failure result so the caller can respond to the correct cause. */ if (status != UMI_STATUS_OK) return status;
    set_text(operation.id,sizeof(operation.id),"create-order"); set_text(operation.method,sizeof(operation.method),"POST"); set_text(operation.path,sizeof(operation.path),"/orders"); set_text(operation.request_schema_id,sizeof(operation.request_schema_id),"order-v1"); set_text(operation.response_schema_id,sizeof(operation.response_schema_id),"order-v1");
    status = umi_integration_api_contract_add_operation(&contract,&operation); /* Preserve the original failure result so the caller can respond to the correct cause. */ if (status != UMI_STATUS_OK) return status;
    status = umi_studio_integration_designer_add_contract(centre,&contract); /* Preserve the original failure result so the caller can respond to the correct cause. */ if (status != UMI_STATUS_OK) return status;
    status = umi_integration_field_mapping_init(&mapping,"order-map","order-v1","order-v1"); /* Preserve the original failure result so the caller can respond to the correct cause. */ if (status != UMI_STATUS_OK) return status;
    set_text(rule.source,sizeof(rule.source),"order_id"); set_text(rule.target,sizeof(rule.target),"orderId"); rule.transform = UMI_INTEGRATION_DESIGNER_COPY; rule.required = true;
    status = umi_integration_field_mapping_add(&mapping,&rule); /* Preserve the original failure result so the caller can respond to the correct cause. */ if (status != UMI_STATUS_OK) return status;
    status = umi_studio_integration_designer_add_mapping(centre,&mapping); /* Preserve the original failure result so the caller can respond to the correct cause. */ if (status != UMI_STATUS_OK) return status;
    status = umi_integration_workflow_init(&workflow,"order-sync","Order sync"); /* Preserve the original failure result so the caller can respond to the correct cause. */ if (status != UMI_STATUS_OK) return status;
    set_text(trigger.id,sizeof(trigger.id),"trigger"); set_text(trigger.name,sizeof(trigger.name),"On order"); trigger.kind = UMI_INTEGRATION_DESIGNER_TRIGGER;
    set_text(request.id,sizeof(request.id),"request"); set_text(request.name,sizeof(request.name),"Create order"); set_text(request.reference_id,sizeof(request.reference_id),"create-order"); request.kind = UMI_INTEGRATION_DESIGNER_REQUEST;
    set_text(response.id,sizeof(response.id),"response"); set_text(response.name,sizeof(response.name),"Return result"); response.kind = UMI_INTEGRATION_DESIGNER_RESPONSE;
    status = umi_integration_workflow_add_node(&workflow,&trigger); /* Preserve the original failure result so the caller can respond to the correct cause. */ if (status == UMI_STATUS_OK) status = umi_integration_workflow_add_node(&workflow,&request); /* Preserve the original failure result so the caller can respond to the correct cause. */ if (status == UMI_STATUS_OK) status = umi_integration_workflow_add_node(&workflow,&response); /* Preserve the original failure result so the caller can respond to the correct cause. */ if (status != UMI_STATUS_OK) return status;
    set_text(first.from,sizeof(first.from),"trigger"); set_text(first.to,sizeof(first.to),"request"); set_text(second.from,sizeof(second.from),"request"); set_text(second.to,sizeof(second.to),"response");
    status = umi_integration_workflow_connect(&workflow,&first); /* Preserve the original failure result so the caller can respond to the correct cause. */ if (status == UMI_STATUS_OK) status = umi_integration_workflow_connect(&workflow,&second); /* Preserve the original failure result so the caller can respond to the correct cause. */ if (status != UMI_STATUS_OK) return status;
    return umi_studio_integration_designer_add_workflow(centre,&workflow);
}

/*
 * Perform studio integration designer through the module contract so client applications
 * do not duplicate its policy.
 */
UmiStatus umi_studio_integration_designer_execute(UmiStudioIntegrationDesignerCentre *centre,UmiStudioIntegrationDesignerCommand command,UmiIntegrationSimulation *out_simulation)
{
    UmiIntegrationDesignerValidation validation = {0};
    /* Select the behaviour associated with the requested command or state value. */
    switch (command) {
        case UMI_STUDIO_INTEGRATION_COMMAND_SEED_SAMPLE: return umi_studio_integration_designer_seed_sample(centre);
        case UMI_STUDIO_INTEGRATION_COMMAND_VALIDATE: return umi_studio_integration_designer_validate_workflow(centre,"order-sync",&validation);
        case UMI_STUDIO_INTEGRATION_COMMAND_SIMULATE:
            /*
             * Protect caller-owned memory by checking that required state is available before it is
             * used.
             */
            if (out_simulation == NULL) return UMI_STATUS_INVALID_ARGUMENT;
            return umi_studio_integration_designer_simulate(centre,"order-sync",out_simulation);
        default: return UMI_STATUS_NOT_FOUND;
    }
}
