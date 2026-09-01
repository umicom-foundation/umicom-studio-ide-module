/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gtk/main.c
 *
 * PURPOSE:
 *   Start the GTK4 Umicom Studio IDE frontend inside the Umicom Framework
 *   lifecycle. The Framework-owned workbench is the default while the
 *   established product frontend remains available behind --legacy-ui.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * Normal startup presents the reusable Framework workbench. --legacy-ui keeps
 * the older product shell available during migration and comparison testing.
 */

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app.h"
#include "splash.h"
#include "workbench_window.h"
#include "umicom/studio/appearance_centre.h"
#include "umicom/studio/bootstrap.h"

static int str_eq(const char *a, const char *b)
{
    return a != NULL && b != NULL && strcmp(a, b) == 0;
}

static void configure_runtime_branding(UmiStudioBootstrap *bootstrap,
                                       const char *program_path)
{
    char *absolute_program;
    char *program_directory;
    char *branding_directory;
    char *logo_path;
    char *icon_path;
    UmiStudioUi *ui;

    if (bootstrap == NULL || program_path == NULL) return;
    absolute_program = g_canonicalize_filename(program_path, NULL);
    if (absolute_program == NULL) return;
    program_directory = g_path_get_dirname(absolute_program);
    branding_directory = g_build_filename(program_directory, "branding", NULL);
    logo_path = g_build_filename(
        branding_directory, "umicom-logo.svg", NULL);
    icon_path = g_build_filename(
        branding_directory, "umicom-icon.svg", NULL);
    ui = umi_studio_bootstrap_ui(bootstrap);
    if (ui != NULL && logo_path != NULL && icon_path != NULL &&
        g_file_test(logo_path, G_FILE_TEST_IS_REGULAR) &&
        g_file_test(icon_path, G_FILE_TEST_IS_REGULAR)) {
        (void)umi_studio_appearance_set_brand_resources(
            umi_studio_ui_workbench(ui), logo_path, icon_path);
    }
    g_free(icon_path);
    g_free(logo_path);
    g_free(branding_directory);
    g_free(program_directory);
    g_free(absolute_program);
}

static void log_line(const char *text)
{
    (void)fprintf(stderr, "%s\n", text != NULL ? text : "");
}

static gboolean on_bare_close(GtkWindow *window, gpointer user_data)
{
    GMainLoop *loop = (GMainLoop *)user_data;
    (void)window;
    if (loop != NULL) g_main_loop_quit(loop);
    return FALSE;
}

static int run_bare_gtk(void)
{
    GMainLoop *loop;
    GtkWindow *window;
    GtkWidget *label;

    log_line("[USIDE] bare GTK validation selected");
    gtk_init();
    window = GTK_WINDOW(gtk_window_new());
    gtk_window_set_title(window, "Umicom Studio IDE - bare GTK validation");
    gtk_window_set_default_size(window, 640, 400);
    label = gtk_label_new("GTK4 is working. Umicom Framework is running.");
    gtk_window_set_child(window, label);
    loop = g_main_loop_new(NULL, FALSE);
    g_signal_connect(window, "close-request", G_CALLBACK(on_bare_close), loop);
    gtk_window_present(window);
    g_main_loop_run(loop);
    g_main_loop_unref(loop);
    return 0;
}

static void on_test_activate(GtkApplication *application, gpointer user_data)
{
    GtkWindow *window;
    GtkWidget *label;
    (void)user_data;
    window = GTK_WINDOW(gtk_application_window_new(application));
    gtk_window_set_title(window, "Umicom Studio IDE - test window");
    gtk_window_set_default_size(window, 640, 400);
    label = gtk_label_new("GtkApplication and Umicom Framework are working.");
    gtk_window_set_child(window, label);
    gtk_window_present(window);
}

static int run_test_window_app(int argc, char **argv)
{
    GtkApplication *application = gtk_application_new(
        "org.umicom.studio.test-window",
        G_APPLICATION_NON_UNIQUE
    );
    int result;
    g_signal_connect(application, "activate", G_CALLBACK(on_test_activate), NULL);
    result = g_application_run(G_APPLICATION(application), argc, argv);
    g_object_unref(application);
    return result;
}

typedef struct UmiStudioFrameworkWorkbenchRun {
    UmiStudioBootstrap *bootstrap;
    UmiStudioGtkWorkbench *workbench;
    UmiSplash *splash;
    int create_failed;
} UmiStudioFrameworkWorkbenchRun;

