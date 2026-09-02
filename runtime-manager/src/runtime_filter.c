/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/runtime-manager/src/runtime_filter.c
 *
 * PURPOSE:
 *   Implement Application Hub free-text, category, favourite, running and installed filters.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This file is part of the Studio Application Hub / Runtime Manager.
 * The code is intentionally explicit and bounded so a new C developer can
 * follow the data flow without hidden ownership or private product coupling.
 */

#include "umicom/studio/runtime_filter.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

/*
 * Provide the contains case insensitive operation used by this module and its client
 * applications.
 */
static bool contains_case_insensitive(const char *haystack,
                                      const char *needle)
{
    size_t haystack_length;
    size_t needle_length;
    size_t start;
    size_t offset;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (haystack == NULL || needle == NULL) {
        return false;
    }

    needle_length = strlen(needle);
    /* Apply this branch only when its contract condition is satisfied. */
    if (needle_length == 0U) {
        return true;
    }

    haystack_length = strlen(haystack);
    /* Apply this branch only when its contract condition is satisfied. */
    if (needle_length > haystack_length) {
        return false;
    }

    /* Visit each bounded item once so every record receives the same rule. */
    for (start = 0U; start + needle_length <= haystack_length; ++start) {
        bool equal = true;
        /* Visit each bounded item once so every record receives the same rule. */
        for (offset = 0U; offset < needle_length; ++offset) {
            /* Apply this branch only when its contract condition is satisfied. */
            if (tolower((unsigned char)haystack[start + offset]) !=
                tolower((unsigned char)needle[offset])) {
                equal = false;
                break;
            }
        }
        /* Use the stable identifier comparison to choose the matching record or policy. */
        if (equal) {
            return true;
        }
    }

    return false;
}

/*
 * Initialise studio runtime filter from caller-provided values so later operations receive
 * a known state.
 */
void umi_studio_runtime_filter_init(UmiStudioRuntimeFilter *filter)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (filter == NULL) {
        return;
    }

    (void)memset(filter, 0, sizeof(*filter));
    filter->category = UMI_STUDIO_RUNTIME_CATEGORY_ALL;
}

/*
 * Provide the studio runtime filter match operation used by this module and its client
 * applications.
 */
bool umi_studio_runtime_filter_match(
    const UmiStudioRuntimeFilter *filter,
    const UmiStudioRuntimeEntry *entry,
    UmiIntegrationApplicationState state)
{
    size_t index;
    bool query_match;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (filter == NULL || entry == NULL) {
        return false;
    }

    /* Apply this branch only when its contract condition is satisfied. */
    if (filter->category != UMI_STUDIO_RUNTIME_CATEGORY_ALL &&
        filter->category != entry->category) {
        return false;
    }

    /* Apply this branch only when its contract condition is satisfied. */
    if (filter->favourites_only && !entry->favourite) {
        return false;
    }

    /* Apply this branch only when its contract condition is satisfied. */
    if (filter->running_only && state != UMI_INTEGRATION_APP_RUNNING) {
        return false;
    }

    /* Apply this branch only when its contract condition is satisfied. */
    if (filter->installed_only && !entry->installed) {
        return false;
    }

    /* Apply this branch only when its contract condition is satisfied. */
    if (filter->query[0] == '\0') {
        return true;
    }

    query_match =
        contains_case_insensitive(entry->application.id, filter->query) ||
        contains_case_insensitive(entry->application.name, filter->query) ||
        contains_case_insensitive(entry->description, filter->query) ||
        contains_case_insensitive(umi_studio_runtime_category_text(entry->category),
                                  filter->query);

    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (query_match) {
        return true;
    }

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < entry->application.capability_count; ++index) {
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (contains_case_insensitive(entry->application.capabilities[index],
                                      filter->query)) {
            return true;
        }
    }

    return false;
}
