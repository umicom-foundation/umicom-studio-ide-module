/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_panel_helpers.inc
 *
 * PURPOSE:
 *   Shared presentation helpers for Framework-backed Studio tool panels.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static GtkWidget *runtime_panel_new(const char *title)
{
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (box == NULL) return NULL;

    (void)title;

    gtk_widget_add_css_class(box, "umicom-runtime-tool-panel");
    gtk_widget_set_size_request(
        box,
        UMI_STUDIO_GTK_RUNTIME_TOOL_MIN_WIDTH,
        -1);

    return box;
}

/*
 * Provide the runtime panel add stat operation used by this module and its client
 * applications.
 */
static void runtime_panel_add_stat(
    GtkWidget *panel,
    const char *label,
    const char *value)
{
    GtkWidget *row;
    GtkWidget *left;
    GtkWidget *right;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (panel == NULL) return;

    row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    left = runtime_label(label, 0.0F);
    right = runtime_label(value, 1.0F);

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (row == NULL || left == NULL || right == NULL) return;

    gtk_widget_add_css_class(left, "umicom-runtime-muted");
    gtk_widget_set_hexpand(left, TRUE);
    gtk_widget_set_hexpand(right, TRUE);
    gtk_box_append(GTK_BOX(row), left);
    gtk_box_append(GTK_BOX(row), right);
    gtk_box_append(GTK_BOX(panel), row);
}

/*
 * Provide the runtime panel add commands operation used by this module and its client
 * applications.
 */
static void runtime_panel_add_commands(
    UmiStudioGtkRuntimeChrome *runtime,
    GtkWidget *panel,
    const char *const *labels,
    const char *const *commands,
    size_t count)
{
    GtkWidget *row;
    size_t index;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || panel == NULL ||
        labels == NULL || commands == NULL) {
        return;
    }

    row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (row == NULL) return;

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < count; ++index) {
        GtkWidget *button = runtime_make_command_button(
            runtime,
            labels[index],
            commands[index],
            commands[index]);
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (button != NULL) gtk_box_append(GTK_BOX(row), button);
    }

    gtk_box_append(GTK_BOX(panel), row);
}
