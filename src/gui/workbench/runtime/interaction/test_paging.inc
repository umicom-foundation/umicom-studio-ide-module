/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/test_paging.inc
 * PURPOSE: Publish checked Test Explorer pages and preserve navigation across periodic refresh.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/test_platform/explorer_page.h"
#include "umicom/ui/gtk4/list_publication.h"
#include "umicom/ui/gtk4/window_lifecycle.h"

/* The list owns presentation state; Framework owns rows, hierarchy and outcomes.
 * No state object borrows a service across GTK notifications. */
typedef struct RuntimeTestPageState {
    UmiStudioFileDialogRequest *owner;
    GtkWidget *summary, *search, *group, *sort, *failed;
    GtkWidget *list; /* non-owning identity, cleared on list disposal */
    GtkWidget *previous, *next, *range;
    GtkWidget *first, *last;
    GObject *query, *published;
    size_t offset;
    int publishing;
} RuntimeTestPageState;
typedef struct RuntimeTestPageCapture {
    UmiStudioFileDialogRequest *owner;
    UmiTestExplorerModel *model;
    UmiTestExplorerPage *page;
    UmiTestExplorerPageSummary summary;
    const UmiTestPlatformItemRegistry *items;
    const UmiTestPlatformResultRegistry *results;
    GObject *query;
    GtkWidget *list; /* identity only; publisher retains the list */
    size_t requested_offset;
    int running;
} RuntimeTestPageCapture;
static void RuntimeTestPageCaptureFree(gpointer data)
{
    RuntimeTestPageCapture *capture = data;
    runtime_file_dialog_request_free(capture->owner);
    UmiTestExplorerPageDestroy(capture->page);
    umi_test_explorer_model_destroy(capture->model);
    g_clear_object(&capture->query);
    g_free(capture);
}
static void RuntimeTestPageStateFree(gpointer data)
{
    RuntimeTestPageState *state = data;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(state->owner, &window);
    if (runtime != NULL && runtime->test_list == state->list) {
        runtime->test_list = NULL;
        if (runtime->test_search_entry == state->search) runtime->test_search_entry = NULL;
        if (runtime->test_summary_label == state->summary) runtime->test_summary_label = NULL;
    }
    g_clear_object(&window);
    runtime_file_dialog_request_free(state->owner);
    g_clear_object(&state->published); g_clear_object(&state->query);
    g_clear_object(&state->summary); g_clear_object(&state->search);
    g_clear_object(&state->group); g_clear_object(&state->sort); g_clear_object(&state->failed);
    g_clear_object(&state->previous); g_clear_object(&state->next); g_clear_object(&state->range);
    g_clear_object(&state->first); g_clear_object(&state->last);
    g_free(state);
}
static RuntimeTestPageState *RuntimeTestPages(UmiStudioGtkRuntimeChrome *runtime)
{
    return runtime != NULL && runtime->test_list != NULL ?
        g_object_get_data(G_OBJECT(runtime->test_list), "studio-test-pages") : NULL;
}
static void RuntimeTestPageFilter(UmiStudioGtkRuntimeChrome *runtime, UmiTestPlatformFilter *filter)
{
    umi_test_platform_filter_init(filter);
    const char *query = runtime->test_search_entry != NULL ?
        gtk_editable_get_text(GTK_EDITABLE(runtime->test_search_entry)) : "";
    g_strlcpy(filter->text, query, sizeof filter->text);
    filter->include_disabled = 1;
    filter->failed_only = RuntimeTestFailedOnly(runtime);
}
static UmiStatus RuntimeTestPageCurrent(UmiStudioGtkRuntimeChrome *runtime, GObject *token)
{
    RuntimeTestPageState *state = RuntimeTestPages(runtime);
    RuntimeTestPageCapture *capture = token != NULL ? g_object_get_data(token, "capture") : NULL;
    if (state == NULL || capture == NULL || capture->list != runtime->test_list ||
        state->query != capture->query || state->offset != capture->requested_offset ||
        capture->running != runtime->test_snapshot.operation_running)
        return UMI_STATUS_INVALID_STATE;
    UmiTestPlatformService *platform = runtime_test_platform(runtime);
    if (platform == NULL) return UMI_STATUS_UNAVAILABLE;
    UmiTestPlatformItemRegistry *items = umi_test_platform_service_item(platform);
    UmiTestPlatformResultRegistry *results = umi_test_platform_service_result(platform);
    UmiTestPlatformFilter filter;
    RuntimeTestPageFilter(runtime, &filter);
    const UmiTestExplorerModelSnapshot *m = &capture->summary.model;
    if (items != capture->items || results != capture->results ||
        umi_test_platform_item_registry_revision(items) != m->item_revision ||
        umi_test_platform_result_registry_revision(results) != m->result_revision ||
        strcmp(filter.text, m->filter.text) != 0 || filter.failed_only != m->filter.failed_only ||
        RuntimeTestGroupMode(runtime) != m->group_mode || RuntimeTestSortMode(runtime) != m->sort_mode)
        return UMI_STATUS_INVALID_STATE;
    return UmiTestExplorerPageValidate(capture->page, capture->model);
}
static UmiStatus RuntimeTestPagePublicationCurrent(GtkListBox *list, void *data)
{
    GObject *token = data;
    RuntimeTestPageCapture *capture = g_object_get_data(token, "capture");
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(capture->owner, &window);
    UmiStatus status = UMI_STATUS_CANCELLED;
    if (runtime != NULL && UmiGtk4WindowIsOpen(GTK_WINDOW(window)) &&
        runtime->test_list == GTK_WIDGET(list))
        status = RuntimeTestPageCurrent(runtime, token);
    g_clear_object(&window);
    return status;
}
static void RuntimeTestPageClicked(GtkButton *button, gpointer data)
{
    GObject *token = data;
    RuntimeTestPageCapture *capture = g_object_get_data(token, "capture");
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(capture->owner, &window);
    RuntimeTestPageState *state = RuntimeTestPages(runtime);
    GtkWidget *row = gtk_widget_get_ancestor(GTK_WIDGET(button), GTK_TYPE_LIST_BOX_ROW);
    if (state != NULL && state->published == token && !state->publishing &&
        gtk_widget_is_sensitive(GTK_WIDGET(button)) && row != NULL && gtk_widget_get_parent(row) == runtime->test_list &&
        (gpointer)gtk_widget_get_root(GTK_WIDGET(button)) == (gpointer)window &&
        RuntimeTestPageCurrent(runtime, token) == UMI_STATUS_OK) {
        int action = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button), "studio-test-action"));
        /* Retain the button because selection can replace this entire page. */
        g_object_ref(button);
        if (action == 0) runtime_on_test_select(button, runtime);
        else if (action == 1 || action == 2) {
            const char *id = g_object_get_data(G_OBJECT(button), "umicom-test-id");
            if (runtime_test_select(runtime, id)) {
                g_clear_object(&window);
                runtime = runtime_file_dialog_owner(capture->owner, &window);
                state = RuntimeTestPages(runtime);
                if (state != NULL && state->published == token && !state->publishing &&
                    RuntimeTestPageCurrent(runtime, token) == UMI_STATUS_OK)
                    (void)runtime_execute_registered_command(runtime, action == 1 ?
                        UMI_STUDIO_COMMAND_TESTS_RUN_SELECTED : UMI_STUDIO_COMMAND_TESTS_DEBUG_SELECTED, NULL);
            }
        }
        else runtime_on_test_source(button, runtime);
        g_object_unref(button);
    }
    g_clear_object(&window);
}
static void RuntimeTestPageTokenClosure(gpointer data, GClosure *closure)
{ (void)closure; g_object_unref(data); }
static GtkWidget *RuntimeTestPageRow(const UmiTestExplorerRow *test, GObject *token)
{
    GtkWidget *row = gtk_list_box_row_new(); g_object_ref_sink(row);
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    GtkWidget *labels = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
    const UmiTestPlatformItemSnapshot *item = &test->item;
    char *name = g_utf8_make_valid(item->name, -1);
    GtkWidget *title = runtime_label(name, 0.0F); g_free(name);
    const char *outcome = test->has_result ?
        umi_test_platform_outcome_text((UmiTestPlatformOutcome)test->latest_result.outcome) : "not run";
    char *raw = g_strdup_printf("%s — %s — %s — %s — %.1f ms", outcome,
        item->framework, item->kind, item->labels,
        test->has_result ? test->latest_result.duration_ms : item->last_duration_ms);
    char *detail = g_utf8_make_valid(raw, -1); g_free(raw);
    gtk_box_append(GTK_BOX(labels), title);
    gtk_box_append(GTK_BOX(labels), runtime_label(detail, 0.0F)); g_free(detail);
    gtk_widget_set_margin_start(labels, (int)((test->depth > 8U ? 8U : test->depth) * 12U));
    gtk_widget_set_hexpand(labels, TRUE);
    if (test->child_count != 0U) gtk_widget_add_css_class(title, "umicom-runtime-section-title");
    gtk_box_append(GTK_BOX(box), labels);
    const char *names[] = {"Select", "Run", "Debug", "Source"};
    for (int i = 0; i < 4; ++i) {
        GtkWidget *button = gtk_button_new_with_label(names[i]);
        g_object_set_data_full(G_OBJECT(button), "umicom-test-id", g_strdup(item->id), g_free);
        g_object_set_data(G_OBJECT(button), "studio-test-action", GINT_TO_POINTER(i));
        if (i == 3) {
            UmiTestPlatformItemSnapshot *source = g_new(UmiTestPlatformItemSnapshot, 1);
            *source = *item;
            g_object_set_data_full(G_OBJECT(button), "umicom-test-source-item", source, g_free);
            gtk_widget_set_sensitive(button, item->source_uri[0] != '\0');
        } else if (i != 0) gtk_widget_set_sensitive(button, item->enabled != 0);
        g_signal_connect_data(button, "clicked", G_CALLBACK(RuntimeTestPageClicked),
            g_object_ref(token), RuntimeTestPageTokenClosure, 0);
        gtk_box_append(GTK_BOX(box), button);
    }
    gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), box);
    g_object_set_data_full(G_OBJECT(row), "umicom-test-id", g_strdup(item->id), g_free);
    return row;
}
/* A weak list binding is used for controls, including editable changes. Holding
 * a removed search entry must never keep its window or call a retired runtime. */
