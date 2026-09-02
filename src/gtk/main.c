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
#include "icon.h"
#include "splash.h"
#include "workbench_window.h"
#include "umicom/studio/appearance_centre.h"
#include "umicom/studio/bootstrap.h"

/* Provide the str eq operation used by this module and its client applications. */
static int str_eq(const char *a, const char *b)
{
    return a != NULL && b != NULL && strcmp(a, b) == 0;
}

/*
 * Provide the configure runtime branding operation used by this module and its client
 * applications.
 */
static void configure_runtime_branding(UmiStudioBootstrap *bootstrap,
                                       const char *program_path)
{
    char *absolute_program;
    char *program_directory;
    char *branding_directory;
    char *logo_path;
    char *icon_path;
    UmiStudioUi *ui;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (bootstrap == NULL || program_path == NULL) return;
    absolute_program = g_canonicalize_filename(program_path, NULL);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (absolute_program == NULL) return;
    program_directory = g_path_get_dirname(absolute_program);
    branding_directory = g_build_filename(program_directory, "branding", NULL);
    /* Studio starts with its dark workspace, so the first visible controls use
     * the light-on-dark vectors. The appearance service can select another
     * contrast variant later when the user changes theme. */
    logo_path = g_build_filename(
        branding_directory, "umicom-logo-on-dark.svg", NULL);
    icon_path = g_build_filename(
        branding_directory, "umicom-icon-on-dark.svg", NULL);
    ui = umi_studio_bootstrap_ui(bootstrap);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (ui != NULL && logo_path != NULL && icon_path != NULL &&
        g_file_test(logo_path, G_FILE_TEST_IS_REGULAR) &&
        g_file_test(icon_path, G_FILE_TEST_IS_REGULAR)) {
        (void)umi_studio_appearance_set_brand_resources(
            umi_studio_ui_workbench(ui), logo_path, icon_path);
        /* Legacy Studio header widgets use this shared vector path rather than
         * carrying a second embedded raster copy of the Umicom mark. */
        (void)umi_icon_set_brand_path(icon_path);
    }
    g_free(icon_path);
    g_free(logo_path);
    g_free(branding_directory);
    g_free(program_directory);
    g_free(absolute_program);
}

/* Provide the log line operation used by this module and its client applications. */
static void log_line(const char *text)
{
    (void)fprintf(stderr, "%s\n", text != NULL ? text : "");
}

/* Provide the on bare close operation used by this module and its client applications. */
static gboolean on_bare_close(GtkWindow *window, gpointer user_data)
{
    GMainLoop *loop = (GMainLoop *)user_data;
    (void)window;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (loop != NULL) g_main_loop_quit(loop);
    return FALSE;
}

/* Provide the run bare gtk operation used by this module and its client applications. */
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

/* Provide the on test activate operation used by this module and its client applications. */
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

/*
 * Provide the run test window app operation used by this module and its client
 * applications.
 */
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

/*
 * Provide the on framework workbench activate operation used by this module and its client
 * applications.
 */
