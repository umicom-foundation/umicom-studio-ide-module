/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/documents.c
 *
 * PURPOSE:
 *   Implement Studio document creation, opening, atomic saving, session
 *   tracking and crash-recovery checkpoints through Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/documents.h"

#include <string.h>

#include "umicom/studio/session.h"

/*
 * Provide the display name from path operation used by this module and its client
 * applications.
 */
static const char *display_name_from_path(const char *path)
{
    const char *name = strrchr(path, '/');
#ifdef _WIN32
    const char *backslash = strrchr(path, '\\');
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (backslash != NULL && (name == NULL || backslash > name)) {
        name = backslash;
    }
#endif
    return name != NULL ? name + 1 : path;
}

/*
 * Provide the studio documents new operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_documents_new(UmiStudioServices *services,
                                   const char *display_name,
                                   UmiDocumentId *out_document_id)
{
    UmiDocumentStore *documents = umi_studio_services_documents(services);
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (documents == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    status = umi_document_store_new(documents,
                                    display_name,
                                    out_document_id);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        (void)umi_studio_session_set_active_document(services,
                                                     *out_document_id);
    }
    return status;
}

/*
 * Provide the studio documents open operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_documents_open(UmiStudioServices *services,
                                    const char *path,
                                    UmiDocumentId *out_document_id)
{
    UmiDocumentStore *documents = umi_studio_services_documents(services);
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (documents == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    status = umi_document_store_open(documents, path, out_document_id);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        (void)umi_studio_session_remember_document(services,
                                                   *out_document_id,
                                                   path);
        (void)umi_studio_session_set_active_document(services,
                                                     *out_document_id);
    }
    return status;
}

/*
 * Provide the studio documents replace operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_documents_replace(UmiStudioServices *services,
                                       UmiDocumentId document_id,
                                       const char *text,
                                       size_t length)
{
    UmiDocumentStore *documents = umi_studio_services_documents(services);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (documents == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    return umi_document_store_replace_text(documents,
                                           document_id,
                                           text,
                                           length);
}

/* Provide the remove recovery operation used by this module and its client applications. */
static UmiStatus remove_recovery(UmiStudioServices *services,
                                 UmiDocumentId document_id)
{
    UmiRecoveryManager *recovery = umi_studio_services_recovery(services);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (recovery == NULL) {
        return UMI_STATUS_INVALID_STATE;
    }
    return umi_recovery_manager_remove(recovery, document_id);
}

/*
 * Write studio documents in its stable representation and report capacity or input
 * failures to the caller.
 */
UmiStatus umi_studio_documents_save(UmiStudioServices *services,
                                    UmiDocumentId document_id)
{
    UmiDocumentStore *documents = umi_studio_services_documents(services);
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (documents == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    status = umi_document_store_save(documents, document_id);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        (void)remove_recovery(services, document_id);
    }
    return status;
}

/*
 * Provide the studio documents save as operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_documents_save_as(UmiStudioServices *services,
                                       UmiDocumentId document_id,
                                       const char *path)
{
    UmiDocumentStore *documents = umi_studio_services_documents(services);
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (documents == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    status = umi_document_store_save_as(documents, document_id, path);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        (void)umi_studio_session_remember_document(services,
                                                   document_id,
                                                   path);
        (void)remove_recovery(services, document_id);
    }
    return status;
}

/*
 * Provide the studio documents close operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_documents_close(UmiStudioServices *services,
                                     UmiDocumentId document_id,
                                     int force)
{
    UmiDocumentStore *documents = umi_studio_services_documents(services);
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (documents == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    status = umi_document_store_close(documents, document_id, force);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        (void)umi_studio_session_forget_document(services, document_id);
        (void)remove_recovery(services, document_id);
    }
    return status;
}

/*
 * Provide the studio documents checkpoint operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_documents_checkpoint(UmiStudioServices *services,
                                          UmiDocumentId document_id)
{
    UmiDocumentStore *documents = umi_studio_services_documents(services);
    UmiRecoveryManager *recovery = umi_studio_services_recovery(services);
    UmiDocumentSnapshot snapshot;
    char *text = NULL;
    size_t length = 0U;
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (documents == NULL || recovery == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    status = umi_document_store_snapshot(documents,
                                         document_id,
                                         &snapshot);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        return status;
    }
    /* Apply this branch only when its contract condition is satisfied. */
    if (!snapshot.dirty) {
        return remove_recovery(services, document_id);
    }

    status = umi_document_store_copy_text(documents,
                                          document_id,
                                          &text,
                                          &length);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        return status;
    }
    status = umi_recovery_manager_save(recovery,
                                       document_id,
                                       snapshot.path,
                                       snapshot.revision,
                                       text,
                                       length);
    umi_document_store_free_text(text);
    return status;
}

/*
 * Provide the studio documents restore recovery operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_documents_restore_recovery(
    UmiStudioServices *services,
    UmiDocumentId recovery_document_id,
    UmiDocumentId *out_document_id)
{
    UmiRecoveryManager *recovery = umi_studio_services_recovery(services);
    UmiDocumentStore *documents = umi_studio_services_documents(services);
    UmiRecoveryRecord record;
    UmiStatus status;
    const char *display_name;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (recovery == NULL || documents == NULL || out_document_id == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    *out_document_id = 0U;

    status = umi_recovery_manager_load(recovery,
                                       recovery_document_id,
                                       &record);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        return status;
    }

    display_name = record.source_path[0] != '\0'
        ? display_name_from_path(record.source_path)
        : "Recovered document";
    status = umi_document_store_new(documents,
                                    display_name,
                                    out_document_id);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_document_store_replace_text(documents,
                                                 *out_document_id,
                                                 record.text,
                                                 record.length);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK && record.source_path[0] != '\0') {
        (void)umi_studio_session_remember_document(services,
                                                   *out_document_id,
                                                   record.source_path);
    }
    umi_recovery_record_dispose(&record);
    return status;
}

/*
 * Provide the studio documents save all operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_documents_save_all(UmiStudioServices *services,
                                        size_t *out_saved,
                                        size_t *out_failed)
{
    UmiDocumentStore *documents = umi_studio_services_documents(services);
    size_t saved = 0U;
    size_t failed = 0U;
    size_t index;
    size_t count;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (documents == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    count = umi_document_store_count(documents);
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < count; ++index) {
        UmiDocumentSnapshot snapshot;
        UmiStatus status = umi_document_store_at(documents,
                                                 index,
                                                 &snapshot);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) {
            failed += 1U;
            continue;
        }
        /* Apply this branch only when its contract condition is satisfied. */
        if (!snapshot.dirty) {
            continue;
        }
        /* Apply this branch only when its contract condition is satisfied. */
        if (!snapshot.has_path) {
            failed += 1U;
            continue;
        }
        status = umi_studio_documents_save(services,
                                           snapshot.document_id);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) {
            saved += 1U;
        } /* Use this fallback path when the earlier condition does not apply. */ else {
            failed += 1U;
        }
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (out_saved != NULL) {
        *out_saved = saved;
    }
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (out_failed != NULL) {
        *out_failed = failed;
    }
    return failed == 0U ? UMI_STATUS_OK : UMI_STATUS_IO_ERROR;
}
