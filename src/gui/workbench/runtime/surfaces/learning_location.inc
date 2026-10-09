/*-----------------------------------------------------------------------------
 * Umicom Framework
 * File: applications/studio/src/gui/workbench/runtime/surfaces/learning_location.inc
 * PURPOSE: Present explicit lesson-library and practice-project folder choices.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/teacher/learning_library.h"

/* The request owns its copied lesson path. The shared weak window binding
 * prevents a late folder answer from reaching a closed or replaced workbench. */
typedef struct UmiStudioLearningLocationRequest
{
    UmiStudioFileDialogRequest *owner;
    char *resource;
} UmiStudioLearningLocationRequest;

/* Open the documentation surface before publishing its preview so the first lesson click cannot
 * lose its text. The previous implementation is retained for engineering review. */
#if 0
static UmiStatus RuntimeLearningOpen(UmiStudioGtkRuntimeChrome *runtime, const char *resource)
{
    char file[UMI_PATH_CAPACITY];
    UmiStatus status =
        UmiLearningLibraryResource(runtime->learning_directory, resource, file, sizeof(file));
    if (status == UMI_STATUS_OK)
    {
        status = runtime_documentation_display_location(runtime, file, 1);
        runtime_show_surface(runtime, UMI_STUDIO_SURFACE_DOCUMENTATION);
    }
    if (status != UMI_STATUS_OK)
        runtime_show_notification(runtime, "This lesson was not found in the selected library. "
                                           "Choose the Framework lesson folder again.");
    return status;
}
#endif
static UmiStatus RuntimeLearningOpen(UmiStudioGtkRuntimeChrome *runtime, const char *resource)
{
    char file[UMI_PATH_CAPACITY];
    UmiStatus status =
        UmiLearningLibraryResource(runtime->learning_directory, resource, file, sizeof(file));
    if (status == UMI_STATUS_OK)
    {
        /* The first lesson click may create Documentation. Populate its text
         * only after its native view exists, and report a failed surface open. */
        status = runtime_workspace_open_surface(runtime, UMI_STUDIO_SURFACE_DOCUMENTATION, NULL);
        if (status == UMI_STATUS_OK)
            status = runtime_documentation_display_location(runtime, file, 1);
    }
    if (status != UMI_STATUS_OK)
        runtime_show_notification(runtime, "This lesson was not found in the selected library. "
                                           "Choose the Framework lesson folder again.");
    return status;
}

/* Settings remember only the chosen directory, never lesson edits or passwords.
 * An unavailable persistence store does not discard the user's current choice. */
static void RuntimeLearningLocationComplete(GObject *source, GAsyncResult *result, gpointer data)
{
    UmiStudioLearningLocationRequest *request = data;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(request->owner, &window);
    GError *error = NULL;
    GFile *folder = gtk_file_dialog_select_folder_finish(GTK_FILE_DIALOG(source), result, &error);
    char *path = folder != NULL ? g_file_get_path(folder) : NULL;
    char directory[UMI_PATH_CAPACITY];
    if (runtime != NULL)
        runtime->learning_location_pending = 0;
    if (runtime != NULL && folder != NULL)
    {
        UmiStatus status = UmiLearningLibrarySelect(path, directory, sizeof(directory));
        if (status == UMI_STATUS_OK)
        {
            memcpy(runtime->learning_directory, directory, strlen(directory) + 1U);
            /* A panel may have closed while the chooser was open. Resolve its
             * current logical control instead of dereferencing a cached widget. */
            GtkWidget *label = umi_gtk4_automation_find_tagged_widget(GTK_WIDGET(window),
                                                                      "studio.learning.library");
            if (GTK_IS_LABEL(label))
                gtk_label_set_text(GTK_LABEL(label), directory);
            UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
            if (runtime->owner->options.persist_session)
            {
                status = umi_session_store_set(umi_studio_services_session(services),
                                               "learning.library", directory);
                if (status == UMI_STATUS_OK)
                    status = umi_studio_session_save(services);
            }
            runtime_show_notification(
                runtime,
                status == UMI_STATUS_OK
                    ? "Lesson library selected. Your project folder has not changed."
                    : "Lesson library selected for this session; its location could not be saved.");
            if (request->resource != NULL)
                (void)RuntimeLearningOpen(runtime, request->resource);
        }
        else
        {
            runtime_show_notification(runtime,
                                      "Choose Umicom-Applications, Framework, docs, or learning "
                                      "containing welcome.html. The previous library was kept.");
        }
    }
    else if (runtime != NULL && error != NULL && !runtime_file_dialog_cancelled(error))
    {
        runtime_show_notification(runtime, error->message);
    }
    g_free(path);
    g_clear_object(&folder);
    g_clear_error(&error);
    g_clear_object(&window);
    runtime_file_dialog_request_free(request->owner);
    g_free(request->resource);
    g_free(request);
}

