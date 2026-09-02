/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gtk/workbench_main.c
 *
 * PURPOSE:
 *   Launch the Framework GTK4 reference adapter with the Studio workbench
 *   composition as an independent validation frontend.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include <gtk/gtk.h>

#include "umicom/studio/bootstrap.h"
#include "workbench_window.h"

static UmiStudioBootstrap *GLOBAL_BOOTSTRAP = NULL;
static UmiStudioGtkWorkbench *GLOBAL_WORKBENCH = NULL;

/* Provide the on activate operation used by this module and its client applications. */
static void on_activate(GtkApplication *application, gpointer user_data)
{
    UmiStatus status;
    (void)user_data;
    status = umi_studio_bootstrap_create(&GLOBAL_BOOTSTRAP);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) status = umi_studio_bootstrap_start(GLOBAL_BOOTSTRAP);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_gtk_workbench_create(application,
                                                  umi_studio_bootstrap_ui(GLOBAL_BOOTSTRAP),
                                                  umi_studio_bootstrap_desktop_shell(GLOBAL_BOOTSTRAP),
                                                  &GLOBAL_WORKBENCH);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        g_warning("Umicom Studio workbench failed: %s", umi_status_text(status));
        return;
    }
    gtk_window_present(umi_studio_gtk_workbench_window(GLOBAL_WORKBENCH));
}

/* Provide the on shutdown operation used by this module and its client applications. */
static void on_shutdown(GApplication *application, gpointer user_data)
{
    (void)application;
    (void)user_data;
    umi_studio_gtk_workbench_destroy(GLOBAL_WORKBENCH);
    GLOBAL_WORKBENCH = NULL;
    umi_studio_bootstrap_destroy(GLOBAL_BOOTSTRAP);
    GLOBAL_BOOTSTRAP = NULL;
}

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(int argc, char **argv)
{
    GtkApplication *application = gtk_application_new(
        "org.umicom.studio.workbench", G_APPLICATION_DEFAULT_FLAGS);
    int result;
    g_signal_connect(application, "activate", G_CALLBACK(on_activate), NULL);
    g_signal_connect(application, "shutdown", G_CALLBACK(on_shutdown), NULL);
    result = g_application_run(G_APPLICATION(application), argc, argv);
    g_object_unref(application);
    return result;
}
