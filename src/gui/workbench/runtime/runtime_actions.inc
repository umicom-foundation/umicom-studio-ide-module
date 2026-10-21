/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_actions.inc
 *
 * PURPOSE:
 *   Route GTK user intent into the authoritative Framework Studio runtime.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static UmiStatus runtime_refresh_services(UmiStudioGtkRuntimeChrome *runtime);
/*
 * Provide the runtime show notification operation used by this module and its client
 * applications.
 */
static void runtime_show_notification(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *message);

/*
 * Phase 1 compatibility path:
 * Keep the original direct Framework Studio Runtime dispatch intact as the
 * fallback for every command that does not require GTK-side argument capture.
 * Phase 2 routes only the commands that need native file/folder/text input or
 * a canonical Studio service alias; everything else still follows this exact
 * established execution path.
 */
/* Framework command completion may detach this host. The weak binding now guards refresh and feedback; the previous compatibility implementation remains for review. */
#if 0
static UmiStatus runtime_execute_phase1_host_command(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *command_id,
    const char *argument)
{
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL ||
        runtime->runtime_bootstrap == NULL ||
        command_id == NULL ||
        command_id[0] == '\0') {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    status = umi_studio_host_controller_execute_command(
        &runtime->host_controller,
        command_id,
        argument);

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        char message[384];
        (void)snprintf(
            message,
            sizeof(message),
            "%s: %s",
            command_id,
            umi_status_text(status));
        runtime_show_notification(runtime, message);
        return status;
    }

    return runtime_refresh_services(runtime);
}

#endif
static UmiStatus runtime_execute_phase1_host_command(
    UmiStudioGtkRuntimeChrome *runtime, const char *command_id, const char *argument)
{
    if (runtime == NULL || runtime->runtime_bootstrap == NULL || command_id == NULL || command_id[0] == '\0')
        return UMI_STATUS_INVALID_ARGUMENT;
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    char *ownedCommand = g_strdup(command_id), *ownedArgument = g_strdup(argument);
    UmiStatus status = umi_studio_host_controller_execute_command(
        &runtime->host_controller, ownedCommand, ownedArgument);
    GObject *window = NULL;
    runtime = runtime_file_dialog_owner(request, &window);
    if (runtime == NULL) status = UMI_STATUS_CANCELLED;
    else if (status != UMI_STATUS_OK) {
        char message[384];
        (void)snprintf(message, sizeof message, "%s: %s", ownedCommand, umi_status_text(status));
        runtime_show_notification(runtime, message);
    } else status = runtime_refresh_services(runtime);
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
    g_free(ownedCommand);
    g_free(ownedArgument);
    return status;
}

/*
 * Provide the runtime execute command operation used by this module and its client
 * applications.
 */
