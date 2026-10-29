/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_learning_locations_gtk4.c
 * PURPOSE: Check explicit lesson and project locations and the lifetime of learning controls.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/platform/session_store.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/learning_centre.h"
#include "umicom/studio/settings.h"
#include "umicom/studio/workspace.h"
#include "umicom/studio_runtime/workspace_canvas.h"
#include "umicom/teacher/learning_library.h"
#include "umicom/ui/gtk4/automation.h"
#include "workbench_window.h"
#ifdef UMICOM_STUDIO_HAS_LEARNING_CATALOGUE
#include "umicom/ui/gtk4/learning_catalogue.h"
#include "umicom/ui/gtk4/learning_help_binding.h"
#endif
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>

#define CHECK(condition)                                                                           \
    do                                                                                             \
    {                                                                                              \
        if (!(condition))                                                                          \
        {                                                                                          \
            fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition);                        \
            failed = 1;                                                                            \
            goto cleanup;                                                                          \
        }                                                                                          \
    } while (0)


#ifdef UMICOM_STUDIO_HAS_LEARNING_CATALOGUE
/* Wait for accepted catalogue jobs, including completion after a host closes.
 * A test deadline is failure evidence, never a reason to skip the assertion. */
static int FinishCatalogue(void)
{
    gint64 deadline = g_get_monotonic_time() + 15 * G_TIME_SPAN_SECOND;
    while (UmiLearningCatalogueGtk4PendingReads() != 0U && g_get_monotonic_time() < deadline) {
        while (g_main_context_iteration(NULL, FALSE)) {}
        g_usleep(1000U);
    }
    return UmiLearningCatalogueGtk4PendingReads() == 0U;
}
#endif

/* Construct the real product without presenting a window, starting tools,
 * restoring personal sessions or granting a temporary folder workspace trust. */
