/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/workspace-productivity/include/umicom/studio/find_replace.h
 *
 * PURPOSE:
 *   Provide bounded literal find/replace operations and search options.
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

#ifndef UMICOM_STUDIO_FIND_REPLACE_H
#define UMICOM_STUDIO_FIND_REPLACE_H

#include <stdbool.h>
#include <stddef.h>

#include "umicom/base/status.h"
#include "umicom/studio/workspace_document.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio find replace data shared with callers of this public contract.
 */
typedef struct UmiStudioFindReplace {
    char search[UMI_STUDIO_WORKSPACE_NAME_CAPACITY];
    char replacement[UMI_STUDIO_WORKSPACE_NAME_CAPACITY];
    bool case_sensitive;
    bool whole_word;
    bool regular_expression;
    bool selection_only;
} UmiStudioFindReplace;

/**
 * Initialise studio find replace from caller-provided values so later operations receive a
 * known state.
 */
void umi_studio_find_replace_init(UmiStudioFindReplace *operation);

/**
 * Copy studio find replace into module-owned storage so callers keep ownership of their
 * input values.
 */
UmiStatus umi_studio_find_replace_set(
    UmiStudioFindReplace *operation,
    const char *search,
    const char *replacement
);

/**
 * Return the number of records represented by studio find without changing their state.
 */
UmiStatus umi_studio_find_count(
    const char *text,
    const UmiStudioFindReplace *operation,
    size_t *count
);

/**
 * Provide the studio replace all operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_replace_all(
    const char *text,
    const UmiStudioFindReplace *operation,
    char *output,
    size_t output_capacity,
    size_t *replacement_count
);

#ifdef __cplusplus
}
#endif

#endif
