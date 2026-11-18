/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_documentation_ownership_gtk4.c
 *
 * PURPOSE:
 *   Verify Documentation navigation, read-only search and retained control ownership without changing source drafts.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "workbench_window.h"
#include "document_baseline.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include "umicom/studio_runtime/workspace_canvas.h"
#include "umicom/ui/gtk4/automation.h"
#include "umicom/ui/document_view.h"
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "%d: %s\n", __LINE__, #c); failed = 1; goto cleanup; } } while (0)
#define ID(name) UMI_STUDIO_RUNTIME_WORKSPACE_PREFIX name


typedef struct DocumentationObserver {
    UmiStudioGtkWorkbench **owner; GtkWidget *open;
    const char *mode; unsigned calls;
} DocumentationObserver;
static void Observe(GObject *object, GParamSpec *property, gpointer data)
{
    (void)property;
    DocumentationObserver *p = data;
    if (p->calls++ != 0U || *p->owner == NULL) return;
    if (strstr(p->mode, "close")) {
        UmiStudioGtkWorkbench *owner = *p->owner; *p->owner = NULL;
        umi_studio_gtk_workbench_destroy(owner);
    } else if (strstr(p->mode, "replace")) {
        GtkTextBuffer *buffer = gtk_text_buffer_new(NULL);
        gtk_text_buffer_set_text(buffer, "Observer replacement", -1);
        gtk_text_view_set_buffer(GTK_TEXT_VIEW(object), buffer); g_object_unref(buffer);
    } else g_signal_emit_by_name(p->open, "clicked");
}
static void AddressChanged(GtkEditable *entry, gpointer data) { Observe(G_OBJECT(entry), NULL, data); }
static void SelectionChanged(GtkTextBuffer *buffer, const GtkTextIter *position, GtkTextMark *mark, gpointer data)
{ (void)position; (void)mark; Observe(G_OBJECT(buffer), NULL, data); }
static char *PreviewText(GtkWidget *view)
{
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    GtkTextIter start, end; gtk_text_buffer_get_bounds(buffer, &start, &end);
    return gtk_text_buffer_get_text(buffer, &start, &end, TRUE);
}
static int SelectedPhrase(GtkWidget *view, const char *phrase)
{
    GtkTextIter start, end;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    if (!gtk_text_buffer_get_selection_bounds(buffer, &start, &end)) return 0;
    char *text = gtk_text_buffer_get_text(buffer, &start, &end, TRUE);
    int equal = strcmp(text, phrase) == 0; g_free(text); return equal;
}
int main(int argc, char **argv)
{
    const char *cases[] = {"open", "activate", "file-uri", "back", "forward", "reload", "search", "search-empty", "search-missing", "relative-no-project", "invalid-uri", "remote", "retained-open", "retained-back", "retained-forward", "retained-reload", "retained-search", "address-close", "address-reenter", "buffer-close", "buffer-reenter", "label-close", "label-reenter", "buffer-replace", "selection-close", "selection-reenter"};
    if (argc != 2) return 2;
    const char *mode = argv[1]; int known = 0;
    for (size_t i = 0U; i < sizeof cases / sizeof cases[0]; ++i) if (!strcmp(mode, cases[i])) known = 1;
    if (!known) return 2;
    if (!gtk_init_check()) return 77;
    int failed = 0, moved = 0;
    StudioTestDocumentBaseline documentBaseline = {0};
    char *original = g_get_current_dir(), *base = NULL;
    char *old_data = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    char *output = NULL, *preview_text = NULL, *first = NULL, *second = NULL, *uri = NULL;
    char document_id[128] = {0}; size_t output_bytes = 0U;
    UmiSettings *settings = NULL; UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL; GtkApplication *application = NULL;
    GtkWidget *preview = NULL, *summary = NULL, *location = NULL, *search = NULL;
    GtkWidget *open = NULL, *back = NULL, *forward = NULL, *reload = NULL;
    GObject *observed = NULL; gulong handler = 0U;
    DocumentationObserver observer = {&workbench, NULL, mode, 0U};
    base = g_dir_make_tmp("umicom-documentation-XXXXXX", NULL);
    CHECK(base != NULL && g_chdir(base) == 0); moved = 1;
    g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    CHECK(g_mkdir_with_parents("config", 0700) == 0);
    CHECK(umi_studio_settings_create(&settings) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_ALLOW_REMOTE, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AUTO_SAVE, 0) == UMI_STATUS_OK);
    CHECK(umi_studio_settings_save(settings, umi_studio_settings_default_path()) == UMI_STATUS_OK);
    UmiStudioServicesOptions service_options = {0};
    CHECK(umi_studio_bootstrap_create_with_options(&service_options, &bootstrap) == UMI_STATUS_OK);
    UmiStudioUi *ui = umi_studio_bootstrap_ui(bootstrap);
    UmiDocumentCoordinator *documents = umi_studio_ui_documents(ui);
    UmiUiDocumentViewModel *views = umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
    application = gtk_application_new("org.umicom.studio.documentation-test", G_APPLICATION_NON_UNIQUE);
    CHECK(g_application_register(G_APPLICATION(application), NULL, NULL));
    UmiStudioGtkWorkbenchOptions options = {0};
    CHECK(umi_studio_gtk_workbench_create_with_options(application, ui,
        umi_studio_bootstrap_desktop_shell(bootstrap), &options, &workbench) == UMI_STATUS_OK);
    CHECK(umi_document_coordinator_new(documents, "draft.c", document_id, sizeof document_id) == UMI_STATUS_OK);
    UmiUiDocumentViewSnapshot view;
    const char *text = "int draft_value = 17;\n";
    CHECK(umi_ui_document_view_model_find(views, document_id, &view) == UMI_STATUS_OK);
    view.cursor_offset = 4U; view.selection_length = 6U; view.dirty = 1;
    CHECK(UmiUiDocumentViewModelUpsertText(views, &view, text, strlen(text)) == UMI_STATUS_OK);
    CHECK(StudioTestDocumentsCapture(documents, views, &documentBaseline) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_EDITOR, NULL) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);

    first = g_build_filename(base, "first.html", NULL);
    second = g_build_filename(base, "second.html", NULL);
    CHECK(g_file_set_contents(first, "<html><title>First lesson</title><body><p>Alpha beta lesson.</p></body></html>", -1, NULL));
    CHECK(g_file_set_contents(second, "<html><title>Second lesson</title><body><p>Second chapter.</p></body></html>", -1, NULL));
    GtkWidget *owner = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_DOCUMENTATION, NULL) == UMI_STATUS_OK);