static void RuntimeLearningChoose(UmiStudioGtkRuntimeChrome *runtime, const char *resource)
{
    if (runtime == NULL || runtime->owner == NULL || runtime->owner->window == NULL)
        return;
    if (runtime->learning_location_pending)
    {
        runtime_show_notification(runtime,
                                  "Finish or cancel the open lesson-library chooser first.");
        return;
    }
    UmiStudioLearningLocationRequest *request = g_try_new0(UmiStudioLearningLocationRequest, 1);
    if (request == NULL)
        return;
    request->owner = runtime_file_dialog_request_new(runtime, 0U);
    if (request->owner == NULL)
    {
        g_free(request);
        return;
    }
    request->resource = g_strdup(resource);
    GtkFileDialog *dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, "Choose your Umicom lesson library");
    if (runtime->learning_directory[0] != '\0')
    {
        GFile *initial = g_file_new_for_path(runtime->learning_directory);
        gtk_file_dialog_set_initial_folder(dialog, initial);
        g_object_unref(initial);
    }
    runtime->learning_location_pending = 1;
    gtk_file_dialog_select_folder(dialog, runtime->owner->window, NULL,
                                  RuntimeLearningLocationComplete, request);
    g_object_unref(dialog);
}
/* Each action keeps a weak window identity rather than borrowing the runtime.
 * Retaining a button after its workbench closes must not keep permission to open
 * a chooser or access the former project's services. */
static void RuntimeLearningActionFree(gpointer data) { runtime_file_dialog_request_free(data); }
static void RuntimeLearningBind(GtkWidget *button, GCallback callback,
                                UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL)
    {
        gtk_widget_set_sensitive(button, FALSE);
        return;
    }
    g_object_set_data_full(G_OBJECT(button), "umicom-learning-action-owner", request,
                           RuntimeLearningActionFree);
    g_signal_connect(button, "clicked", callback, request);
}

/* Resolve the weak workbench binding on each click so retained learning controls cannot access a
 * closed owner. The previous implementation is retained for engineering review. */
#if 0
static void RuntimeLearningChooseClicked(GtkButton *button, gpointer data)
{
    (void)button;
    RuntimeLearningChoose(data, NULL);
}
#endif
static void RuntimeLearningChooseClicked(GtkButton *button, gpointer data)
{
    (void)button;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(data, &window);
    if (runtime != NULL)
    {
        RuntimeLearningChoose(runtime, NULL);
    }
    g_clear_object(&window);
}
/* Resolve the weak workbench binding on each click so retained learning controls cannot access a
 * closed owner. The previous implementation is retained for engineering review. */
#if 0
static void RuntimeLearningProjectClicked(GtkButton *button, gpointer data)
{
    (void)button;
    UmiStudioGtkRuntimeChrome *runtime = data;
    UmiStatus status = RuntimeNewProject(runtime);
    if (status != UMI_STATUS_OK) runtime_show_notification(runtime, umi_status_text(status));
}
#endif
static void RuntimeLearningProjectClicked(GtkButton *button, gpointer data)
{
    (void)button;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(data, &window);
    if (runtime != NULL)
    {
        UmiStatus status = RuntimeNewProject(runtime);
        if (status != UMI_STATUS_OK)
            runtime_show_notification(runtime, umi_status_text(status));
    }
    g_clear_object(&window);
}
/* Resolve the weak workbench binding on each click so retained learning controls cannot access a
 * closed owner. The previous implementation is retained for engineering review. */
