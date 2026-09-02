/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_command_centre.inc
 *
 * PURPOSE:
 *   Provide Phase 4 global navigation and workflow entry points above the
 *   central Framework editor without moving reusable IDE behaviour into GTK.
 *   Buttons delegate to existing command, layout and service surfaces.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static void runtime_on_command_centre_commands(GtkButton *button, gpointer user_data)
{
    (void)button;
    runtime_show_command_palette((UmiStudioGtkRuntimeChrome *)user_data);
}

/*
 * Provide the runtime on command centre layouts operation used by this module and its
 * client applications.
 */
static void runtime_on_command_centre_layouts(GtkButton *button, gpointer user_data)
{
    (void)button;
    runtime_show_layout_browser((UmiStudioGtkRuntimeChrome *)user_data);
}

/*
 * Provide the runtime on command centre workspace operation used by this module and its
 * client applications.
 */
static void runtime_on_command_centre_workspace(GtkButton *button, gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime = (UmiStudioGtkRuntimeChrome *)user_data;
    (void)button;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL) return;
    runtime_show_command_palette(runtime);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->command_entry != NULL) gtk_editable_set_text(GTK_EDITABLE(runtime->command_entry), "workspace");
}

/*
 * Provide the runtime on command centre build operation used by this module and its client
 * applications.
 */
static void runtime_on_command_centre_build(GtkButton *button, gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime = (UmiStudioGtkRuntimeChrome *)user_data;
    (void)button;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL) return;
    runtime_show_command_palette(runtime);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->command_entry != NULL) gtk_editable_set_text(GTK_EDITABLE(runtime->command_entry), "build");
}

/*
 * Provide the runtime on command centre test operation used by this module and its client
 * applications.
 */
static void runtime_on_command_centre_test(GtkButton *button, gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime = (UmiStudioGtkRuntimeChrome *)user_data;
    (void)button;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL) return;
    runtime_show_command_palette(runtime);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->command_entry != NULL) gtk_editable_set_text(GTK_EDITABLE(runtime->command_entry), "test");
}

/*
 * Provide the runtime command centre button operation used by this module and its client
 * applications.
 */
static GtkWidget *runtime_command_centre_button(const char *label, const char *tooltip, GCallback callback, UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *button = gtk_button_new_with_label(label);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (button == NULL) return NULL;
    gtk_widget_add_css_class(button, "flat");
    gtk_widget_set_tooltip_text(button, tooltip);
    g_signal_connect(button, "clicked", callback, runtime);
    return button;
}

/*
 * Provide the runtime build command centre operation used by this module and its client
 * applications.
 */
static GtkWidget *runtime_build_command_centre(UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *bar, *spacer, *workspace, *commands, *layouts, *build, *tests;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL) return NULL;
    bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (bar == NULL) return NULL;
    gtk_widget_add_css_class(bar, "umicom-runtime-command-centre");
    runtime->command_centre_workspace_label = runtime_label(runtime->workspace_name[0] != '\0' ? runtime->workspace_name : "No workspace", 0.0F);
    spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    workspace = runtime_command_centre_button("Workspace", "Search workspace commands", G_CALLBACK(runtime_on_command_centre_workspace), runtime);
    commands = runtime_command_centre_button("Commands", "Search all Studio commands and menus", G_CALLBACK(runtime_on_command_centre_commands), runtime);
    layouts = runtime_command_centre_button("Layouts", "Open the Framework layout browser", G_CALLBACK(runtime_on_command_centre_layouts), runtime);
    build = runtime_command_centre_button("Build", "Search build and delivery operations", G_CALLBACK(runtime_on_command_centre_build), runtime);
    tests = runtime_command_centre_button("Tests", "Search test operations", G_CALLBACK(runtime_on_command_centre_test), runtime);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (spacer != NULL) gtk_widget_set_hexpand(spacer, TRUE);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->command_centre_workspace_label != NULL) {
        gtk_widget_add_css_class(runtime->command_centre_workspace_label, "umicom-runtime-muted");
        gtk_box_append(GTK_BOX(bar), runtime->command_centre_workspace_label);
    }
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (spacer != NULL) gtk_box_append(GTK_BOX(bar), spacer);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (workspace != NULL) gtk_box_append(GTK_BOX(bar), workspace);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (commands != NULL) gtk_box_append(GTK_BOX(bar), commands);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (layouts != NULL) gtk_box_append(GTK_BOX(bar), layouts);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (build != NULL) gtk_box_append(GTK_BOX(bar), build);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (tests != NULL) gtk_box_append(GTK_BOX(bar), tests);
    return bar;
}

/*
 * Provide the runtime command centre refresh operation used by this module and its client
 * applications.
 */
static void runtime_command_centre_refresh(UmiStudioGtkRuntimeChrome *runtime)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->command_centre_workspace_label == NULL) return;
    gtk_label_set_text(GTK_LABEL(runtime->command_centre_workspace_label), runtime->workspace_name[0] != '\0' ? runtime->workspace_name : "No workspace");
}