typedef struct RuntimeTestPageControl {
    UmiStudioFileDialogRequest *owner;
    GWeakRef list;
} RuntimeTestPageControl;
static void RuntimeTestPageControlFree(gpointer data, GClosure *closure)
{
    (void)closure; RuntimeTestPageControl *binding = data;
    runtime_file_dialog_request_free(binding->owner); g_weak_ref_clear(&binding->list); g_free(binding);
}
static void RuntimeTestPageQueryChanged(GtkWidget *control, gpointer data)
{
    RuntimeTestPageControl *binding = data;
    GObject *window = NULL;
    GtkWidget *list = g_weak_ref_get(&binding->list);
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(binding->owner, &window);
    if (runtime != NULL && list != NULL && runtime->test_list == list &&
        (gpointer)gtk_widget_get_root(control) == (gpointer)window) {
        RuntimeTestPageState *state = RuntimeTestPages(runtime);
        if (state != NULL) {
            state->offset = 0U;
            g_clear_object(&state->published);
            g_clear_object(&state->query); state->query = g_object_new(G_TYPE_OBJECT, NULL);
            runtime_tests_refresh_list(runtime);
        }
    }
    g_clear_object(&list); g_clear_object(&window);
}
static void RuntimeTestPageChoiceChanged(GObject *control, GParamSpec *parameter, gpointer data)
{ (void)parameter; RuntimeTestPageQueryChanged(GTK_WIDGET(control), data); }
static void RuntimeTestPageBind(UmiStudioGtkRuntimeChrome *runtime, GtkWidget *control,
    const char *signal, GCallback callback)
{
    RuntimeTestPageControl *binding = g_new0(RuntimeTestPageControl, 1);
    binding->owner = runtime_file_dialog_request_new(runtime, 0U);
    g_weak_ref_init(&binding->list, runtime->test_list);
    if (binding->owner == NULL) { RuntimeTestPageControlFree(binding, NULL); return; }
    g_signal_connect_data(control, signal, callback, binding, RuntimeTestPageControlFree, 0);
}
/* Direct first/last navigation shares the same publication identity checks as adjacent pages; the former two-control dispatch remains below. Retained for engineering review. */
#if 0
static void RuntimeTestPageMove(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    RuntimeTestPageState *state = RuntimeTestPages(runtime);
    if (state == NULL || state->publishing) return;
    if (RuntimeTestPageCurrent(runtime, state->published) != UMI_STATUS_OK) {
        runtime_tests_refresh_list(runtime); return;
    }
    RuntimeTestPageCapture *capture = g_object_get_data(state->published, "capture");
    int next = GTK_WIDGET(button) == state->next;
    if ((next && !capture->summary.has_next) || (!next && !capture->summary.has_previous)) return;
    state->offset = next ? capture->summary.next_offset : capture->summary.previous_offset;
    g_clear_object(&state->published);
    g_clear_object(&state->query); state->query = g_object_new(G_TYPE_OBJECT, NULL);
    runtime_tests_refresh_list(runtime);
}
#endif
static void RuntimeTestPageMove(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    RuntimeTestPageState *state = RuntimeTestPages(runtime);
    if (state == NULL || state->publishing) return;
    if (RuntimeTestPageCurrent(runtime, state->published) != UMI_STATUS_OK) {
        runtime_tests_refresh_list(runtime); return;
    }
    RuntimeTestPageCapture *capture = g_object_get_data(state->published, "capture");
    size_t offset;
    if (GTK_WIDGET(button) == state->first || GTK_WIDGET(button) == state->previous) {
        if (!capture->summary.has_previous) return;
        offset = GTK_WIDGET(button) == state->first ? 0U : capture->summary.previous_offset;
    } else if (GTK_WIDGET(button) == state->last || GTK_WIDGET(button) == state->next) {
        if (!capture->summary.has_next) return;
        /* Framework clamps the last-page request to its current projection.
         * Studio does not duplicate page-boundary or overflow arithmetic. */
        offset = GTK_WIDGET(button) == state->last ? SIZE_MAX : capture->summary.next_offset;
    } else return;
    state->offset = offset;
    g_clear_object(&state->published);
    g_clear_object(&state->query); state->query = g_object_new(G_TYPE_OBJECT, NULL);
    runtime_tests_refresh_list(runtime);
}
static GtkWidget *RuntimeTestPageControls(UmiStudioGtkRuntimeChrome *runtime)
{
    RuntimeTestPageState *state = g_new0(RuntimeTestPageState, 1);
    state->owner = runtime_file_dialog_request_new(runtime, 0U);
    state->list = runtime->test_list;
    state->query = g_object_new(G_TYPE_OBJECT, NULL);
    state->summary = g_object_ref(runtime->test_summary_label);
    state->search = g_object_ref(runtime->test_search_entry);
    state->group = g_object_ref(g_object_get_data(G_OBJECT(runtime->test_list), "umicom-test-group-selector"));
    state->sort = g_object_ref(g_object_get_data(G_OBJECT(runtime->test_list), "umicom-test-sort-selector"));
    state->failed = g_object_ref(g_object_get_data(G_OBJECT(runtime->test_list), "umicom-test-failed-only"));
    GtkWidget *bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    state->first = gtk_button_new_with_label("First page"); g_object_ref_sink(state->first);
    state->last = gtk_button_new_with_label("Last page"); g_object_ref_sink(state->last);
    state->previous = gtk_button_new_with_label("Previous page"); g_object_ref_sink(state->previous);
    state->next = gtk_button_new_with_label("Next page"); g_object_ref_sink(state->next);
    state->range = runtime_label("No page loaded", 0.0F); g_object_ref_sink(state->range);
    gtk_box_append(GTK_BOX(bar), state->first);
    gtk_box_append(GTK_BOX(bar), state->previous);
    gtk_box_append(GTK_BOX(bar), state->range);
    gtk_box_append(GTK_BOX(bar), state->next);
    gtk_box_append(GTK_BOX(bar), state->last);
    g_object_set_data_full(G_OBJECT(runtime->test_list), "studio-test-pages", state, RuntimeTestPageStateFree);
    (void)umi_gtk4_automation_tag_widget(runtime->test_list, "studio.tests.list");
    (void)umi_gtk4_automation_tag_widget(state->search, "studio.tests.search");
    (void)umi_gtk4_automation_tag_widget(state->previous, "studio.tests.previous-page");
    (void)umi_gtk4_automation_tag_widget(state->next, "studio.tests.next-page");
    (void)umi_gtk4_automation_tag_widget(state->range, "studio.tests.page-range");
    (void)umi_gtk4_automation_tag_widget(state->first, "studio.tests.first-page");
    (void)umi_gtk4_automation_tag_widget(state->last, "studio.tests.last-page");
    runtime_bind_editor_action(runtime, state->first, RuntimeTestPageMove);
    runtime_bind_editor_action(runtime, state->last, RuntimeTestPageMove);
    runtime_bind_editor_action(runtime, state->previous, RuntimeTestPageMove);
    runtime_bind_editor_action(runtime, state->next, RuntimeTestPageMove);
    RuntimeTestPageBind(runtime, state->search, "changed", G_CALLBACK(RuntimeTestPageQueryChanged));
    RuntimeTestPageBind(runtime, state->failed, "toggled", G_CALLBACK(RuntimeTestPageQueryChanged));
    RuntimeTestPageBind(runtime, state->group, "notify::selected", G_CALLBACK(RuntimeTestPageChoiceChanged));
    RuntimeTestPageBind(runtime, state->sort, "notify::selected", G_CALLBACK(RuntimeTestPageChoiceChanged));
    return bar;
}