/* Commands may release their workbench through observers. The replacement resolves the existing lifetime token before feedback; the raw-pointer completion is retained for review. */
#if 0
static UmiStatus runtime_execute_command(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *command_id,
    const char *argument)
{
    /* Framework reports bookmark results through editing completion, which
     * may release this host. Never send a second notification afterwards. */
    if (command_id != NULL && (strcmp(command_id, "editor.bookmark.toggle") == 0 ||
        strcmp(command_id, "editor.bookmark.next") == 0 || strcmp(command_id, "editor.bookmark.previous") == 0 ||
        strcmp(command_id, "editor.bookmark.clear") == 0)) return runtime_route_command(runtime, command_id, argument);

    /* This modal route owns lifetime-checked immediate feedback itself. Return
     * directly so generic error handling cannot revisit a destroyed host. */
    if (command_id != NULL && strcmp(command_id, "editor.replace-complete-set") == 0)
        return RuntimeReviewReplacementSet(runtime);
    if (command_id != NULL && strcmp(command_id, "editor.replace-open-documents") == 0)
        return RuntimeReviewOpenReplacements(runtime);

/* REFACTOR NOTE: Editing and source navigation deliver their result through
 * Framework completion and runtime_document_edit_completed. Preserve fallback
 * feedback for unbound routes, but do not overwrite an already displayed error.
 * The former code is retained below for reference; do not enable both paths.
 */
//     UmiStatus status = runtime_route_command(runtime, command_id, argument);
//     if (status != UMI_STATUS_OK) {

    UmiDocumentEditCommand editCommand;
    /* Close now also completes through Framework's editing binding. Preserve
     * the former classification below; do not repeat its error notification
     * after the shared callback may already have refreshed or detached Studio. */
    // int editing = UmiDocumentEditCommandFromId(command_id, &editCommand) == UMI_STATUS_OK ||
    // (command_id != NULL && strcmp(command_id, "umicom.document.go-to-line") == 0);
    /* Reopen and Forget now report through the same Framework completion.
     * Keep the previous classification for review; the extended predicate
     * prevents a second notification after completion has detached a host. */
#if 0
    int editing = UmiDocumentEditCommandFromId(command_id, &editCommand) == UMI_STATUS_OK ||
        (command_id != NULL && (strcmp(command_id, "umicom.document.go-to-line") == 0 ||
            strcmp(command_id, "file.close") == 0));
#endif
    /* Framework reports navigation through the editing completion too. Keep
     * the earlier classification for review; avoid a second error notification. */
#if 0
    int editing = UmiDocumentEditCommandFromId(command_id, &editCommand) == UMI_STATUS_OK ||
        (command_id != NULL && (strcmp(command_id, "umicom.document.go-to-line") == 0 ||
            strcmp(command_id, "file.close") == 0 || strcmp(command_id, "file.reopen-closed") == 0 ||
            strcmp(command_id, "file.forget-closed") == 0));
#endif
    int editing = UmiDocumentEditCommandFromId(command_id, &editCommand) == UMI_STATUS_OK ||
        (command_id != NULL && (strcmp(command_id, "umicom.document.go-to-line") == 0 ||
            strcmp(command_id, "file.close") == 0 || strcmp(command_id, "file.reopen-closed") == 0 ||
            strcmp(command_id, "file.forget-closed") == 0 ||
            strcmp(command_id, "navigate.back") == 0 || strcmp(command_id, "navigate.forward") == 0 ||
            strcmp(command_id, "editor.navigate.back") == 0 || strcmp(command_id, "editor.navigate.forward") == 0 ||
            strcmp(command_id, "navigate.history.clear") == 0));
    int completedByBinding = editing && runtime != NULL && runtime->owner != NULL &&
        UmiGtk4AdapterDocumentHasCompletion(runtime->owner->adapter);
    UmiStatus status = runtime_route_command(runtime, command_id, argument);
    if (status != UMI_STATUS_OK && !completedByBinding) {
        char message[384];
        (void)snprintf(message, sizeof(message), "%s: %s", command_id, umi_status_text(status));
        runtime_show_notification(runtime, message);
    }
    return status;
}
#endif
static UmiStatus runtime_execute_command(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *command_id,
    const char *argument)
{
    if (runtime == NULL || runtime->owner == NULL || command_id == NULL || command_id[0] == '\0')
        return UMI_STATUS_INVALID_ARGUMENT;
    /* Framework reports bookmark results through editing completion, which
     * may release this host. Never send a second notification afterwards. */
    if (command_id != NULL && (strcmp(command_id, "editor.bookmark.toggle") == 0 ||
        strcmp(command_id, "editor.bookmark.next") == 0 || strcmp(command_id, "editor.bookmark.previous") == 0 ||
        strcmp(command_id, "editor.bookmark.clear") == 0)) return runtime_route_command(runtime, command_id, argument);

    /* This modal route owns lifetime-checked immediate feedback itself. Return
     * directly so generic error handling cannot revisit a destroyed host. */
    if (command_id != NULL && strcmp(command_id, "editor.replace-complete-set") == 0)
        return RuntimeReviewReplacementSet(runtime);
    if (command_id != NULL && strcmp(command_id, "editor.replace-open-documents") == 0)
        return RuntimeReviewOpenReplacements(runtime);

/* REFACTOR NOTE: Editing and source navigation deliver their result through
 * Framework completion and runtime_document_edit_completed. Preserve fallback
 * feedback for unbound routes, but do not overwrite an already displayed error.
 * The former code is retained below for reference; do not enable both paths.
 */
//     UmiStatus status = runtime_route_command(runtime, command_id, argument);
//     if (status != UMI_STATUS_OK) {

    UmiDocumentEditCommand editCommand;
    /* Close now also completes through Framework's editing binding. Preserve
     * the former classification below; do not repeat its error notification
     * after the shared callback may already have refreshed or detached Studio. */
    // int editing = UmiDocumentEditCommandFromId(command_id, &editCommand) == UMI_STATUS_OK ||
    // (command_id != NULL && strcmp(command_id, "umicom.document.go-to-line") == 0);
    /* Reopen and Forget now report through the same Framework completion.
     * Keep the previous classification for review; the extended predicate
     * prevents a second notification after completion has detached a host. */
#if 0
    int editing = UmiDocumentEditCommandFromId(command_id, &editCommand) == UMI_STATUS_OK ||
        (command_id != NULL && (strcmp(command_id, "umicom.document.go-to-line") == 0 ||
            strcmp(command_id, "file.close") == 0));
#endif
    /* Framework reports navigation through the editing completion too. Keep
     * the earlier classification for review; avoid a second error notification. */
#if 0
    int editing = UmiDocumentEditCommandFromId(command_id, &editCommand) == UMI_STATUS_OK ||
        (command_id != NULL && (strcmp(command_id, "umicom.document.go-to-line") == 0 ||
            strcmp(command_id, "file.close") == 0 || strcmp(command_id, "file.reopen-closed") == 0 ||
            strcmp(command_id, "file.forget-closed") == 0));
#endif
    int editing = UmiDocumentEditCommandFromId(command_id, &editCommand) == UMI_STATUS_OK ||
        (command_id != NULL && (strcmp(command_id, "umicom.document.go-to-line") == 0 ||
            strcmp(command_id, "file.close") == 0 || strcmp(command_id, "file.reopen-closed") == 0 ||
            strcmp(command_id, "file.forget-closed") == 0 ||
            strcmp(command_id, "navigate.back") == 0 || strcmp(command_id, "navigate.forward") == 0 ||
            strcmp(command_id, "editor.navigate.back") == 0 || strcmp(command_id, "editor.navigate.forward") == 0 ||
            strcmp(command_id, "navigate.history.clear") == 0));
    int completedByBinding = editing && runtime != NULL && runtime->owner != NULL &&
        UmiGtk4AdapterDocumentHasCompletion(runtime->owner->adapter);
    /* A document-change or focus observer may close the workbench while an
     * action is executing. Keep only copied inputs and the existing weak host
     * token across dispatch; never issue feedback through the old raw pointer. */
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    char *command = g_strdup(command_id);
    char *copiedArgument = g_strdup(argument);
    uint64_t feedbackRevision = runtime->notification_revision;
    UmiStatus status = runtime_route_command(runtime, command, copiedArgument);
    GObject *window = NULL;
    runtime = runtime_file_dialog_owner(request, &window);
    if (runtime != NULL && status != UMI_STATUS_OK && !completedByBinding &&
        runtime->notification_revision == feedbackRevision) {
        char message[384];
        (void)snprintf(message, sizeof(message), "%s: %s", command, umi_status_text(status));
        runtime_show_notification(runtime, message);
    }
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
    g_free(copiedArgument);
    g_free(command);
    return status;
}