int main(int argc, char **argv)
{
    if (argc != 3)
        return 2;
    if (!gtk_init_check())
        return 77;
    int failed = 0, changed_directory = 0, isolated = 0;
    GError *error = NULL;
    char *original_directory = g_get_current_dir();
    char *original_data = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    char *directory = g_dir_make_tmp("umicom-learning-locations-XXXXXX", &error);
    char *project = NULL;
    char *document_path = NULL;
    char *displayed_text = NULL;
    GString *document_bytes = NULL;
    char library[UMI_PATH_CAPACITY] = {0};
    GtkApplication *application = NULL;
    GtkWindow *catalogue_window = NULL;
    GtkWidget *catalogue_button = NULL, *catalogue_panel = NULL;
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL;
    UmiSettings *settings = NULL;
    GtkWidget *retained[4] = {NULL, NULL, NULL, NULL};
    UmiStudioServicesOptions service_options = {0};
    UmiStudioGtkWorkbenchOptions options = {0};
    CHECK(original_directory && directory && g_chdir(directory) == 0);
    changed_directory = 1;
    g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    isolated = 1;
    CHECK(g_mkdir_with_parents("config", 0700) == 0);
    CHECK(umi_studio_settings_create(&settings) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_ALLOW_REMOTE, 0) ==
          UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS, 0) ==
          UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION, 0) ==
          UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AUTO_SAVE, 0) == UMI_STATUS_OK);
    CHECK(umi_studio_settings_save(settings, umi_studio_settings_default_path()) == UMI_STATUS_OK);
    CHECK(umi_studio_bootstrap_create_with_options(&service_options, &bootstrap) == UMI_STATUS_OK);
    UmiStudioServices *services = umi_studio_bootstrap_services(bootstrap);
    UmiStudioWorkspaceSnapshot workspace;
    CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK);
    CHECK(!workspace.graph.open);
    if (strcmp(argv[1], "first-lesson") == 0)
    {
        /* An explicit in-memory choice exercises the product path without
         * presenting a platform file chooser or writing personal settings. */
        CHECK(UmiLearningLibrarySelect(argv[2], library, sizeof(library)) == UMI_STATUS_OK);
        CHECK(umi_session_store_set(umi_studio_services_session(services), "learning.library",
                                    library) == UMI_STATUS_OK);
    }

    application =
        gtk_application_new("org.umicom.studio.learning-locations-test", G_APPLICATION_NON_UNIQUE);
    CHECK(application && g_application_register(G_APPLICATION(application), NULL, &error));
    CHECK(umi_studio_gtk_workbench_create_with_options(
              application, umi_studio_bootstrap_ui(bootstrap),
              umi_studio_bootstrap_desktop_shell(bootstrap), &options,
              &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_LEARNING,
                                                          NULL) == UMI_STATUS_OK);
    GtkWidget *root = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    GtkWidget *label = umi_gtk4_automation_find_tagged_widget(root, "studio.learning.library");
    CHECK(GTK_IS_LABEL(label));
    if (strcmp(argv[1], "first-lesson") != 0)
    {
        CHECK(strstr(gtk_label_get_text(GTK_LABEL(label)), "Choose the folder") != NULL);
    }
    else
    {
        CHECK(umi_path_equal(gtk_label_get_text(GTK_LABEL(label)), library));
    }
    const char *actions[] = {"studio.learning.choose-library", "studio.learning.create-project",
                             "studio.learning.open-project", "studio.learning.reference"};
    for (size_t i = 0U; i < 4U; ++i)
    {
        GtkWidget *button = umi_gtk4_automation_find_tagged_widget(root, actions[i]);
        CHECK(GTK_IS_BUTTON(button));
        retained[i] = g_object_ref(button);
    }
    if (strcmp(argv[1], "first-lesson") == 0)
    {
        g_signal_emit_by_name(retained[3], "clicked");
        GtkWidget *preview =
            umi_gtk4_automation_find_tagged_widget(root, "studio.documentation.preview");
        GtkWidget *location =
            umi_gtk4_automation_find_tagged_widget(root, "studio.documentation.location");
        CHECK(GTK_IS_TEXT_VIEW(preview) && GTK_IS_ENTRY(location));
        CHECK(gtk_text_buffer_get_char_count(gtk_text_view_get_buffer(GTK_TEXT_VIEW(preview))) > 0);
        UmiStudioLearningLessonSnapshot lesson;
        char expected[UMI_PATH_CAPACITY];
        CHECK(umi_studio_learning_centre_find_lesson("foundations.c-hello", &lesson) ==
              UMI_STATUS_OK);
        CHECK(UmiLearningLibraryResource(library, lesson.resource_path, expected,
                                         sizeof(expected)) == UMI_STATUS_OK);
        char *actual =
            g_filename_from_uri(gtk_editable_get_text(GTK_EDITABLE(location)), NULL, NULL);
        int same = actual != NULL && umi_path_equal(actual, expected);
        g_free(actual);
        CHECK(same);
        CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK);
        CHECK(!workspace.graph.open);
    }
    else if (strcmp(argv[1], "relative-without-project") == 0)
    {
        CHECK(umi_studio_gtk_workbench_workspace_open_surface(
                  workbench, UMI_STUDIO_SURFACE_DOCUMENTATION, NULL) == UMI_STATUS_OK);
        GtkWidget *location =
            umi_gtk4_automation_find_tagged_widget(root, "studio.documentation.location");
        GtkWidget *open = umi_gtk4_automation_find_tagged_widget(root, "studio.documentation.open");
        GtkWidget *notice =
            umi_gtk4_automation_find_tagged_widget(root, "studio.documentation.summary");
        CHECK(GTK_IS_ENTRY(location) && GTK_IS_BUTTON(open) && GTK_IS_LABEL(notice));
        gtk_editable_set_text(GTK_EDITABLE(location), "welcome.html");
        g_signal_emit_by_name(open, "clicked");
        CHECK(strcmp(gtk_label_get_text(GTK_LABEL(notice)), "Choose a document location") == 0);
        CHECK(strcmp(gtk_editable_get_text(GTK_EDITABLE(location)), "welcome.html") == 0);
    }
    else if (g_str_has_prefix(argv[1], "document-"))
    {
        /* Exercise the actual documentation surface with synthetic files.
         * The input budget and presentation encoding must both survive the
         * boundary from Framework bytes to native GTK text. */
        document_path = g_build_filename(directory, "lesson.html", NULL);
        document_bytes = g_string_new(NULL);
        CHECK(document_path != NULL && document_bytes != NULL);
        if (strcmp(argv[1], "document-utf8") == 0)
        {
            g_string_append(document_bytes, "<title>");
            for (size_t i = 0U; i < 255U; ++i)
                g_string_append_c(document_bytes, (char)0xff);
            g_string_append(document_bytes, "</title><p>Readable body</p>");
        }
        else if (strcmp(argv[1], "document-overflow") == 0)
        {
            g_string_set_size(document_bytes, 4U * 1024U * 1024U + 1U);
            memset(document_bytes->str, 'x', document_bytes->len);
        }
        else if (strcmp(argv[1], "document-binary") == 0)
        {
            static const char binary[] = "<p>Before\0After</p>";
            g_string_append_len(document_bytes, binary, (gssize)(sizeof(binary) - 1U));
        }
        else
        {
            CHECK(0 && "Unknown document boundary case");
        }
        CHECK(g_file_set_contents(document_path, document_bytes->str,
                                  (gssize)document_bytes->len, &error));
        CHECK(umi_studio_gtk_workbench_workspace_open_surface(
                  workbench, UMI_STUDIO_SURFACE_DOCUMENTATION, NULL) == UMI_STATUS_OK);
        GtkWidget *location =
            umi_gtk4_automation_find_tagged_widget(root, "studio.documentation.location");
        GtkWidget *open =
            umi_gtk4_automation_find_tagged_widget(root, "studio.documentation.open");
        GtkWidget *notice =
            umi_gtk4_automation_find_tagged_widget(root, "studio.documentation.summary");
        GtkWidget *preview =
            umi_gtk4_automation_find_tagged_widget(root, "studio.documentation.preview");
        CHECK(GTK_IS_ENTRY(location) && GTK_IS_BUTTON(open) &&
              GTK_IS_LABEL(notice) && GTK_IS_TEXT_VIEW(preview));
        gtk_editable_set_text(GTK_EDITABLE(location), document_path);
        g_signal_emit_by_name(open, "clicked");
        const char *summary = gtk_label_get_text(GTK_LABEL(notice));
        CHECK(g_utf8_validate(summary, -1, NULL));
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(preview));
        GtkTextIter first, last;
        gtk_text_buffer_get_bounds(buffer, &first, &last);
        displayed_text = gtk_text_buffer_get_text(buffer, &first, &last, FALSE);
        CHECK(displayed_text != NULL && g_utf8_validate(displayed_text, -1, NULL));
        if (strcmp(argv[1], "document-utf8") == 0)
        {
            /* The repaired title alone is 765 bytes. A 512-byte snprintf
             * would cut its UTF-8 and lose the useful source-size suffix. */
            CHECK(strlen(summary) > 765U);
            CHECK(strstr(summary, "source bytes") != NULL);
            CHECK(strstr(displayed_text, "Readable body") != NULL);
        }
        else if (strcmp(argv[1], "document-overflow") == 0)
        {
            CHECK(strcmp(summary, "Document is larger than the safe preview limit") == 0);
        }
        else
        {
            CHECK(strcmp(summary, "Document could not be previewed") == 0);
        }
        CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK);
        CHECK(!workspace.graph.open);
    }