#if 0
static void RuntimeLearningWorkspaceClicked(GtkButton *button, gpointer data)
{
    (void)button;
    UmiStudioGtkRuntimeChrome *runtime = data;
    UmiStatus status = runtime_choose_workspace_folder(runtime);
    if (status != UMI_STATUS_OK) runtime_show_notification(runtime, umi_status_text(status));
}
#endif
static void RuntimeLearningWorkspaceClicked(GtkButton *button, gpointer data)
{
    (void)button;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(data, &window);
    if (runtime != NULL)
    {
        UmiStatus status = runtime_choose_workspace_folder(runtime);
        if (status != UMI_STATUS_OK)
            runtime_show_notification(runtime, umi_status_text(status));
    }
    g_clear_object(&window);
}

/* Keep the source library and writable practice projects visibly separate.
 * No implicit project is created in AppData when the user reads a lesson. */
/* Learning actions now retain a weak window token instead of a borrowed runtime pointer; the
 * original composition remains for ownership review. The previous implementation is retained for
 * engineering review. */
#if 0
static void RuntimeLearningLocations(UmiStudioGtkRuntimeChrome *runtime, GtkWidget *panel)
{
    char saved[UMI_PATH_CAPACITY];
    if (runtime->learning_directory[0] == '\0' && runtime->owner->options.persist_session &&
        umi_session_store_get(umi_studio_services_session(
            umi_studio_ui_services(runtime->owner->ui)), "learning.library",
            saved, sizeof(saved)) == UMI_STATUS_OK) {
        (void)UmiLearningLibrarySelect(saved, runtime->learning_directory,
            sizeof(runtime->learning_directory));
    }
    gtk_box_append(GTK_BOX(panel), runtime_section_title("Lesson library and practice projects"));
    GtkWidget *location_label = runtime_label(runtime->learning_directory[0] != '\0'
        ? runtime->learning_directory : "Choose the folder containing the Umicom lessons.", 0.0F);
    (void)umi_gtk4_automation_tag_widget(location_label, "studio.learning.library");
    gtk_box_append(GTK_BOX(panel), location_label);
    GtkWidget *choose = gtk_button_new_with_label("Choose lesson library…");
    (void)umi_gtk4_automation_tag_widget(choose, "studio.learning.choose-library");
    g_signal_connect(choose, "clicked", G_CALLBACK(RuntimeLearningChooseClicked), runtime);
    gtk_box_append(GTK_BOX(panel), choose);
    GtkWidget *create = gtk_button_new_with_label("Create a practice project…");
    (void)umi_gtk4_automation_tag_widget(create, "studio.learning.create-project");
    g_signal_connect(create, "clicked", G_CALLBACK(RuntimeLearningProjectClicked), runtime);
    gtk_box_append(GTK_BOX(panel), create);
    GtkWidget *open = gtk_button_new_with_label("Open my practice folder…");
    (void)umi_gtk4_automation_tag_widget(open, "studio.learning.open-project");
    g_signal_connect(open, "clicked", G_CALLBACK(RuntimeLearningWorkspaceClicked), runtime);
    gtk_box_append(GTK_BOX(panel), open);
    gtk_box_append(GTK_BOX(panel), runtime_label(
        "Read a lesson from your library. Create or open a project in a folder you choose, "
        "then edit src/main.c and save it. In Build > Project Settings, check your compiler, "
        "build folder and workspace trust. Configure, Build and Run use those settings. "
        "Debug also requires a configured debugger; Deploy requires an installation destination.", 0.0F));
}
#endif
/* Read the explicit library choice from the already-owned session model even when disk persistence
 * is disabled. This does not open a settings file or infer a filesystem location. The previous
 * implementation is retained for engineering review. */