/*
 * Provide the runtime on command button operation used by this module and its client
 * applications.
 */
/* Button callbacks now resolve a weak Studio owner rather than borrowing the runtime for the entire widget lifetime. Retain the original handler for review. */
#if 0
static void runtime_on_command_button(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *command_id =
        (const char *)g_object_get_data(
            G_OBJECT(button),
            "umicom-command-id");
    const char *argument =
        (const char *)g_object_get_data(
            G_OBJECT(button),
            "umicom-command-argument");

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || command_id == NULL) return;
    (void)runtime_execute_command(runtime, command_id, argument);
}

#endif


/* A button can be retained independently of its window. Its signal owns a
 * weak Studio token, never the runtime allocation. Command text is copied by
 * runtime_execute_command before a command can rebuild or release the button. */
static void runtime_on_command_button(GtkButton *button, gpointer user_data)
{
    UmiStudioFileDialogRequest *request = user_data;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(request, &window);
    const char *command = g_object_get_data(G_OBJECT(button), "umicom-command-id");
    const char *argument = g_object_get_data(G_OBJECT(button), "umicom-command-argument");
    if (runtime != NULL && command != NULL)
        (void)runtime_execute_command(runtime, command, argument);
    g_clear_object(&window);
}
/*
 * Provide the runtime on local command palette operation used by this module and its
 * client applications.
 */
/* Local controls use the same weak signal owner as canonical command buttons, so retained controls cannot reopen a released host. The previous callback is retained for review. */
#if 0
static void runtime_on_local_command_palette(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    runtime_show_command_palette((UmiStudioGtkRuntimeChrome *)user_data);
}