static void on_framework_workbench_activate(GtkApplication *application,
                                            gpointer user_data)
{
    UmiStudioFrameworkWorkbenchRun *run =
        (UmiStudioFrameworkWorkbenchRun *)user_data;
    UmiStatus status;

    if (run == NULL || run->bootstrap == NULL || run->workbench != NULL) return;
    umi_splash_set_progress(
        run->splash, 0.82, "Composing the professional workbench…");
    status = umi_studio_gtk_workbench_create(
        application,
        umi_studio_bootstrap_ui(run->bootstrap),
        umi_studio_bootstrap_desktop_shell(run->bootstrap),
        &run->workbench);
    if (status != UMI_STATUS_OK) {
        (void)fprintf(stderr,
                      "[USIDE] Framework workbench create failed: %s\n",
                      umi_status_text(status));
        run->create_failed = 1;
        g_application_quit(G_APPLICATION(application));
        return;
    }

    umi_splash_set_progress(run->splash, 1.0, "Studio is ready");
    umi_splash_close(run->splash);
    umi_splash_free(run->splash);
    run->splash = NULL;
}

static int run_framework_workbench(UmiStudioBootstrap *bootstrap,
                                   UmiSplash *splash,
                                   int argc,
                                   char **argv)
{
    GtkApplication *application;
    UmiStudioFrameworkWorkbenchRun run = {0};
    int result;

    run.bootstrap = bootstrap;
    run.splash = splash;
    application = gtk_application_new(
        "org.umicom.studio.framework-workbench",
        G_APPLICATION_NON_UNIQUE);
    if (application == NULL) {
        umi_splash_free(splash);
        return 1;
    }

    g_signal_connect(application,
                     "activate",
                     G_CALLBACK(on_framework_workbench_activate),
                     &run);
    result = g_application_run(G_APPLICATION(application), argc, argv);
    umi_splash_free(run.splash);
    umi_studio_gtk_workbench_destroy(run.workbench);
    g_object_unref(application);
    return run.create_failed ? 1 : result;
}

static int filter_dev_flags(int argc, char **argv, char ***out_argv)
{
    char **filtered;
    int count = 0;
    int index;
    if (out_argv == NULL) return 0;
    filtered = (char **)malloc((size_t)(argc + 1) * sizeof(*filtered));
    if (filtered == NULL) {
        *out_argv = NULL;
        return 0;
    }
    filtered[count++] = argv[0];
    for (index = 1; index < argc; ++index) {
        if (str_eq(argv[index], "--console") ||
            str_eq(argv[index], "--dev") ||
            str_eq(argv[index], "--framework-workbench") ||
            str_eq(argv[index], "--legacy-ui")) {
            continue;
        }
        filtered[count++] = argv[index];
    }
    filtered[count] = NULL;
    *out_argv = filtered;
    return count;
}

static int run_studio(
    UmiStudioBootstrap *bootstrap,
    UmiSplash *splash,
    int argc,
    char **argv)
{
    char **filtered_argv = NULL;
    int filtered_argc;
    int index;
    GtkApplication *application;
    int result;

    for (index = 0; index < argc; ++index) {
        if (str_eq(argv[index], "--bare-gtk")) {
            umi_splash_free(splash);
            return run_bare_gtk();
        }
        if (str_eq(argv[index], "--test-window")) {
            umi_splash_free(splash);
            return run_test_window_app(argc, argv);
        }
        if (str_eq(argv[index], "--legacy-ui")) {
            filtered_argc = filter_dev_flags(argc, argv, &filtered_argv);
            if (filtered_argv == NULL) {
                umi_splash_free(splash);
                return 1;
            }
            application = umi_app_new();
            if (application == NULL) {
                free(filtered_argv);
                umi_splash_free(splash);
                return 1;
            }
            umi_splash_close(splash);
            umi_splash_free(splash);
            result = g_application_run(G_APPLICATION(application),
                                       filtered_argc,
                                       filtered_argv);
            g_object_unref(application);
            free(filtered_argv);
            return result;
        }
    }

    filtered_argc = filter_dev_flags(argc, argv, &filtered_argv);
    if (filtered_argv == NULL) {
        umi_splash_free(splash);
        return 1;
    }

    result = run_framework_workbench(bootstrap,
                                     splash,
                                     filtered_argc,
                                     filtered_argv);
    free(filtered_argv);
    return result;
}

static void flush_startup_presentation(void)
{
    while (g_main_context_pending(NULL)) {
        (void)g_main_context_iteration(NULL, FALSE);
    }
}