#if 0
static void RuntimeLearningLocations(UmiStudioGtkRuntimeChrome *runtime, GtkWidget *panel)
{
    char saved[UMI_PATH_CAPACITY];
    if (runtime->learning_directory[0] == '\0' && runtime->owner->options.persist_session &&
        umi_session_store_get(
            umi_studio_services_session(umi_studio_ui_services(runtime->owner->ui)),
            "learning.library", saved, sizeof(saved)) == UMI_STATUS_OK)
    {
        (void)UmiLearningLibrarySelect(saved, runtime->learning_directory,
                                       sizeof(runtime->learning_directory));
    }
    gtk_box_append(GTK_BOX(panel), runtime_section_title("Lesson library and practice projects"));
    GtkWidget *location_label = runtime_label(
        runtime->learning_directory[0] != '\0' ? runtime->learning_directory
                                               : "Choose the folder containing the Umicom lessons.",
        0.0F);
    (void)umi_gtk4_automation_tag_widget(location_label, "studio.learning.library");
    gtk_box_append(GTK_BOX(panel), location_label);
    GtkWidget *choose = gtk_button_new_with_label("Choose lesson library…");
    (void)umi_gtk4_automation_tag_widget(choose, "studio.learning.choose-library");
    RuntimeLearningBind(choose, G_CALLBACK(RuntimeLearningChooseClicked), runtime);
    gtk_box_append(GTK_BOX(panel), choose);
    GtkWidget *create = gtk_button_new_with_label("Create a practice project…");
    (void)umi_gtk4_automation_tag_widget(create, "studio.learning.create-project");
    RuntimeLearningBind(create, G_CALLBACK(RuntimeLearningProjectClicked), runtime);
    gtk_box_append(GTK_BOX(panel), create);
    GtkWidget *open = gtk_button_new_with_label("Open my practice folder…");
    (void)umi_gtk4_automation_tag_widget(open, "studio.learning.open-project");
    RuntimeLearningBind(open, G_CALLBACK(RuntimeLearningWorkspaceClicked), runtime);
    gtk_box_append(GTK_BOX(panel), open);
    gtk_box_append(
        GTK_BOX(panel),
        runtime_label(
            "Read a lesson from your library. Create or open a project in a folder you choose, "
            "then edit src/main.c and save it. In Build > Project Settings, check your compiler, "
            "build folder and workspace trust. Configure, Build and Run use those settings. "
            "Debug also requires a configured debugger; Deploy requires an installation "
            "destination.",
            0.0F));
}
#endif
static void RuntimeLearningLocations(UmiStudioGtkRuntimeChrome *runtime, GtkWidget *panel)
{
    char saved[UMI_PATH_CAPACITY];
    if (runtime->learning_directory[0] == '\0' &&
        umi_session_store_get(
            umi_studio_services_session(umi_studio_ui_services(runtime->owner->ui)),
            "learning.library", saved, sizeof(saved)) == UMI_STATUS_OK)
    {
        (void)UmiLearningLibrarySelect(saved, runtime->learning_directory,
                                       sizeof(runtime->learning_directory));
    }
    gtk_box_append(GTK_BOX(panel), runtime_section_title("Lesson library and practice projects"));
    GtkWidget *location_label = runtime_label(
        runtime->learning_directory[0] != '\0' ? runtime->learning_directory
                                               : "Choose the folder containing the Umicom lessons.",
        0.0F);
    (void)umi_gtk4_automation_tag_widget(location_label, "studio.learning.library");
    gtk_box_append(GTK_BOX(panel), location_label);
    GtkWidget *choose = gtk_button_new_with_label("Choose lesson library…");
    (void)umi_gtk4_automation_tag_widget(choose, "studio.learning.choose-library");
    RuntimeLearningBind(choose, G_CALLBACK(RuntimeLearningChooseClicked), runtime);
    gtk_box_append(GTK_BOX(panel), choose);
    GtkWidget *create = gtk_button_new_with_label("Create a practice project…");
    (void)umi_gtk4_automation_tag_widget(create, "studio.learning.create-project");
    RuntimeLearningBind(create, G_CALLBACK(RuntimeLearningProjectClicked), runtime);
    gtk_box_append(GTK_BOX(panel), create);
    GtkWidget *open = gtk_button_new_with_label("Open my practice folder…");
    (void)umi_gtk4_automation_tag_widget(open, "studio.learning.open-project");
    RuntimeLearningBind(open, G_CALLBACK(RuntimeLearningWorkspaceClicked), runtime);
    gtk_box_append(GTK_BOX(panel), open);
    gtk_box_append(
        GTK_BOX(panel),
        runtime_label(
            "Read a lesson from your library. Create or open a project in a folder you choose, "
            "then edit src/main.c and save it. In Build > Project Settings, check your compiler, "
            "build folder and workspace trust. Configure, Build and Run use those settings. "
            "Debug also requires a configured debugger; Deploy requires an installation "
            "destination.",
            0.0F));
}