#ifdef UMICOM_STUDIO_HAS_LEARNING_CATALOGUE
    else if (g_str_has_prefix(argv[1],"catalogue-topic-"))
    {
        const char *mode=argv[1];
        const char *tag=strcmp(mode,"catalogue-topic-errors")==0?"studio.learning.errors-guide":
            strcmp(mode,"catalogue-topic-tests")==0?"studio.learning.tests-guide":"studio.learning.editing-guide";
        const char *resource=strcmp(mode,"catalogue-topic-errors")==0?"reading-build-errors":
            strcmp(mode,"catalogue-topic-tests")==0?"running-tests":"editing-files";
        CHECK(strcmp(mode,"catalogue-topic-editing")==0 || strcmp(mode,"catalogue-topic-errors")==0 ||
            strcmp(mode,"catalogue-topic-tests")==0 || strcmp(mode,"catalogue-topic-switch")==0 ||
            strcmp(mode,"catalogue-topic-busy")==0 || strcmp(mode,"catalogue-topic-retained")==0);
        catalogue_button=umi_gtk4_automation_find_tagged_widget(root,tag);
        CHECK(GTK_IS_BUTTON(catalogue_button));g_object_ref(catalogue_button);
        if(strcmp(mode,"catalogue-topic-retained")==0) {
            umi_studio_gtk_workbench_destroy(workbench);workbench=NULL;
            umi_studio_bootstrap_destroy(bootstrap);bootstrap=NULL;
            guint count=g_list_model_get_n_items(gtk_window_get_toplevels());
            g_signal_emit_by_name(catalogue_button,"clicked");
            CHECK(g_list_model_get_n_items(gtk_window_get_toplevels())==count);
        } else {
            g_signal_emit_by_name(catalogue_button,"clicked");
            for(GList *item=gtk_application_get_windows(application);item!=NULL;item=item->next) {
                GtkWindow *candidate=item->data;
                GtkWidget *panel=umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(candidate),"learning.catalogue");
                if(panel!=NULL) {
                    catalogue_window=g_object_ref(candidate);catalogue_panel=g_object_ref(panel);break;
                }
            }
            CHECK(catalogue_panel!=NULL && catalogue_window!=NULL);
            UmiLearningCatalogueGtkState state;
            CHECK(UmiLearningCatalogueGtk4CopyState(catalogue_panel,&state)==UMI_STATUS_OK &&
                state.last_status==UMI_STATUS_NOT_FOUND && !state.reader.has_document);
            CHECK(UmiLearningCatalogueGtk4Load(catalogue_panel,argv[2])==UMI_STATUS_OK && FinishCatalogue());
            g_signal_emit_by_name(catalogue_button,"clicked");
            GtkWidget *errors=umi_gtk4_automation_find_tagged_widget(root,"studio.learning.errors-guide");
            CHECK(GTK_IS_BUTTON(errors));
            if(strcmp(mode,"catalogue-topic-busy")==0) {
                CHECK(UmiLearningCatalogueGtk4CopyState(catalogue_panel,&state)==UMI_STATUS_OK && state.busy);
                g_signal_emit_by_name(errors,"clicked");
            }
            CHECK(FinishCatalogue());
            CHECK(UmiLearningCatalogueGtk4CopyState(catalogue_panel,&state)==UMI_STATUS_OK &&
                state.last_status==UMI_STATUS_OK && state.reader.has_document);
            CHECK(strcmp(state.reader.selection.resource.id,resource)==0);
            if(strcmp(mode,"catalogue-topic-switch")==0) {
                guint count=g_list_model_get_n_items(gtk_window_get_toplevels());
                g_signal_emit_by_name(errors,"clicked");CHECK(FinishCatalogue());
                CHECK(g_list_model_get_n_items(gtk_window_get_toplevels())==count);
                CHECK(UmiLearningCatalogueGtk4CopyState(catalogue_panel,&state)==UMI_STATUS_OK &&
                    strcmp(state.reader.selection.resource.id,"reading-build-errors")==0);
                CHECK(UmiLearningCatalogueGtk4Back(catalogue_panel)==UMI_STATUS_OK && FinishCatalogue());
                CHECK(UmiLearningCatalogueGtk4CopyState(catalogue_panel,&state)==UMI_STATUS_OK &&
                    strcmp(state.reader.selection.resource.id,"editing-files")==0);
            }
            UmiLearningHelpGtkState help;
            CHECK(UmiLearningHelpGtk4CopyState(catalogue_panel,&help)==UMI_STATUS_OK &&
                help.attempts==0U && strcmp(help.request.key,"learning")==0);
            CHECK(umi_studio_workspace_snapshot(services,&workspace)==UMI_STATUS_OK && !workspace.graph.open);
        }
    }
    else if (strcmp(argv[1], "catalogue") == 0 || strcmp(argv[1], "catalogue-retained") == 0 ||
             strcmp(argv[1], "catalogue-guide") == 0 || strcmp(argv[1], "catalogue-guide-retained") == 0)
    {
        catalogue_button = umi_gtk4_automation_find_tagged_widget(root,
            g_str_has_prefix(argv[1], "catalogue-guide") ? "studio.learning.guide" : "studio.learning.catalogue");
        CHECK(GTK_IS_BUTTON(catalogue_button));
        g_object_ref(catalogue_button);
        if (strcmp(argv[1], "catalogue-retained") == 0 || strcmp(argv[1], "catalogue-guide-retained") == 0)
        {
            umi_studio_gtk_workbench_destroy(workbench);
            workbench = NULL;
            umi_studio_bootstrap_destroy(bootstrap);
            bootstrap = NULL;
            guint before = g_list_model_get_n_items(gtk_window_get_toplevels());
            g_signal_emit_by_name(catalogue_button, "clicked");
            CHECK(g_list_model_get_n_items(gtk_window_get_toplevels()) == before);
        }
        else
        {
            /* This case presents the real reader through the product action,
             * but neither chooses a file nor starts a project or a command. */
            g_signal_emit_by_name(catalogue_button, "clicked");
            for (GList *item = gtk_application_get_windows(application); item != NULL; item = item->next)
            {
                GtkWindow *candidate = item->data;
                GtkWidget *panel = umi_gtk4_automation_find_tagged_widget(
                    GTK_WIDGET(candidate), "learning.catalogue");
                if (panel != NULL)
                {
                    catalogue_window = g_object_ref(candidate);
                    catalogue_panel = g_object_ref(panel);
                    break;
                }
            }
            CHECK(catalogue_window != NULL && catalogue_panel != NULL);
            CHECK(gtk_window_get_transient_for(catalogue_window) == GTK_WINDOW(root));
            UmiLearningCatalogueGtkState state;
            CHECK(UmiLearningCatalogueGtk4CopyState(catalogue_panel, &state) == UMI_STATUS_OK);
            CHECK(!state.busy && !state.retired && state.pack_count == 0U);
            if (strcmp(argv[1], "catalogue-guide") == 0) {
                CHECK(state.last_status == UMI_STATUS_NOT_FOUND);
                GtkWidget *notice = umi_gtk4_automation_find_tagged_widget(
                    catalogue_panel, "learning.catalogue.status");
                CHECK(GTK_IS_LABEL(notice));
                CHECK(strstr(gtk_label_get_text(GTK_LABEL(notice)), "matching local learning pack") != NULL);
                UmiLearningHelpGtkState help;
                CHECK(UmiLearningHelpGtk4CopyState(catalogue_panel, &help) == UMI_STATUS_OK);
                CHECK(strcmp(help.request.product, "studio") == 0 &&
                    strcmp(help.request.key, "learning") == 0 && help.attempts == 1U);
                CHECK(UmiLearningCatalogueGtk4Load(catalogue_panel, argv[2]) == UMI_STATUS_OK);
                CHECK(FinishCatalogue());
                g_signal_emit_by_name(catalogue_button, "clicked");
                CHECK(FinishCatalogue());
                CHECK(UmiLearningCatalogueGtk4CopyState(catalogue_panel, &state) == UMI_STATUS_OK);
                CHECK(state.last_status == UMI_STATUS_OK && state.reader.has_document);
                CHECK(strcmp(state.reader.pack.id, "studio.learning-guide") == 0);
                CHECK(strcmp(state.reader.selection.resource.id, "learning-in-studio") == 0);
                CHECK(UmiLearningHelpGtk4CopyState(catalogue_panel, &help) == UMI_STATUS_OK &&
                    help.attempts == 2U && !help.dispatching);
            }

            /* A second product action must reuse the same live catalogue,
             * without creating a second empty learning window. */
            guint catalogue_windows = g_list_model_get_n_items(gtk_window_get_toplevels());
            g_signal_emit_by_name(catalogue_button, "clicked");
            CHECK(g_list_model_get_n_items(gtk_window_get_toplevels()) == catalogue_windows);
            CHECK(gtk_window_get_child(catalogue_window) == catalogue_panel);
            CHECK(FinishCatalogue());
            CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK);
            CHECK(!workspace.graph.open);
            umi_studio_gtk_workbench_destroy(workbench);
            workbench = NULL;
            CHECK(UmiLearningCatalogueGtk4CopyState(catalogue_panel, &state) == UMI_STATUS_OK);
            CHECK(state.retired);
            CHECK(UmiLearningCatalogueGtk4PendingReads() == 0U);
        }
    }