#define RETAIN_TAG(variable, tag, predicate) do { GtkWidget *found = umi_gtk4_automation_find_tagged_widget(owner, tag); CHECK(predicate(found)); variable = g_object_ref(found); } while (0)
    RETAIN_TAG(preview, "studio.documentation.preview", GTK_IS_TEXT_VIEW);
    RETAIN_TAG(summary, "studio.documentation.summary", GTK_IS_LABEL);
    RETAIN_TAG(location, "studio.documentation.location", GTK_IS_EDITABLE);
    RETAIN_TAG(search, "studio.documentation.search", GTK_IS_SEARCH_ENTRY);
    RETAIN_TAG(open, "studio.documentation.open", GTK_IS_BUTTON);
    RETAIN_TAG(back, "studio.documentation.back", GTK_IS_BUTTON);
    RETAIN_TAG(forward, "studio.documentation.forward", GTK_IS_BUTTON);
    RETAIN_TAG(reload, "studio.documentation.reload", GTK_IS_BUTTON);
#undef RETAIN_TAG
    observer.open = open;
    if (!strcmp(mode, "relative-no-project")) {
        gtk_editable_set_text(GTK_EDITABLE(location), "first.html");
        g_signal_emit_by_name(open, "clicked");
        preview_text = PreviewText(preview);
        CHECK(strstr(preview_text, "Open a project") != NULL);
        goto inspect;
    }
    gtk_editable_set_text(GTK_EDITABLE(location), first);
    g_signal_emit_by_name(open, "clicked");
    preview_text = PreviewText(preview);
    CHECK(strstr(preview_text, "Alpha beta lesson.") != NULL);
    g_clear_pointer(&preview_text, g_free);
    if (!strncmp(mode, "retained-", 9U)) {
        umi_studio_gtk_workbench_destroy(workbench); workbench = NULL;
        GtkWidget *control = !strcmp(mode, "retained-open") ? open :
            !strcmp(mode, "retained-back") ? back : !strcmp(mode, "retained-forward") ? forward :
            !strcmp(mode, "retained-reload") ? reload : search;
        g_signal_emit_by_name(control, control == search ? "search-changed" : "clicked");
        goto inspect;
    }
    if (!strncmp(mode, "selection-", 10U)) {
        observed = g_object_ref(G_OBJECT(gtk_text_view_get_buffer(GTK_TEXT_VIEW(preview))));
        handler = g_signal_connect(observed, "mark-set", G_CALLBACK(SelectionChanged), &observer);
    }
    if (!strncmp(mode, "search", 6U) || !strncmp(mode, "selection-", 10U)) {
        gtk_editable_set_text(GTK_EDITABLE(search), "BETA");
        g_signal_emit_by_name(search, "search-changed");
        if (handler != 0U) {
            CHECK(observer.calls != 0U);
            g_signal_handler_disconnect(observed, handler); handler = 0U;
        }
        if (workbench == NULL) goto inspect;
        CHECK(SelectedPhrase(preview, "beta"));
        if (!strcmp(mode, "search-empty") || !strcmp(mode, "search-missing")) {
/* A whole-value replacement emits an intermediate empty-query signal. Exercise missing-text preservation without implicitly exercising the separate clear-selection operation. The former implementation is retained for owner review. */
#if 0
            gtk_editable_set_text(GTK_EDITABLE(search), !strcmp(mode, "search-empty") ? "" : "not in this lesson");
#endif
            if (!strcmp(mode, "search-empty"))
                gtk_editable_set_text(GTK_EDITABLE(search), "");
            else {
                /* Extend the query without first deleting it. GtkSearchEntry
                 * emits search-changed for the transient empty value inside
                 * set_text, which deliberately clears the old selection. This
                 * case isolates a nonempty miss rather than clear-then-search. */
                int position = -1;
                gtk_editable_insert_text(GTK_EDITABLE(search), " not in this lesson", -1, &position);
            }
            g_signal_emit_by_name(search, "search-changed");
            if (!strcmp(mode, "search-empty")) {
                GtkTextIter a, b; CHECK(!gtk_text_buffer_get_selection_bounds(gtk_text_view_get_buffer(GTK_TEXT_VIEW(preview)), &a, &b));
            } else CHECK(SelectedPhrase(preview, "beta"));
        }
        goto inspect;
    }
    if (!strcmp(mode, "invalid-uri")) {
        gtk_editable_set_text(GTK_EDITABLE(location), "file:///%zz");
        g_signal_emit_by_name(open, "clicked");
        preview_text = PreviewText(preview); CHECK(strstr(preview_text, "Alpha beta lesson.") != NULL);
        goto inspect;
    }
    if (!strcmp(mode, "remote")) {
        gtk_editable_set_text(GTK_EDITABLE(location), "https://example.invalid/lesson");
        g_signal_emit_by_name(open, "clicked");
        preview_text = PreviewText(preview); CHECK(strstr(preview_text, "local HTML documentation") != NULL);
        goto inspect;
    }
    if (!strcmp(mode, "reload")) {
        CHECK(g_file_set_contents(first, "<html><body>Reloaded lesson.</body></html>", -1, NULL));
        g_signal_emit_by_name(reload, "clicked");
        preview_text = PreviewText(preview); CHECK(strstr(preview_text, "Reloaded lesson.") != NULL);
        goto inspect;
    }
    gtk_editable_set_text(GTK_EDITABLE(location), second);
    if (!strncmp(mode, "address-", 8U)) {
        observed = g_object_ref(G_OBJECT(location));
        handler = g_signal_connect(location, "changed", G_CALLBACK(AddressChanged), &observer);
    } else if (!strncmp(mode, "buffer-", 7U)) {
        observed = g_object_ref(G_OBJECT(preview));
        handler = g_signal_connect(preview, "notify::buffer", G_CALLBACK(Observe), &observer);
    } else if (!strncmp(mode, "label-", 6U)) {
        observed = g_object_ref(G_OBJECT(summary));
        handler = g_signal_connect(summary, "notify::label", G_CALLBACK(Observe), &observer);
    }
    if (!strcmp(mode, "file-uri")) {
        uri = g_filename_to_uri(second, NULL, NULL); CHECK(uri != NULL);
        gtk_editable_set_text(GTK_EDITABLE(location), uri);
    }
    g_signal_emit_by_name(!strcmp(mode, "activate") ? location : open,
        !strcmp(mode, "activate") ? "activate" : "clicked");
    if (handler != 0U) {
        CHECK(observer.calls != 0U); g_signal_handler_disconnect(observed, handler); handler = 0U;
    }
    if (strstr(mode, "close")) { CHECK(workbench == NULL); goto inspect; }
    preview_text = PreviewText(preview);
    if (!strcmp(mode, "buffer-replace")) CHECK(!strcmp(preview_text, "Observer replacement"));
    else CHECK(strstr(preview_text, "Second chapter.") != NULL);
    g_clear_pointer(&preview_text, g_free);
    if (!strcmp(mode, "back") || !strcmp(mode, "forward")) {
        g_signal_emit_by_name(back, "clicked");
        preview_text = PreviewText(preview); CHECK(strstr(preview_text, "Alpha beta lesson.") != NULL);
        g_clear_pointer(&preview_text, g_free);
        if (!strcmp(mode, "forward")) {
            g_signal_emit_by_name(forward, "clicked");
            preview_text = PreviewText(preview); CHECK(strstr(preview_text, "Second chapter.") != NULL);
        }
    }
