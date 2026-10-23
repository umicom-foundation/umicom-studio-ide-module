/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/problems_paging.inc
 * PURPOSE: Keep Problems page state with its native list and route navigation through Framework.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/diagnostic_ui/page_navigation.h"
/* C23 requires the refresh declaration before page-button callbacks. */
static void runtime_problems_refresh_list(UmiStudioGtkRuntimeChrome *runtime);
struct RuntimeProblemsBinding;
static UmiStudioGtkRuntimeChrome *RuntimeProblemsBindingOwner(struct RuntimeProblemsBinding *binding,
    GObject **window, GObject **list);
static void RuntimeProblemsBind(UmiStudioGtkRuntimeChrome *runtime, GtkWidget *widget,
    const char *signal, GCallback callback, const char *id);
static UmiStatus RuntimeProblemsTokenCurrent(UmiStudioGtkRuntimeChrome *runtime, GObject *token);
typedef struct RuntimeProblemsPaging {
    size_t offset;
    GtkWidget *previous, *next, *range;
} RuntimeProblemsPaging;
static RuntimeProblemsPaging *RuntimeProblemsPagingState(GtkWidget *list)
{ return g_object_get_data(G_OBJECT(list), "studio-problems-paging"); }
static void RuntimeProblemsPagingFree(gpointer data)
{
    RuntimeProblemsPaging *state = data;
    g_clear_object(&state->previous); g_clear_object(&state->next); g_clear_object(&state->range);
    g_free(state);
}
static void RuntimeProblemsPageClicked(GtkButton *button, gpointer data)
{
    GObject *window = NULL, *list = NULL;
    UmiStudioGtkRuntimeChrome *runtime = RuntimeProblemsBindingOwner(data, &window, &list);
    if (runtime != NULL && gtk_widget_get_root(GTK_WIDGET(button)) == GTK_ROOT(window)) {
        RuntimeProblemsPaging *state = RuntimeProblemsPagingState(GTK_WIDGET(list));
        GObject *token = g_object_get_data(list, "studio-problems-page");
        if (state == NULL || g_object_get_data(list, "studio-problems-refreshing") != NULL) goto done;
        if (RuntimeProblemsTokenCurrent(runtime, token) != UMI_STATUS_OK) {
            /* A stale click refreshes the visible evidence before another click
             * can move it. Never infer a destination from an obsolete total. */
            runtime->problemsRendered = 0;
            runtime_problems_refresh_list(runtime);
            goto done;
        }
        const UmiDiagnosticPageNavigation *navigation = g_object_get_data(token, "navigation");
        if (navigation == NULL || !gtk_widget_get_sensitive(GTK_WIDGET(button))) goto done;
        size_t offset;
        if (GTK_WIDGET(button) == state->previous && navigation->has_previous)
            offset = navigation->previous_offset;
        else if (GTK_WIDGET(button) == state->next && navigation->has_next)
            offset = navigation->next_offset;
        else goto done;
        state->offset = offset;
        /* A fresh query identity revokes retained rows even if a later click
         * returns to the same offset without a model revision change. */
        g_object_set_data_full(list, "studio-problems-query", g_object_new(G_TYPE_OBJECT, NULL), g_object_unref);
        g_object_set_data(list, "studio-problems-page", NULL);
        runtime->problemsSelectedId[0] = '\0';
        runtime->problemsRendered = 0;
        runtime_problems_refresh_list(runtime);
    }
done:
    g_clear_object(&list); g_clear_object(&window);
}
static void RuntimeProblemsPagingControls(UmiStudioGtkRuntimeChrome *runtime, GtkWidget *panel)
{
    RuntimeProblemsPaging *state = g_new0(RuntimeProblemsPaging, 1);
    GtkWidget *controls = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    state->previous = g_object_ref_sink(gtk_button_new_with_label("Previous page"));
    state->next = g_object_ref_sink(gtk_button_new_with_label("Next page"));
    state->range = g_object_ref_sink(runtime_label("No diagnostic page", 0.0F));
    gtk_widget_set_sensitive(state->previous, FALSE);
    gtk_widget_set_sensitive(state->next, FALSE);
    RuntimeProblemsBind(runtime, state->previous, "clicked", G_CALLBACK(RuntimeProblemsPageClicked),
        "studio.problems.previous-page");
    RuntimeProblemsBind(runtime, state->next, "clicked", G_CALLBACK(RuntimeProblemsPageClicked),
        "studio.problems.next-page");
    g_object_set_data_full(G_OBJECT(state->range), "umicom-automation-id",
        g_strdup("studio.problems.page-range"), g_free);
    gtk_widget_set_tooltip_text(state->previous, "Show the preceding page of the current diagnostic query");
    gtk_widget_set_tooltip_text(state->next, "Show the next page without changing source navigation");
    gtk_label_set_wrap(GTK_LABEL(state->range), TRUE);
    gtk_box_append(GTK_BOX(controls), state->previous);
    gtk_box_append(GTK_BOX(controls), state->next);
    gtk_box_append(GTK_BOX(controls), state->range);
    g_object_set_data_full(G_OBJECT(runtime->problems_list), "studio-problems-paging", state, RuntimeProblemsPagingFree);
    gtk_box_append(GTK_BOX(panel), controls);
}
