/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_layout_tabs.inc
 *
 * PURPOSE:
 *   Provide compact workspace tabs and explicit save, restore and edit-lock
 *   controls inspired by professional IDEs and trading workstations.
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
    GtkWidget *bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *profile_strip = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    GtkWidget *scroller = gtk_scrolled_window_new();
    GtkWidget *manage;
    GtkWidget *restore;
    GtkWidget *save;
    size_t index;

    if (bar == NULL || runtime == NULL || profile_strip == NULL ||
        scroller == NULL) {
        return bar;
    }

    gtk_widget_add_css_class(bar, "umicom-runtime-layout-tabs");
    gtk_widget_add_css_class(profile_strip, "umicom-runtime-layout-profiles");

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
        if (button != NULL) {
            gtk_widget_add_css_class(button, "flat");
            gtk_box_append(GTK_BOX(profile_strip), button);
        }
    }

    gtk_scrolled_window_set_policy(
        GTK_SCROLLED_WINDOW(scroller), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
    gtk_scrolled_window_set_child(
        GTK_SCROLLED_WINDOW(scroller), profile_strip);
    gtk_widget_set_hexpand(scroller, TRUE);
    gtk_box_append(GTK_BOX(bar), scroller);

    runtime->layout_lock_button = gtk_toggle_button_new_with_label(
        "Layout Locked");
    if (runtime->layout_lock_button != NULL) {
        gtk_widget_add_css_class(
            runtime->layout_lock_button, "umicom-runtime-layout-lock");
        gtk_toggle_button_set_active(
            GTK_TOGGLE_BUTTON(runtime->layout_lock_button), TRUE);
        g_signal_connect(
            runtime->layout_lock_button,
            "toggled",
            G_CALLBACK(runtime_on_layout_lock_toggled),
            runtime);
        gtk_box_append(GTK_BOX(bar), runtime->layout_lock_button);
    }

    runtime->layout_cancel_button = gtk_button_new_with_label("Cancel Edit");
    if (runtime->layout_cancel_button != NULL) {
        gtk_widget_add_css_class(runtime->layout_cancel_button, "flat");
        gtk_widget_set_visible(runtime->layout_cancel_button, FALSE);
        gtk_widget_set_tooltip_text(
            runtime->layout_cancel_button,
            "Discard panel moves and splitter changes made since unlocking");
        g_signal_connect(
            runtime->layout_cancel_button,
            "clicked",
            G_CALLBACK(runtime_on_layout_cancel_clicked),
            runtime);
        gtk_box_append(GTK_BOX(bar), runtime->layout_cancel_button);
    }

    manage = runtime_make_local_button(
        runtime,
        "Manage",
        "Browse, apply and inspect workspace layouts",
        1);
    restore = runtime_make_command_button(
        runtime,
        "Restore",
        "studio.layout.restore",
        "Restore the saved Framework workspace state");
    save = runtime_make_command_button(
        runtime,
        "Save",
        "studio.layout.save",
        "Save the current Framework workspace state");
    if (manage != NULL) gtk_box_append(GTK_BOX(bar), manage);
    if (restore != NULL) gtk_box_append(GTK_BOX(bar), restore);
    if (save != NULL) gtk_box_append(GTK_BOX(bar), save);
    return bar;
}
