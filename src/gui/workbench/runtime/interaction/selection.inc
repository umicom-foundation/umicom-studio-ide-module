/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/selection.inc
 *
 * PURPOSE:
 *   Keep transient GTK row selections bounded and presentation-only. Durable
 *   selection models continue to be owned by Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static void runtime_copy_selection(
    char *destination,
    size_t capacity,
    const char *value)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (destination == NULL || capacity == 0U) return;

    destination[0] = '\0';
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (value == NULL) return;

    (void)snprintf(destination, capacity, "%s", value);
}

/*
 * Provide the runtime text contains casefold operation used by this module and its client
 * applications.
 */
static int runtime_text_contains_casefold(
    const char *text,
    const char *query)
{
    gchar *folded_text;
    gchar *folded_query;
    int found;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (query == NULL || query[0] == '\0') return 1;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (text == NULL) return 0;

    folded_text = g_utf8_casefold(text, -1);
    folded_query = g_utf8_casefold(query, -1);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (folded_text == NULL || folded_query == NULL) {
        g_free(folded_text);
        g_free(folded_query);
        return 0;
    }

    found = strstr(folded_text, folded_query) != NULL;
    g_free(folded_text);
    g_free(folded_query);
    return found;
}
