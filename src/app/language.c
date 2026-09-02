/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/language.c
 *
 * PURPOSE:
 *   Implement Studio language-server protocol state and document synchronisation.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/language.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct UmiStudioLanguageService {
    UmiProtocolTransport *transport;
    UmiProtocolClient *client;
    UmiLspClient lsp;
    UmiLspDocumentRegistry *documents;
    UmiLanguageService *model;
    UmiLanguageClientSession session;
};

/*
 * Initialise studio language service from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_language_service_create(
    const char *root_uri,
    UmiStudioLanguageService **out_service)
{
    UmiStudioLanguageService *service;
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (root_uri == NULL || out_service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    *out_service = NULL;
    service = (UmiStudioLanguageService *)calloc(1U, sizeof(*service));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    status = umi_protocol_transport_create_memory(256U, &service->transport);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_protocol_client_create(service->transport,
                                            &service->client);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = umi_protocol_client_start(service->client);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_lsp_client_init(&service->lsp, service->client, root_uri);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_lsp_document_registry_create(&service->documents);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_language_service_create(&service->model);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_language_client_session_init(&service->session,
                                                  "studio.primary",
                                                  "studio.language-server",
                                                  root_uri);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        umi_studio_language_service_destroy(service);
        return status;
    }
    *out_service = service;
    return UMI_STATUS_OK;
}

/*
 * Release or reset state held by studio language service so the same storage can be reused
 * safely.
 */
void umi_studio_language_service_destroy(UmiStudioLanguageService *service)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) return;
    umi_language_service_destroy(service->model);
    umi_lsp_document_registry_destroy(service->documents);
    umi_protocol_client_destroy(service->client);
    umi_protocol_transport_destroy(service->transport);
    free(service);
}

/*
 * Initialise studio language service from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_language_service_initialize(
    UmiStudioLanguageService *service,
    int64_t process_id,
    int64_t *out_request_id)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_language_client_session_transition(
        &service->session, UMI_LANGUAGE_CLIENT_STARTING);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_language_client_session_transition(
            &service->session, UMI_LANGUAGE_CLIENT_INITIALIZING);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_lsp_initialize(&service->lsp, process_id, out_request_id);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = umi_lsp_initialized(&service->lsp);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        umi_language_client_session_record_request(&service->session);
        status = umi_language_client_session_transition(
            &service->session, UMI_LANGUAGE_CLIENT_READY);
    }
    return status;
}

/*
 * Provide the studio language service open operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_language_service_open(
    UmiStudioLanguageService *service,
    const char *uri,
    const char *language_id,
    int version,
    const char *text)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_lsp_document_registry_open(service->documents,
                                            uri, language_id, version);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_lsp_did_open(&service->lsp,
                                  uri, language_id, version, text);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        UmiLanguageDocumentSnapshot document = {0};
        (void)snprintf(document.id, sizeof(document.id), "%s", uri);
        (void)snprintf(document.uri, sizeof(document.uri), "%s", uri);
        (void)snprintf(document.language_id, sizeof(document.language_id),
                       "%s", language_id);
        document.version = (uint64_t)version;
        document.open = 1;
        status = umi_language_document_registry_upsert(
            umi_language_service_document(service->model), &document);
    }
    return status;
}

/*
 * Provide the studio language service change operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_language_service_change(
    UmiStudioLanguageService *service,
    const char *uri,
    int version,
    const char *text)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_lsp_document_registry_change(service->documents, uri, version);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_lsp_did_change(&service->lsp, uri, version, text);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        UmiLanguageDocumentSnapshot document;
        /* Apply this branch only when its contract condition is satisfied. */
        if (umi_language_document_registry_find(
                umi_language_service_document(service->model), uri,
                &document) == UMI_STATUS_OK) {
            document.version = (uint64_t)version;
            document.dirty = 1;
            status = umi_language_document_registry_upsert(
                umi_language_service_document(service->model), &document);
        }
    }
    return status;
}