#endif
    else if (strcmp(argv[1], "controls") == 0)
    {
        CHECK(!gtk_widget_get_visible(root));
        CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK);
        CHECK(!workspace.graph.open);
    }
    else if (strcmp(argv[1], "project-separation") == 0)
    {
        project = g_build_filename(directory, "practice", NULL);
        CHECK(project && g_mkdir(project, 0700) == 0);
        CHECK(umi_studio_workspace_open(services, project, 0, 0) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
        CHECK(umi_studio_workspace_snapshot(services, &workspace) == UMI_STATUS_OK);
        CHECK(workspace.graph.open);
        /* Selecting a project does not invent or replace the lesson library. */
        label = umi_gtk4_automation_find_tagged_widget(root, "studio.learning.library");
        CHECK(GTK_IS_LABEL(label));
        CHECK(strstr(gtk_label_get_text(GTK_LABEL(label)), "Choose the folder") != NULL);
        CHECK(umi_studio_workspace_close(services) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    }
    else if (strcmp(argv[1], "retained-controls") == 0)
    {
        umi_studio_gtk_workbench_destroy(workbench);
        workbench = NULL;
        umi_studio_bootstrap_destroy(bootstrap);
        bootstrap = NULL;
        guint before = g_list_model_get_n_items(gtk_window_get_toplevels());
        for (size_t i = 0U; i < 4U; ++i)
            g_signal_emit_by_name(retained[i], "clicked");
        CHECK(g_list_model_get_n_items(gtk_window_get_toplevels()) == before);
    }
    else
    {
        failed = 2;
    }
cleanup:
#ifdef UMICOM_STUDIO_HAS_LEARNING_CATALOGUE
    if (catalogue_panel != NULL) UmiLearningCatalogueGtk4Retire(catalogue_panel);
    if (!FinishCatalogue()) failed = 1;
#endif
    if (catalogue_window != NULL) gtk_window_destroy(catalogue_window);
    g_clear_object(&catalogue_window);
    g_clear_object(&catalogue_panel);
    g_clear_object(&catalogue_button);
    for (size_t i = 0U; i < 4U; ++i)
        g_clear_object(&retained[i]);
    umi_studio_gtk_workbench_destroy(workbench);
    umi_studio_bootstrap_destroy(bootstrap);
    umi_settings_destroy(settings);
    g_clear_object(&application);
    if (changed_directory && g_chdir(original_directory) != 0)
        failed = 1;
    if (isolated)
    {
        if (original_data != NULL)
        {
            if (!g_setenv("UMICOM_STUDIO_DATA_PATH", original_data, TRUE))
                failed = 1;
        }
        else
            g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    }
    if (error != NULL)
        fprintf(stderr, "%s\n", error->message);
    if (directory != NULL)
        printf("Retained isolated learning fixture: %s\n", directory);
    g_clear_error(&error);
    if (document_bytes != NULL)
        g_string_free(document_bytes, TRUE);
    g_free(document_path);
    g_free(displayed_text);
    g_free(project);
    g_free(directory);
    g_free(original_directory);
    g_free(original_data);
    return failed;
}
