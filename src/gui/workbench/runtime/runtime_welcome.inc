/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_welcome.inc
 *
 * PURPOSE:
 *   Render a calm startup centre for creating, opening, learning and resuming
 *   work. Commands remain owned by Framework and Studio services; this file
 *   only composes their GTK presentation.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static GtkWidget *runtime_welcome_card(
    const char *title,
    const char *description)
{
    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *heading = runtime_label(title, 0.0F);
    GtkWidget *detail = runtime_label(description, 0.0F);

    if (card == NULL || heading == NULL || detail == NULL) return card;
    gtk_widget_add_css_class(card, "umicom-welcome-card");
    gtk_widget_add_css_class(heading, "umicom-welcome-card-title");
    gtk_widget_add_css_class(detail, "umicom-runtime-muted");
    gtk_label_set_wrap(GTK_LABEL(detail), TRUE);
    gtk_label_set_wrap_mode(GTK_LABEL(detail), PANGO_WRAP_WORD_CHAR);
    gtk_label_set_max_width_chars(GTK_LABEL(detail), 38);
    gtk_box_append(GTK_BOX(card), heading);
    gtk_box_append(GTK_BOX(card), detail);
    return card;
}

static void runtime_welcome_add_action(
    UmiStudioGtkRuntimeChrome *runtime,
    GtkWidget *card,
    const char *label,
    const char *command_id,
    const char *description,
    int primary)
{
    GtkWidget *button = runtime_make_command_button(
        runtime, label, command_id, description);
    if (button == NULL || card == NULL) return;
    gtk_widget_set_halign(button, GTK_ALIGN_FILL);
    if (primary) gtk_widget_add_css_class(button, "suggested-action");
    gtk_box_append(GTK_BOX(card), button);
}

static GtkWidget *runtime_build_welcome_surface(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioWelcomeCentreSnapshot snapshot;
    GtkWidget *page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 24);
    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 18);
    GtkWidget *heading = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *quick_tools = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    GtkWidget *title = runtime_label("Welcome to Umicom Studio", 0.0F);
    GtkWidget *summary;
    GtkWidget *grid = gtk_grid_new();
    GtkWidget *create;
    GtkWidget *recent;
    GtkWidget *learn;
    GtkWidget *scroll;
    GtkWidget *empty_recent;

    if (page == NULL || runtime == NULL || runtime->owner == NULL ||
        header == NULL || heading == NULL || quick_tools == NULL ||
        title == NULL || grid == NULL) {
        return page;
    }
    (void)memset(&snapshot, 0, sizeof(snapshot));
    (void)umi_studio_welcome_centre_snapshot(
        umi_studio_ui_services(runtime->owner->ui), &snapshot);
    summary = runtime_label(
        snapshot.summary[0] != '\0'
            ? snapshot.summary
            : "Create a project, open existing work or learn Umicom Framework.",
        0.0F);

    gtk_widget_add_css_class(page, "umicom-welcome-page");
    gtk_widget_add_css_class(title, "umicom-welcome-title");
    gtk_box_append(GTK_BOX(heading), title);
    if (summary != NULL) {
        gtk_widget_add_css_class(summary, "umicom-welcome-summary");
        gtk_box_append(GTK_BOX(heading), summary);
    }
    gtk_widget_set_hexpand(heading, TRUE);
    runtime_welcome_add_action(
        runtime, quick_tools, "Extensions", "extensions.open",
        "Browse installed and available Studio extensions", 0);
    runtime_welcome_add_action(
        runtime, quick_tools, "Toolchains", "toolchains.open",
        "Inspect compilers, SDKs and build tools", 0);
    runtime_welcome_add_action(
        runtime, quick_tools, "Settings", "settings.open",
        "Change Studio and Framework preferences", 0);
    gtk_widget_set_valign(quick_tools, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(header), heading);
    gtk_box_append(GTK_BOX(header), quick_tools);
    gtk_box_append(GTK_BOX(page), header);

    gtk_grid_set_column_spacing(GTK_GRID(grid), 18);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 18);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_widget_set_hexpand(grid, TRUE);
    gtk_widget_set_vexpand(grid, TRUE);

    create = runtime_welcome_card(
        "Start",
        "Begin with a project template or open files already on your computer.");
    runtime_welcome_add_action(
        runtime, create, "Create New Project", "project.new",
        "Open the Framework project wizard", 1);
    runtime_welcome_add_action(
        runtime, create, "Open Project or File", "file.open",
        "Open a project or source file", 0);
    runtime_welcome_add_action(
        runtime, create, "Open Folder", "workspace.open-folder",
        "Open and index an existing workspace", 0);

    recent = runtime_welcome_card(
        "Recent Work",
        "Projects and folders you open will appear here for quick access.");
    empty_recent = runtime_label(
        "No recent project is recorded yet.", 0.0F);
    if (recent != NULL && empty_recent != NULL) {
        gtk_widget_add_css_class(empty_recent, "umicom-runtime-muted");
        gtk_box_append(GTK_BOX(recent), empty_recent);
    }
    runtime_welcome_add_action(
        runtime, recent, "Browse for a Workspace", "workspace.open-folder",
        "Choose a folder to add to recent work", 0);

    learn = runtime_welcome_card(
        "Learn and Explore",
        "Build confidence with guided C, GTK4 and Umicom Framework lessons.");
    runtime_welcome_add_action(
        runtime, learn, "Open Interactive Learning", "help.ai-teacher",
        "Open the Studio learning assistant", 1);
    runtime_welcome_add_action(
        runtime, learn, "Read Documentation", "help.documentation",
        "Browse Umicom developer documentation", 0);
    runtime_welcome_add_action(
        runtime, learn, "Keyboard Shortcuts", "help.shortcuts",
        "Review IDE keyboard shortcuts", 0);

    if (create != NULL) {
        gtk_grid_attach(GTK_GRID(grid), create, 0, 0, 1, 1);
    }
    if (recent != NULL) {
        gtk_grid_attach(GTK_GRID(grid), recent, 1, 0, 1, 1);
    }
    if (learn != NULL) {
        gtk_grid_attach(GTK_GRID(grid), learn, 2, 0, 1, 1);
    }
    gtk_box_append(GTK_BOX(page), grid);

    scroll = runtime_scroll(page);
    if (scroll != NULL) {
        gtk_widget_add_css_class(scroll, "umicom-welcome-scroll");
        return scroll;
    }
    return page;
}

static void runtime_welcome_update_mode(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiUiWorkbenchState state;
    int show_welcome;

    if (runtime == NULL || runtime->centre_stack == NULL ||
        runtime->owner == NULL || runtime->owner->ui == NULL) {
        return;
    }

    umi_ui_workbench_state_init(&state);
    if (umi_ui_workbench_state_snapshot(
            umi_studio_ui_workbench(runtime->owner->ui),
            &state) != UMI_STATUS_OK) {
        return;
    }

    show_welcome = state.active_document[0] == '\0' ||
        strcmp(state.active_document, "studio.editor.welcome") == 0;
    gtk_stack_set_visible_child_name(
        GTK_STACK(runtime->centre_stack),
        show_welcome ? "welcome" : "workbench");
}
