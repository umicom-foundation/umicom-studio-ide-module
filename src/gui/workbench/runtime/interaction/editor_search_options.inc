/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/editor_search_options.inc
 * PURPOSE: Collect explicit search preferences without owning document matching or replacement behavior.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/document/search.h"
static UmiEditorSearchOptions RuntimeEditorSearchOptions(UmiStudioGtkRuntimeChrome *runtime)
{
    UmiEditorSearchOptions options = {UMI_EDITOR_SEARCH_CASE_SMART, 0, 0, 0U};
    if (runtime == NULL || runtime->editor_find_entry == NULL)
        return options;
    GtkWidget *mode = g_object_get_data(G_OBJECT(runtime->editor_find_entry), "umicom-search-case");
    GtkWidget *word = g_object_get_data(G_OBJECT(runtime->editor_find_entry), "umicom-search-word");
    if (mode != NULL)
    {
        guint selected = gtk_drop_down_get_selected(GTK_DROP_DOWN(mode));
        if (selected == 1U)
            options.case_mode = UMI_EDITOR_SEARCH_CASE_SENSITIVE;
        else if (selected == 2U)
            options.case_mode = UMI_EDITOR_SEARCH_CASE_ASCII_INSENSITIVE;
    }
    if (word != NULL)
        options.whole_word = gtk_check_button_get_active(GTK_CHECK_BUTTON(word)) ? 1 : 0;
    return options;
}
static const char *RuntimeSearchDescription(const UmiEditorSearchOptions *options)
{
    if (options->case_mode == UMI_EDITOR_SEARCH_CASE_SENSITIVE)
        return options->whole_word ? "Match case, whole word" : "Match case";
    if (options->case_mode == UMI_EDITOR_SEARCH_CASE_ASCII_INSENSITIVE)
        return options->whole_word ? "Ignore ASCII case, whole word" : "Ignore ASCII case";
    return options->whole_word ? "Smart case, whole word" : "Smart case";
}
static void RuntimeAppendSearchOptions(UmiStudioGtkRuntimeChrome *runtime, GtkWidget *bar)
{
    const char *modes[] = {"Smart case", "Match case", "Ignore ASCII case", NULL};
    GtkWidget *mode = gtk_drop_down_new_from_strings(modes);
    GtkWidget *word = gtk_check_button_new_with_label("Whole word");
    /* The Find field retains its companion controls, so retained fields never
     * expose dangling preferences when an editor surface is rebuilt. The
     * string constructor transfers its internal model; no extra model unref. */
    g_object_set_data_full(G_OBJECT(runtime->editor_find_entry), "umicom-search-case",
                           g_object_ref_sink(mode), g_object_unref);
    g_object_set_data_full(G_OBJECT(runtime->editor_find_entry), "umicom-search-word",
                           g_object_ref_sink(word), g_object_unref);
    (void)umi_gtk4_automation_tag_widget(mode, "studio.editor.search-case");
    (void)umi_gtk4_automation_tag_widget(word, "studio.editor.search-whole-word");
    gtk_accessible_update_property(GTK_ACCESSIBLE(mode), GTK_ACCESSIBLE_PROPERTY_LABEL,
                                   "Search case matching", -1);
    gtk_widget_set_tooltip_text(mode, "Smart case ignores ASCII case unless the query has an uppercase ASCII "
                                      "letter. The same policy is captured by each replacement review.");
    gtk_widget_set_tooltip_text(word, "Match complete words: letters, digits, underscore and non-ASCII text "
                                      "stay together. This is literal search, not symbol analysis.");
    gtk_box_append(GTK_BOX(bar), mode);
    gtk_box_append(GTK_BOX(bar), word);
}