inspect:
    CHECK(UmiUiDocumentViewModelCopyText(views, document_id, &output, &output_bytes) == UMI_STATUS_OK);
    CHECK(output_bytes == strlen(text) && !memcmp(output, text, output_bytes));
    CHECK(umi_ui_document_view_model_find(views, document_id, &view) == UMI_STATUS_OK);
    CHECK(view.dirty && view.cursor_offset == 4U && view.selection_length == 6U);
/* Welcome is an adopted editable document; verify all original document identities and full text rather than assuming an initially empty coordinator. The former implementation is retained for owner review. */
#if 0
    CHECK(umi_document_coordinator_count(documents) == 1U);
#endif
    CHECK(StudioTestDocumentsUnchanged(documents, views, &documentBaseline));
cleanup:
    StudioTestDocumentsRelease(&documentBaseline);
    if (handler != 0U) g_signal_handler_disconnect(observed, handler);
    g_clear_object(&observed);
    UmiUiDocumentViewModelFreeText(output); g_free(preview_text);
    if (workbench != NULL) umi_studio_gtk_workbench_destroy(workbench);
    g_clear_object(&preview); g_clear_object(&summary); g_clear_object(&location); g_clear_object(&search);
    g_clear_object(&open); g_clear_object(&back); g_clear_object(&forward); g_clear_object(&reload);
    if (bootstrap != NULL) umi_studio_bootstrap_destroy(bootstrap);
    if (settings != NULL) umi_settings_destroy(settings);
    g_clear_object(&application);
    if (moved) (void)g_chdir(original);
    if (old_data != NULL) (void)g_setenv("UMICOM_STUDIO_DATA_PATH", old_data, TRUE);
    else g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    g_free(old_data); g_free(first); g_free(second); g_free(uri); g_free(base); g_free(original);
    return failed;
}