#endif


static void runtime_on_local_command_palette(GtkButton *button, gpointer user_data)
{
    (void)button;
    UmiStudioFileDialogRequest *request = user_data;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(request, &window);
    if (runtime != NULL) runtime_show_command_palette(runtime);
    g_clear_object(&window);
}
/*
 * Provide the runtime on local layout browser operation used by this module and its client
 * applications.
 */
/* Local controls use the same weak signal owner as canonical command buttons, so retained controls cannot reopen a released host. The previous callback is retained for review. */
#if 0
static void runtime_on_local_layout_browser(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    runtime_show_layout_browser((UmiStudioGtkRuntimeChrome *)user_data);
}

#endif


static void runtime_on_local_layout_browser(GtkButton *button, gpointer user_data)
{
    (void)button;
    UmiStudioFileDialogRequest *request = user_data;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(request, &window);
    if (runtime != NULL) runtime_show_layout_browser(runtime);
    g_clear_object(&window);
}
/*
 * Provide the runtime make local button operation used by this module and its client
 * applications.
 */
/* Each signal closure owns one request. Binding teardown invalidates the
 * shared runtime token; final widget release then frees this weak reference. */
static void RuntimeCommandButtonReleased(gpointer data, GClosure *closure)
{
    (void)closure;
    runtime_file_dialog_request_free(data);
}

static int RuntimeConnectCommandButton(UmiStudioGtkRuntimeChrome *runtime,
    GtkWidget *button, GCallback callback)
{
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL) return 0;
    g_signal_connect_data(button, "clicked", callback, request,
        RuntimeCommandButtonReleased, 0);
    return 1;
}

/* Local buttons now own weak signal bindings. Keep the original construction for comparison with the checked connector below. */
#if 0
static GtkWidget *runtime_make_local_button(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *label,
    const char *tooltip,
    int show_layouts)
{
    GtkWidget *button = gtk_button_new_with_label(label != NULL ? label : "");

    /* The value is consumed by the signal callback selection below. */
    (void)show_layouts;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (button == NULL || runtime == NULL) return button;
    /* Stable IDs let UAT select these local controls without using captions. */
    (void)umi_gtk4_automation_tag_widget(
        button,
        show_layouts
            ? "studio.local.layout-browser"
            : "studio.local.command-palette");
    gtk_widget_set_tooltip_text(button, tooltip != NULL ? tooltip : "");
    g_signal_connect(
        button,
        "clicked",
        show_layouts
            ? G_CALLBACK(runtime_on_local_layout_browser)
            : G_CALLBACK(runtime_on_local_command_palette),
        runtime);
    return button;
}

#endif


static GtkWidget *runtime_make_local_button(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *label,
    const char *tooltip,
    int show_layouts)
{
    GtkWidget *button = gtk_button_new_with_label(label != NULL ? label : "");

    /* The value is consumed by the signal callback selection below. */
    (void)show_layouts;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (button == NULL || runtime == NULL) return button;
    /* Stable IDs let UAT select these local controls without using captions. */
    (void)umi_gtk4_automation_tag_widget(
        button,
        show_layouts
            ? "studio.local.layout-browser"
            : "studio.local.command-palette");
    gtk_widget_set_tooltip_text(button, tooltip != NULL ? tooltip : "");
    if (!RuntimeConnectCommandButton(runtime, button, show_layouts
            ? G_CALLBACK(runtime_on_local_layout_browser)
            : G_CALLBACK(runtime_on_local_command_palette)))
        gtk_widget_set_sensitive(button, FALSE);
    return button;
}

/*
 * Provide the runtime make command button operation used by this module and its client
 * applications.
 */