/*
 * Provide the studio language service completion operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_language_service_completion(
    UmiStudioLanguageService *service,
    const char *uri,
    UmiLspPosition position,
    int64_t *out_request_id)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    {
        UmiStatus status = umi_lsp_completion(&service->lsp, uri, position,
                                              out_request_id);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) {
            umi_language_client_session_record_request(&service->session);
        }
        return status;
    }
}

/*
 * Provide the studio language service hover operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_language_service_hover(UmiStudioLanguageService *service,
    const char *uri, UmiLspPosition position, int64_t *out_request_id)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_lsp_hover(&service->lsp, uri, position, out_request_id);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) umi_language_client_session_record_request(&service->session);
    return status;
}

/*
 * Provide the studio language service definition operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_language_service_definition(UmiStudioLanguageService *service,
    const char *uri, UmiLspPosition position, int64_t *out_request_id)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_lsp_definition(&service->lsp, uri, position, out_request_id);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) umi_language_client_session_record_request(&service->session);
    return status;
}

/*
 * Provide the studio language service references operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_language_service_references(UmiStudioLanguageService *service,
    const char *uri, UmiLspPosition position, int include_declaration,
    int64_t *out_request_id)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_lsp_references(&service->lsp, uri, position,
                                include_declaration, out_request_id);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) umi_language_client_session_record_request(&service->session);
    return status;
}

/*
 * Provide the studio language service workspace symbols operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_language_service_workspace_symbols(
    UmiStudioLanguageService *service, const char *query,
    int64_t *out_request_id)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_lsp_workspace_symbols(&service->lsp,
        query != NULL ? query : "", out_request_id);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) umi_language_client_session_record_request(&service->session);
    return status;
}

/*
 * Provide the studio language service import compilation database operation used by this
 * module and its client applications.
 */
UmiStatus umi_studio_language_service_import_compilation_database(
    UmiStudioLanguageService *service, const char *json, size_t *out_imported)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    return umi_compilation_database_import_json(
        umi_language_service_compilation_database(service->model),
        json, "memory://studio/compile_commands.json", out_imported);
}

/*
 * Provide the studio language service snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_language_service_snapshot(
    const UmiStudioLanguageService *service,
    UmiStudioLanguageSnapshot *out_snapshot)
{
    UmiProtocolTransportStats stats;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (service == NULL || out_snapshot == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    (void)memset(out_snapshot, 0, sizeof(*out_snapshot));
    stats = umi_protocol_transport_stats(service->transport);
    out_snapshot->client_state = umi_protocol_client_state(service->client);
    out_snapshot->initialized = service->lsp.initialized;
    out_snapshot->open_documents =
        umi_lsp_document_registry_count(service->documents);
    out_snapshot->queued_messages = stats.queued;
    out_snapshot->sent_messages = stats.sent;
    out_snapshot->received_messages = stats.received;
    {
        UmiLanguageServiceSnapshot model;
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (umi_language_service_snapshot(service->model, &model) == UMI_STATUS_OK) {
            out_snapshot->language_definitions = model.definition_count;
            out_snapshot->diagnostics = model.diagnostic_count;
            out_snapshot->compilation_commands = model.compilation_command_count;
            out_snapshot->pending_requests = model.pending_request_count;
        }
    }
    (void)snprintf(out_snapshot->session_state,
                   sizeof(out_snapshot->session_state), "%s",
                   umi_language_client_state_text(service->session.state));
    return UMI_STATUS_OK;
}

/*
 * Provide the studio language service model operation used by this module and its client
 * applications.
 */
UmiLanguageService *umi_studio_language_service_model(
    UmiStudioLanguageService *service)
{
    return service != NULL ? service->model : NULL;
}

/*
 * Provide the studio language service transport operation used by this module and its
 * client applications.
 */
UmiProtocolTransport *umi_studio_language_service_transport(
    UmiStudioLanguageService *service)
{
    return service != NULL ? service->transport : NULL;
}
