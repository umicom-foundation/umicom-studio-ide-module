/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_layout_tabs.inc
 *
 * PURPOSE:
 *   Provide TWS-inspired one-click workspace/layout tabs at the bottom of the
 *   Studio workbench.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static GtkWidget *runtime_build_layout_tabs(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
    GtkWidget *spacer;
    GtkWidget *save;
    size_t index;

    if (bar == NULL || runtime == NULL) return bar;

    gtk_widget_add_css_class(bar, "umicom-runtime-layout-tabs");

    for (index = 0U;
         index < sizeof(RUNTIME_LAYOUT_PROFILES) /
                 sizeof(RUNTIME_LAYOUT_PROFILES[0]);
         ++index) {
        const UmiStudioGtkLayoutProfile *profile =
            &RUNTIME_LAYOUT_PROFILES[index];
        GtkWidget *button = runtime_make_command_button(
            runtime,
            profile->label,
            profile->command_id,
            profile->description);
        if (button != NULL) gtk_box_append(GTK_BOX(bar), button);
    }

    spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    if (spacer != NULL) {
        gtk_widget_set_hexpand(spacer, TRUE);
        gtk_box_append(GTK_BOX(bar), spacer);
    }

    save = runtime_make_command_button(
        runtime,
        "Save Layout",
        "studio.layout.save",
        "Persist the semantic Studio layout");
    if (save != NULL) gtk_box_append(GTK_BOX(bar), save);

    return bar;
}