/* Command controls now retain weak signal ownership instead of the runtime address. The prior labelled/icon construction remains for review. */
#if 0
static GtkWidget *runtime_make_command_button(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *label,
    const char *command_id,
    const char *tooltip)
{
    GtkWidget *button = gtk_button_new_with_label(label != NULL ? label : "");

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (button == NULL) return NULL;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (tooltip != NULL && tooltip[0] != '\0') {
        gtk_widget_set_tooltip_text(button, tooltip);
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (command_id != NULL && command_id[0] != '\0') {
        /* The canonical command ID is also the control's automation address. */
        (void)umi_gtk4_automation_tag_widget(button, command_id);
        g_object_set_data_full(
            G_OBJECT(button),
            "umicom-command-id",
            g_strdup(command_id),
            g_free);
        g_signal_connect(
            button,
            "clicked",
            G_CALLBACK(runtime_on_command_button),
            runtime);
        gtk_widget_set_sensitive(
            button,
            runtime_command_enabled(runtime, command_id));
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        gtk_widget_set_sensitive(button, FALSE);
    }

    return button;
}

#endif


static GtkWidget *runtime_make_command_button(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *label,
    const char *command_id,
    const char *tooltip)
{
    GtkWidget *button = gtk_button_new_with_label(label != NULL ? label : "");

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (button == NULL) return NULL;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (tooltip != NULL && tooltip[0] != '\0') {
        gtk_widget_set_tooltip_text(button, tooltip);
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (command_id != NULL && command_id[0] != '\0') {
        /* The canonical command ID is also the control's automation address. */
        (void)umi_gtk4_automation_tag_widget(button, command_id);
        g_object_set_data_full(
            G_OBJECT(button),
            "umicom-command-id",
            g_strdup(command_id),
            g_free);
        int connected = RuntimeConnectCommandButton(runtime, button,
            G_CALLBACK(runtime_on_command_button));
        gtk_widget_set_sensitive(button,
            connected && runtime_command_enabled(runtime, command_id));
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        gtk_widget_set_sensitive(button, FALSE);
    }

    return button;
}

/*
 * Provide the runtime make icon command button operation used by this module and its
 * client applications.
 */
/* Command controls now retain weak signal ownership instead of the runtime address. The prior labelled/icon construction remains for review. */
#if 0
static GtkWidget *runtime_make_icon_command_button(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *icon_name,
    const char *command_id,
    const char *tooltip)
{
    GtkWidget *button =
        gtk_button_new_from_icon_name(icon_name != NULL ? icon_name : "");

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (button == NULL) return NULL;

    gtk_widget_set_tooltip_text(button, tooltip != NULL ? tooltip : "");

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (command_id != NULL && command_id[0] != '\0') {
        /* Icon-only buttons remain discoverable even when their label is hidden. */
        (void)umi_gtk4_automation_tag_widget(button, command_id);
        g_object_set_data_full(
            G_OBJECT(button),
            "umicom-command-id",
            g_strdup(command_id),
            g_free);
        g_signal_connect(
            button,
            "clicked",
            G_CALLBACK(runtime_on_command_button),
            runtime);
        gtk_widget_set_sensitive(
            button,
            runtime_command_enabled(runtime, command_id));
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        gtk_widget_set_sensitive(button, FALSE);
    }

    return button;
}

#endif


static GtkWidget *runtime_make_icon_command_button(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *icon_name,
    const char *command_id,
    const char *tooltip)
{
    GtkWidget *button =
        gtk_button_new_from_icon_name(icon_name != NULL ? icon_name : "");

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (button == NULL) return NULL;

    gtk_widget_set_tooltip_text(button, tooltip != NULL ? tooltip : "");

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (command_id != NULL && command_id[0] != '\0') {
        /* Icon-only buttons remain discoverable even when their label is hidden. */
        (void)umi_gtk4_automation_tag_widget(button, command_id);
        g_object_set_data_full(
            G_OBJECT(button),
            "umicom-command-id",
            g_strdup(command_id),
            g_free);
        int connected = RuntimeConnectCommandButton(runtime, button,
            G_CALLBACK(runtime_on_command_button));
        gtk_widget_set_sensitive(button,
            connected && runtime_command_enabled(runtime, command_id));
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        gtk_widget_set_sensitive(button, FALSE);
    }

    return button;
}

/*
 * Provide the runtime refresh command sensitivity operation used by this module and its
 * client applications.
 */
static void runtime_refresh_command_sensitivity(
    UmiStudioGtkRuntimeChrome *runtime,
    GtkWidget *container)
{
    GtkWidget *child;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || container == NULL) return;

    child = gtk_widget_get_first_child(container);
    /*
     * Continue only while work remains available; the loop body advances the state on each
     * pass.
     */
    while (child != NULL) {
        const char *command_id =
            (const char *)g_object_get_data(
                G_OBJECT(child),
                "umicom-command-id");

        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (command_id != NULL) {
            gtk_widget_set_sensitive(
                child,
                runtime_command_enabled(runtime, command_id));
        }

        runtime_refresh_command_sensitivity(runtime, child);
        child = gtk_widget_get_next_sibling(child);
    }
}
