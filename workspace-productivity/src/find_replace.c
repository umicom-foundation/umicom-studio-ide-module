/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/workspace-productivity/src/find_replace.c
 *
 * PURPOSE:
 *   Implement bounded literal find/replace with case and whole-word options.
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

#include "umicom/studio/find_replace.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

/* Provide the is word character operation used by this module and its client applications. */
static bool is_word_character(char c)
{
    const unsigned char value = (unsigned char)c;
    return isalnum(value) != 0 || c == '_';
}

/* Provide the text equal n operation used by this module and its client applications. */
static bool text_equal_n(const char *left,
                         const char *right,
                         size_t length,
                         bool case_sensitive)
{
    size_t index;

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < length; ++index) {
        unsigned char a = (unsigned char)left[index];
        unsigned char b = (unsigned char)right[index];

        /* Apply this branch only when its contract condition is satisfied. */
        if (!case_sensitive) {
            a = (unsigned char)tolower(a);
            b = (unsigned char)tolower(b);
        }

        /* Apply this branch only when its contract condition is satisfied. */
        if (a != b) {
            return false;
        }
    }

    return true;
}

/*
 * Provide the occurrence matches operation used by this module and its client
 * applications.
 */
static bool occurrence_matches(const char *text,
                               size_t text_length,
                               size_t offset,
                               const UmiStudioFindReplace *operation)
{
    const size_t search_length = strlen(operation->search);

    /* Apply this branch only when its contract condition is satisfied. */
    if (offset + search_length > text_length) {
        return false;
    }

    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (!text_equal_n(text + offset,
                      operation->search,
                      search_length,
                      operation->case_sensitive)) {
        return false;
    }

    /* Apply this branch only when its contract condition is satisfied. */
    if (operation->whole_word) {
        /* Apply this branch only when its contract condition is satisfied. */
        if (offset > 0U && is_word_character(text[offset - 1U])) {
            return false;
        }

        /* Apply this branch only when its contract condition is satisfied. */
        if (offset + search_length < text_length &&
            is_word_character(text[offset + search_length])) {
            return false;
        }
    }

    return true;
}

/*
 * Initialise studio find replace from caller-provided values so later operations receive a
 * known state.
 */
void umi_studio_find_replace_init(UmiStudioFindReplace *operation)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (operation != NULL) {
        (void)memset(operation, 0, sizeof(*operation));
    }
}

/*
 * Copy studio find replace into module-owned storage so callers keep ownership of their
 * input values.
 */
UmiStatus umi_studio_find_replace_set(
    UmiStudioFindReplace *operation,
    const char *search,
    const char *replacement)
{
    int written;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (operation == NULL || search == NULL || replacement == NULL ||
        search[0] == '\0') {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    written = snprintf(operation->search,
                       sizeof(operation->search),
                       "%s",
                       search);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (written < 0 || (size_t)written >= sizeof(operation->search)) {
        return UMI_STATUS_CAPACITY_EXCEEDED;
    }

    written = snprintf(operation->replacement,
                       sizeof(operation->replacement),
                       "%s",
                       replacement);
    return written >= 0 &&
           (size_t)written < sizeof(operation->replacement)
        ? UMI_STATUS_OK
        : UMI_STATUS_CAPACITY_EXCEEDED;
}

/* Return the number of records represented by studio find without changing their state. */
UmiStatus umi_studio_find_count(
    const char *text,
    const UmiStudioFindReplace *operation,
    size_t *count)
{
    const size_t text_length = text != NULL ? strlen(text) : 0U;
    const size_t search_length =
        operation != NULL ? strlen(operation->search) : 0U;
    size_t offset = 0U;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (text == NULL || operation == NULL || count == NULL ||
        search_length == 0U) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    /* Apply this branch only when its contract condition is satisfied. */
    if (operation->regular_expression) {
        return UMI_STATUS_NOT_IMPLEMENTED;
    }

    *count = 0U;

    /*
     * Continue only while work remains available; the loop body advances the state on each
     * pass.
     */
    while (offset + search_length <= text_length) {
        /* Use the stable identifier comparison to choose the matching record or policy. */
        if (occurrence_matches(text, text_length, offset, operation)) {
            ++(*count);
            offset += search_length;
        } /* Use this fallback path when the earlier condition does not apply. */ else {
            ++offset;
        }
    }

    return UMI_STATUS_OK;
}

/*
 * Provide the studio replace all operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_replace_all(
    const char *text,
    const UmiStudioFindReplace *operation,
    char *output,
    size_t output_capacity,
    size_t *replacement_count)
{
    const size_t text_length = text != NULL ? strlen(text) : 0U;
    const size_t search_length =
        operation != NULL ? strlen(operation->search) : 0U;
    const size_t replacement_length =
        operation != NULL ? strlen(operation->replacement) : 0U;
    size_t source = 0U;
    size_t destination = 0U;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (text == NULL || operation == NULL || output == NULL ||
        output_capacity == 0U || replacement_count == NULL ||
        search_length == 0U) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    /* Apply this branch only when its contract condition is satisfied. */
    if (operation->regular_expression) {
        return UMI_STATUS_NOT_IMPLEMENTED;
    }

    *replacement_count = 0U;

    /*
     * Continue only while work remains available; the loop body advances the state on each
     * pass.
     */
    while (source < text_length) {
        /* Use the stable identifier comparison to choose the matching record or policy. */
        if (occurrence_matches(text, text_length, source, operation)) {
            /* Keep the operation inside its valid bounds before reading, writing or adding data. */
            if (destination + replacement_length >= output_capacity) {
                return UMI_STATUS_CAPACITY_EXCEEDED;
            }
            (void)memcpy(output + destination,
                         operation->replacement,
                         replacement_length);
            destination += replacement_length;
            source += search_length;
            ++(*replacement_count);
        } /* Use this fallback path when the earlier condition does not apply. */ else {
            /* Keep the operation inside its valid bounds before reading, writing or adding data. */
            if (destination + 1U >= output_capacity) {
                return UMI_STATUS_CAPACITY_EXCEEDED;
            }
            output[destination++] = text[source++];
        }
    }

    output[destination] = '\0';
    return UMI_STATUS_OK;
}