static UmiSplash *show_startup_splash(const char *program_path)
{
    UmiSplash *splash;
    char *absolute_program;
    char *program_directory;
    char *logo_path;

    gtk_init();
    splash = umi_splash_new(
        "Umicom Studio IDE",
        "Starting Umicom Framework…",
        0U);
    if (splash == NULL) return NULL;

    absolute_program = g_canonicalize_filename(
        program_path != NULL ? program_path : "umicom-studio-ide", NULL);
    if (absolute_program == NULL) {
        umi_splash_show(splash, NULL);
        flush_startup_presentation();
        return splash;
    }
    program_directory = g_path_get_dirname(absolute_program);
    logo_path = program_directory != NULL
        ? g_build_filename(
              program_directory, "branding", "umicom-logo.png", NULL)
        : NULL;
    if (logo_path != NULL &&
        g_file_test(logo_path, G_FILE_TEST_IS_REGULAR)) {
        umi_splash_set_brand_image(splash, logo_path);
    }
    g_free(logo_path);
    g_free(program_directory);
    g_free(absolute_program);

    umi_splash_show(splash, NULL);
    flush_startup_presentation();
    return splash;
}

int main(int argc, char **argv)
{
    UmiStudioBootstrap *bootstrap = NULL;
    UmiSplash *splash;
    UmiStatus status;
    int result;

    splash = show_startup_splash(argc > 0 ? argv[0] : NULL);
    umi_splash_set_progress(
        splash, 0.20, "Creating Framework services…");
    flush_startup_presentation();

    status = umi_studio_bootstrap_create(&bootstrap);
    if (status != UMI_STATUS_OK) {
        (void)fprintf(stderr,
                      "[USIDE] Framework create failed: %s\n",
                      umi_status_text(status));
        umi_splash_free(splash);
        return 1;
    }

    umi_splash_set_progress(
        splash, 0.48, "Starting developer and project services…");
    flush_startup_presentation();
    status = umi_studio_bootstrap_start(bootstrap);
    if (status != UMI_STATUS_OK) {
        (void)fprintf(stderr,
                      "[USIDE] Framework start failed: %s\n",
                      umi_status_text(status));
        umi_studio_bootstrap_destroy(bootstrap);
        umi_splash_free(splash);
        return 1;
    }

    /* Branding is resolved beside the executable.  This works for both the
     * build tree and installed packages without compiling a developer's
     * private absolute folder structure into the public binary. */
    configure_runtime_branding(bootstrap, argc > 0 ? argv[0] : NULL);
    umi_splash_set_progress(
        splash, 0.68, "Restoring your workspace and layouts…");
    flush_startup_presentation();

    result = run_studio(bootstrap, splash, argc, argv);
    umi_studio_bootstrap_destroy(bootstrap);
    return result;
}

#ifdef _WIN32
#include <windows.h>

static int wants_console(int argc, char **argv)
{
    const char *environment = getenv("USIDE_DEV");
    int index;
    if (environment != NULL &&
        (str_eq(environment, "1") || _stricmp(environment, "true") == 0)) {
        return 1;
    }

    /*
     * Diagnostic UI modes do not automatically need a native console. A
     * console is created only when the developer requests one explicitly.
     */
    for (index = 0; index < argc; ++index) {
        if (str_eq(argv[index], "--console") ||
            str_eq(argv[index], "--dev") ||
            str_eq(argv[index], "--test-window") ||
            str_eq(argv[index], "--bare-gtk") ||
            str_eq(argv[index], "--framework-workbench") ||
            str_eq(argv[index], "--legacy-ui")) {
            return 1;
        }
    }
    return 0;
}

static void attach_or_allocate_console(void)
{
    if (!AttachConsole(ATTACH_PARENT_PROCESS)) (void)AllocConsole();
    (void)freopen("CONOUT$", "w", stdout);
    (void)freopen("CONOUT$", "w", stderr);
    (void)freopen("CONIN$", "r", stdin);
    (void)setvbuf(stdout, NULL, _IONBF, 0);
    (void)setvbuf(stderr, NULL, _IONBF, 0);
    (void)SetConsoleOutputCP(CP_UTF8);
    (void)SetConsoleCP(CP_UTF8);
}

int WINAPI WinMain(HINSTANCE instance,
                   HINSTANCE previous_instance,
                   LPSTR command_line,
                   int show_command)
{
    (void)instance;
    (void)previous_instance;
    (void)command_line;
    (void)show_command;
    if (wants_console(__argc, __argv)) attach_or_allocate_console();
    return main(__argc, __argv);
}
#endif
