/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_live_output_gtk4.c
 * PURPOSE: Check Studio composes live output beside the existing completed result view.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/ui/gtk4/automation.h"
#include "workbench_window.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/build.h"
#include "umicom/studio/settings.h"
#include "umicom/studio_runtime/workspace_canvas.h"
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); failed = 1; goto cleanup; } } while (0)
/* Keep the host test independent of translated labels and GTK child order. */
/* The rendered-child walk omitted controls owned by collapsed expanders. The shared bounded logical-tree lookup replaces it; retain the earlier traversal for review. */
#if 0
static GtkWidget *Find(GtkWidget *root, const char *tag)
{
    if (root == NULL) return NULL;
    const char *actual = g_object_get_data(G_OBJECT(root), "umicom-automation-id");
    if (actual != NULL && strcmp(actual, tag) == 0) return root;
    for (GtkWidget *child = gtk_widget_get_first_child(root); child != NULL; child = gtk_widget_get_next_sibling(child)) {
        GtkWidget *found = Find(child, tag); if (found != NULL) return found;
    }
    return NULL;
}
#endif
/* Use the Framework logical tree so a collapsed panel can be inspected
 * without changing the user's layout or overlooking an ambiguous identifier. */
static GtkWidget *Find(GtkWidget *root, const char *tag)
{
    return umi_gtk4_automation_find_tagged_widget(root, tag);
}
/* Isolate settings and evidence before starting the native host. This fixture
 * never submits a build, launches a compiler or opens a user's project. */
int main(int argc, char **argv)
{
    if (argc != 2) return 2;
    int failed = 0, changedDirectory = 0, isolated = 0;
    GtkApplication *application = NULL; UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioGtkWorkbench *workbench = NULL; UmiSettings *settings = NULL;
    UmiStudioServicesOptions serviceOptions = {0}; UmiStudioGtkWorkbenchOptions options = {0};
    UmiBuildResult *result = NULL; GtkWidget *retained = NULL;
    char *directory = NULL, *originalDirectory = NULL, *originalData = NULL; GError *error = NULL;
    (void)g_setenv("GTK_A11Y", "test", TRUE);
    if (!gtk_init_check()) return 77;
    originalDirectory = g_get_current_dir(); originalData = g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    directory = g_dir_make_tmp("umicom-live-output-XXXXXX", &error);
    CHECK(originalDirectory != NULL && directory != NULL && g_chdir(directory) == 0); changedDirectory = 1;
    g_unsetenv("UMICOM_STUDIO_DATA_PATH"); isolated = 1;
    CHECK(g_mkdir_with_parents("config", 0700) == 0);
    CHECK(umi_studio_settings_create(&settings) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_ALLOW_REMOTE, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION, 0) == UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AUTO_SAVE, 0) == UMI_STATUS_OK);
    CHECK(umi_studio_settings_save(settings, umi_studio_settings_default_path()) == UMI_STATUS_OK);
    CHECK(umi_studio_bootstrap_create_with_options(&serviceOptions, &bootstrap) == UMI_STATUS_OK);
    application = gtk_application_new("org.umicom.studio.live-output-test", G_APPLICATION_NON_UNIQUE);
    CHECK(application != NULL && g_application_register(G_APPLICATION(application), NULL, &error));
    CHECK(umi_studio_gtk_workbench_create_with_options(application, umi_studio_bootstrap_ui(bootstrap),
        umi_studio_bootstrap_desktop_shell(bootstrap), &options, &workbench) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench, UMI_STUDIO_SURFACE_OUTPUT, NULL) == UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
    GtkWidget *root = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    GtkWidget *refresh = Find(root, "build.output.refresh");
    CHECK(GTK_IS_BUTTON(refresh) && GTK_IS_CHECK_BUTTON(Find(root, "build.output.follow")));
    CHECK(GTK_IS_TEXT_VIEW(Find(root, "build.output.text")));
    CHECK(GTK_IS_TEXT_VIEW(Find(root, "studio.build.output")));
    UmiStudioBuildService *build = umi_studio_services_build(umi_studio_bootstrap_services(bootstrap));
    CHECK(!UmiStudioBuildBusy(build));
    if (strcmp(argv[1], "history") == 0) {
        CHECK(umi_build_result_create(&result) == UMI_STATUS_OK);
        umi_build_result_init(result, 1U, UMI_BUILD_PHASE_BUILD, "fixture");
        strcpy(result->command, "inert fixture"); strcpy(result->output, "completed evidence");
        umi_build_result_finish(result, UMI_STATUS_OK, 0, 1U);
        CHECK(umi_build_history_append(umi_studio_build_service_history(build), result) == UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(Find(root, "studio.build.output")));
        GtkTextIter start, end; gtk_text_buffer_get_bounds(buffer, &start, &end);
        char *text = gtk_text_buffer_get_text(buffer, &start, &end, TRUE);
        int contains = strstr(text, "completed evidence") != NULL; g_free(text); CHECK(contains);
        CHECK(!UmiStudioBuildBusy(build));
    } else if (strcmp(argv[1], "retained") == 0) {
        retained = g_object_ref(refresh);
        umi_studio_gtk_workbench_destroy(workbench); workbench = NULL;
        umi_studio_bootstrap_destroy(bootstrap); bootstrap = NULL;
        g_signal_emit_by_name(retained, "clicked");
    } else CHECK(strcmp(argv[1], "surface") == 0);
cleanup:
    if (retained != NULL) g_object_unref(retained);
    umi_build_result_destroy(result); umi_studio_gtk_workbench_destroy(workbench);
    umi_studio_bootstrap_destroy(bootstrap); umi_settings_destroy(settings); g_clear_object(&application);
    if (changedDirectory && g_chdir(originalDirectory) != 0) failed = 1;
    if (isolated) {
        if (originalData != NULL) { if (!g_setenv("UMICOM_STUDIO_DATA_PATH", originalData, TRUE)) failed = 1; }
        else g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    }
    if (error != NULL) fprintf(stderr, "%s\n", error->message);
    if (directory != NULL) printf("Retained isolated output fixture: %s\n", directory);
    g_clear_error(&error); g_free(directory); g_free(originalDirectory); g_free(originalData); return failed;
}
