/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/documents.h
 *
 * PURPOSE:
 *   Define Studio document use cases over the Framework document and recovery
 *   services so console, GTK4 and future frontends share identical behaviour.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_DOCUMENTS_H
#define UMICOM_STUDIO_DOCUMENTS_H

#include <stddef.h>

#include "umicom/studio/services.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Provide the studio documents new operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_documents_new(UmiStudioServices *services,
                                   const char *display_name,
                                   UmiDocumentId *out_document_id);
/**
 * Provide the studio documents open operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_documents_open(UmiStudioServices *services,
                                    const char *path,
                                    UmiDocumentId *out_document_id);
/**
 * Provide the studio documents replace operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_documents_replace(UmiStudioServices *services,
                                       UmiDocumentId document_id,
                                       const char *text,
                                       size_t length);
/**
 * Write studio documents in its stable representation and report capacity or input
 * failures to the caller.
 */
UmiStatus umi_studio_documents_save(UmiStudioServices *services,
                                    UmiDocumentId document_id);
/**
 * Provide the studio documents save as operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_documents_save_as(UmiStudioServices *services,
                                       UmiDocumentId document_id,
                                       const char *path);
/**
 * Provide the studio documents close operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_documents_close(UmiStudioServices *services,
                                     UmiDocumentId document_id,
                                     int force);
/**
 * Provide the studio documents checkpoint operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_documents_checkpoint(UmiStudioServices *services,
                                          UmiDocumentId document_id);
/**
 * Provide the studio documents restore recovery operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_documents_restore_recovery(
    UmiStudioServices *services,
    UmiDocumentId recovery_document_id,
    UmiDocumentId *out_document_id
);
/**
 * Provide the studio documents save all operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_documents_save_all(UmiStudioServices *services,
                                        size_t *out_saved,
                                        size_t *out_failed);

#ifdef __cplusplus
}
#endif

#endif
