/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/session.c
 *
 * PURPOSE:
 *   Implement Studio workspace and document session metadata on top of the
 *   Framework session store and its atomic persistence contract.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/session.h"

#include <inttypes.h>
#include <stdio.h>

/*
 * Provide the studio session default path operation used by this module and its client
 * applications.
 */
const char *umi_studio_session_default_path(void)
{
    return UMI_STUDIO_SESSION_DEFAULT_PATH;
}

/*
 * Provide the studio recovery default root operation used by this module and its client
 * applications.
 */
const char *umi_studio_recovery_default_root(void)
{
    return UMI_STUDIO_RECOVERY_DEFAULT_ROOT;
}

/*
 * Provide the studio session set workspace operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_session_set_workspace(UmiStudioServices *services,
                                           const char *workspace_root)
{
    UmiSessionStore *session = umi_studio_services_session(services);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (session == NULL || workspace_root == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    return umi_session_store_set(session,
                                 "workspace.root",
                                 workspace_root);
}

/*
 * Provide the studio session set active document operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_session_set_active_document(
    UmiStudioServices *services,
    UmiDocumentId document_id)
{
    UmiSessionStore *session = umi_studio_services_session(services);
    char value[64];
    int written;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (session == NULL || document_id == 0U) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    written = snprintf(value, sizeof(value), "%" PRIu64, document_id);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (written < 0 || (size_t)written >= sizeof(value)) {
        return UMI_STATUS_INTERNAL_ERROR;
    }
    return umi_session_store_set(session, "document.active", value);
}

/* Provide the document key operation used by this module and its client applications. */
static UmiStatus document_key(UmiDocumentId document_id,
                              char *out_key,
                              size_t key_capacity)
{
    int written;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (document_id == 0U || out_key == NULL || key_capacity == 0U) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    written = snprintf(out_key,
                       key_capacity,
                       "document.%" PRIu64 ".path",
                       document_id);
    return written < 0 || (size_t)written >= key_capacity
        ? UMI_STATUS_CAPACITY_EXCEEDED
        : UMI_STATUS_OK;
}

/*
 * Provide the studio session remember document operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_session_remember_document(
    UmiStudioServices *services,
    UmiDocumentId document_id,
    const char *path)
{
    UmiSessionStore *session = umi_studio_services_session(services);
    char key[UMI_SESSION_KEY_CAPACITY];
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (session == NULL || path == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    status = document_key(document_id, key, sizeof(key));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        return status;
    }
    return umi_session_store_set(session, key, path);
}

/*
 * Provide the studio session forget document operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_session_forget_document(
    UmiStudioServices *services,
    UmiDocumentId document_id)
{
    UmiSessionStore *session = umi_studio_services_session(services);
    char key[UMI_SESSION_KEY_CAPACITY];
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (session == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    status = document_key(document_id, key, sizeof(key));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        return status;
    }
    status = umi_session_store_remove(session, key);
    return status == UMI_STATUS_NOT_FOUND ? UMI_STATUS_OK : status;
}

/*
 * Write studio session in its stable representation and report capacity or input failures
 * to the caller.
 */
UmiStatus umi_studio_session_save(UmiStudioServices *services)
{
    UmiSessionStore *session = umi_studio_services_session(services);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (session == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    return umi_session_store_save(session,
                                  umi_studio_session_default_path());
}

/*
 * Provide the studio session reload operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_session_reload(UmiStudioServices *services,
                                    int *out_loaded)
{
    UmiSessionStore *session = umi_studio_services_session(services);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (session == NULL || out_loaded == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    return umi_session_store_load(session,
                                  umi_studio_session_default_path(),
                                  out_loaded);
}
