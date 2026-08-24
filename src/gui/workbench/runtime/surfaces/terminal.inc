/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/terminal.inc
 *
 * PURPOSE:
 *   Build the operational integrated terminal surface.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

static GtkWidget *runtime_build_surface_terminal(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Terminal");
    GtkWidget *command_row;
    GtkWidget *run;
    GtkWidget *new_terminal;
    GtkWidget *clear;

    if (panel == NULL || runtime == NULL) return panel;

    runtime->terminal_summary_label = runtime_label("", 0.0F);
    runtime->terminal_text_view = gtk_text_view_new();
    runtime->terminal_entry = gtk_entry_new();

    if (runtime->terminal_summary_label != NULL) {
        gtk_widget_add_css_class(
            runtime->terminal_summary_label,
            "umicom-runtime-muted");
        gtk_box_append(
            GTK_BOX(panel),
            runtime->terminal_summary_label);
    }

    if (runtime->terminal_text_view != NULL) {
        gtk_text_view_set_editable(
            GTK_TEXT_VIEW(runtime->terminal_text_view),
            FALSE);
        gtk_text_view_set_monospace(
            GTK_TEXT_VIEW(runtime->terminal_text_view),
            TRUE);
        gtk_text_view_set_wrap_mode(
            GTK_TEXT_VIEW(runtime->terminal_text_view),
            GTK_WRAP_CHAR);
        gtk_box_append(
            GTK_BOX(panel),
            runtime_scroll(runtime->terminal_text_view));
    }

    command_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    run = gtk_button_new_with_label("Run");
    new_terminal = gtk_button_new_with_label("New Terminal");
    clear = gtk_button_new_with_label("Clear");

    if (command_row != NULL && runtime->terminal_entry != NULL &&
        run != NULL && new_terminal != NULL && clear != NULL) {
        gtk_entry_set_placeholder_text(
            GTK_ENTRY(runtime->terminal_entry),
            "Enter command and press Enter");
        gtk_widget_set_hexpand(runtime->terminal_entry, TRUE);

        g_signal_connect(
            runtime->terminal_entry,
            "activate",
            G_CALLBACK(runtime_on_terminal_entry_activate),
            runtime);
        g_signal_connect(
            run,
            "clicked",
            G_CALLBACK(runtime_on_terminal_run),
            runtime);
        g_signal_connect(
            new_terminal,
            "clicked",
            G_CALLBACK(runtime_on_terminal_new),
            runtime);
        g_signal_connect(
            clear,
            "clicked",
            G_CALLBACK(runtime_on_terminal_clear),
            runtime);

        gtk_box_append(GTK_BOX(command_row), runtime->terminal_entry);
        gtk_box_append(GTK_BOX(command_row), run);
        gtk_box_append(GTK_BOX(command_row), new_terminal);
        gtk_box_append(GTK_BOX(command_row), clear);
        gtk_box_append(GTK_BOX(panel), command_row);
    }

    gtk_box_append(
        GTK_BOX(panel),
        runtime_card_new(
            "Framework Terminal",
            "Terminal sessions, supervised process execution and transcripts "
            "remain Framework-owned; Studio supplies the interactive workbench "
            "presentation and command entry."));

    runtime_terminal_refresh_transcript(runtime);
    return panel;
}
