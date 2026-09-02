/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/workspace-productivity/include/umicom/studio/editor_status.h
 *
 * PURPOSE:
 *   Format editor cursor, language, encoding and line-ending status.
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

#ifndef UMICOM_STUDIO_EDITOR_STATUS_H
#define UMICOM_STUDIO_EDITOR_STATUS_H

#include <stddef.h>

#include "umicom/base/status.h"
#include "umicom/studio/workspace_document.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * List the named studio line ending values accepted by this public contract.
 */
typedef enum UmiStudioLineEnding {
    UMI_STUDIO_LINE_ENDING_LF = 0,
    UMI_STUDIO_LINE_ENDING_CRLF,
    UMI_STUDIO_LINE_ENDING_CR
} UmiStudioLineEnding;

/**
 * Represent the studio editor status data shared with callers of this public contract.
 */
typedef struct UmiStudioEditorStatus {
    size_t line;
    size_t column;
    size_t selection_length;
    char language[UMI_STUDIO_WORKSPACE_LANGUAGE_CAPACITY];
    char encoding[32];
    UmiStudioLineEnding line_ending;
} UmiStudioEditorStatus;

/**
 * Initialise studio editor status from caller-provided values so later operations receive
 * a known state.
 */
void umi_studio_editor_status_init(UmiStudioEditorStatus *status);

/**
 * Provide the studio editor status format operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_editor_status_format(
    const UmiStudioEditorStatus *status,
    char *output,
    size_t output_capacity
);

/**
 * Provide the studio line ending text operation used by this module and its client
 * applications.
 */
const char *umi_studio_line_ending_text(UmiStudioLineEnding line_ending);

#ifdef __cplusplus
}
#endif

#endif
