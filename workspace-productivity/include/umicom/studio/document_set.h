/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/workspace-productivity/include/umicom/studio/document_set.h
 *
 * PURPOSE:
 *   Manage bounded open-editor documents and active-tab state.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This file is part of Studio workspace/editor productivity. The implementation
 * uses bounded C23 data structures and explicit ownership so the behaviour is
 * easy to inspect, test and later connect to the main Studio workbench.
 */

#ifndef UMICOM_STUDIO_DOCUMENT_SET_H
#define UMICOM_STUDIO_DOCUMENT_SET_H

#include "umicom/studio/workspace_document.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio document set data shared with callers of this public contract.
 */
typedef struct UmiStudioDocumentSet {
    UmiStudioWorkspaceDocument documents[UMI_STUDIO_WORKSPACE_MAX_DOCUMENTS];
    size_t count;
    uint64_t usage_tick;
} UmiStudioDocumentSet;

/**
 * Initialise studio document set from caller-provided values so later operations receive a
 * known state.
 */
void umi_studio_document_set_init(UmiStudioDocumentSet *set);

/**
 * Provide the studio document set open operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_document_set_open(
    UmiStudioDocumentSet *set,
    const char *path,
    const char *language,
    bool preview
);

/**
 * Find studio document set while leaving the underlying catalogue or model owned by this
 * module.
 */
UmiStudioWorkspaceDocument *umi_studio_document_set_find(
    UmiStudioDocumentSet *set,
    const char *path
);

/**
 * Provide the studio document set find const operation used by this module and its client
 * applications.
 */
const UmiStudioWorkspaceDocument *umi_studio_document_set_find_const(
    const UmiStudioDocumentSet *set,
    const char *path
);

/**
 * Provide the studio document set activate operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_document_set_activate(
    UmiStudioDocumentSet *set,
    const char *path
);

/**
 * Provide the studio document set active operation used by this module and its client
 * applications.
 */
UmiStudioWorkspaceDocument *umi_studio_document_set_active(
    UmiStudioDocumentSet *set
);

/**
 * Provide the studio document set mark dirty operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_document_set_mark_dirty(
    UmiStudioDocumentSet *set,
    const char *path,
    bool dirty
);

/**
 * Provide the studio document set pin operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_document_set_pin(
    UmiStudioDocumentSet *set,
    const char *path,
    bool pinned
);

/**
 * Provide the studio document set close operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_document_set_close(
    UmiStudioDocumentSet *set,
    const char *path,
    bool force
);

/**
 * Return the number of records represented by studio document set dirty without changing
 * their state.
 */
size_t umi_studio_document_set_dirty_count(const UmiStudioDocumentSet *set);

#ifdef __cplusplus
}
#endif

#endif