static void on_framework_workbench_activate(GtkApplication *application,
                                            gpointer user_data)
{
    UmiStudioFrameworkWorkbenchRun *run =
        (UmiStudioFrameworkWorkbenchRun *)user_data;
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (run == NULL || run->bootstrap == NULL || run->workbench != NULL) return;
    umi_splash_set_progress(
        run->splash, 0.82, "Composing the professional workbench…");
    status = umi_studio_gtk_workbench_create(
        application,
        umi_studio_bootstrap_ui(run->bootstrap),
        umi_studio_bootstrap_desktop_shell(run->bootstrap),
        &run->workbench);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
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

/*
 * Provide the run framework workbench operation used by this module and its client
 * applications.
 */
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
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
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

/* Provide the filter dev flags operation used by this module and its client applications. */
static int filter_dev_flags(int argc, char **argv, char ***out_argv)
{
    char **filtered;
    int count = 0;
    int index;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (out_argv == NULL) return 0;
    filtered = (char **)malloc((size_t)(argc + 1) * sizeof(*filtered));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (filtered == NULL) {
        *out_argv = NULL;
        return 0;
    }
    filtered[count++] = argv[0];
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 1; index < argc; ++index) {
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
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

/* Provide the run studio operation used by this module and its client applications. */
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

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0; index < argc; ++index) {
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (str_eq(argv[index], "--bare-gtk")) {
            umi_splash_free(splash);
            return run_bare_gtk();
        }
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (str_eq(argv[index], "--test-window")) {
            umi_splash_free(splash);
            return run_test_window_app(argc, argv);
        }
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (str_eq(argv[index], "--legacy-ui")) {
            filtered_argc = filter_dev_flags(argc, argv, &filtered_argv);
            /*
             * Protect caller-owned memory by checking that required state is available before it is
             * used.
             */
            if (filtered_argv == NULL) {
                umi_splash_free(splash);
                return 1;
            }
            application = umi_app_new();
            /*
             * Protect caller-owned memory by checking that required state is available before it is
             * used.
             */
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
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
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

/*
 * Provide the flush startup presentation operation used by this module and its client
 * applications.
 */
static void flush_startup_presentation(void)
{
    /*
     * Continue only while work remains available; the loop body advances the state on each
     * pass.
     */
    while (g_main_context_pending(NULL)) {
        (void)g_main_context_iteration(NULL, FALSE);
    }
}

/*
 * Provide the show startup splash operation used by this module and its client
 * applications.
 */
static UmiSplash *show_startup_splash(const char *program_path)
{
    UmiSplash *splash;
    char *absolute_program;
    char *program_directory;
    char *icon_path;

    gtk_init();
    splash = umi_splash_new(
        "Umicom Studio IDE",
        "Starting Umicom Framework…",
        0U);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (splash == NULL) return NULL;

    absolute_program = g_canonicalize_filename(
        program_path != NULL ? program_path : "umicom-studio-ide", NULL);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (absolute_program == NULL) {
        umi_splash_show(splash, NULL);
        flush_startup_presentation();
        return splash;
    }
    program_directory = g_path_get_dirname(absolute_program);
    icon_path = program_directory != NULL
        ? g_build_filename(
              program_directory, "branding", "umicom-icon-on-dark.svg", NULL)
        : NULL;
    /* The splash composes the icon with its native title label; it does not
     * rasterise the application name into a second image. */
    if (icon_path != NULL &&
        g_file_test(icon_path, G_FILE_TEST_IS_REGULAR)) {
        umi_splash_set_brand_image(splash, icon_path);
    }
    g_free(icon_path);
    g_free(program_directory);
    g_free(absolute_program);

    umi_splash_show(splash, NULL);
    flush_startup_presentation();
    return splash;
}

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(int argc, char **argv)
{
    UmiStudioBootstrap *bootstrap = NULL;
    UmiSplash *splash;
    UmiStatus status;
    int result;

    /* Keep operating-system window matching aligned with the packaged Studio
     * identity and its Framework-owned native icon. */
    g_set_prgname("umicom-studio-ide");

    splash = show_startup_splash(argc > 0 ? argv[0] : NULL);
    umi_splash_set_progress(
        splash, 0.20, "Creating Framework services…");
    flush_startup_presentation();

    status = umi_studio_bootstrap_create(&bootstrap);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
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
    /* Preserve the original failure result so the caller can respond to the correct cause. */
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
    /* Release the startup path after all Studio windows have closed. */
    umi_icon_clear_brand_path();
    return result;
}

#ifdef _WIN32
#include <windows.h>

/* Provide the wants console operation used by this module and its client applications. */
static int wants_console(int argc, char **argv)
{
    const char *environment = getenv("USIDE_DEV");
    int index;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (environment != NULL &&
        (str_eq(environment, "1") || _stricmp(environment, "true") == 0)) {
        return 1;
    }

    /*
     * Diagnostic UI modes do not automatically need a native console. A
     * console is created only when the developer requests one explicitly.
     */
    for (index = 0; index < argc; ++index) {
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
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

/*
 * Provide the attach or allocate console operation used by this module and its client
 * applications.
 */
static void attach_or_allocate_console(void)
{
    /* Apply this branch only when its contract condition is satisfied. */
    if (!AttachConsole(ATTACH_PARENT_PROCESS)) (void)AllocConsole();
    (void)freopen("CONOUT$", "w", stdout);
    (void)freopen("CONOUT$", "w", stderr);
    (void)freopen("CONIN$", "r", stdin);
    (void)setvbuf(stdout, NULL, _IONBF, 0);
    (void)setvbuf(stderr, NULL, _IONBF, 0);
    (void)SetConsoleOutputCP(CP_UTF8);
    (void)SetConsoleCP(CP_UTF8);
}

/* Provide the win main operation used by this module and its client applications. */
int WINAPI WinMain(HINSTANCE instance,
                   HINSTANCE previous_instance,
                   LPSTR command_line,
                   int show_command)
{
    (void)instance;
    (void)previous_instance;
    (void)command_line;
    (void)show_command;
    /* Apply this branch only when its contract condition is satisfied. */
    if (wants_console(__argc, __argv)) attach_or_allocate_console();
    return main(__argc, __argv);
}
#endif
