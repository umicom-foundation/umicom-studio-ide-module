/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/debug.inc
 *
 * PURPOSE:
 *   Build the operational DAP-backed Run and Debug surface.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

static GtkWidget *runtime_build_surface_debug(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Run and Debug");
    GtkWidget *thread_row;
    GtkWidget *controls;
    GtkWidget *start;
    GtkWidget *continue_button;
    GtkWidget *pause;
    GtkWidget *next;
    GtkWidget *step_in;
    GtkWidget *step_out;
    GtkWidget *stop;
    GtkWidget *watch;

    if (panel == NULL || runtime == NULL) return panel;

    runtime->debug_summary_label = runtime_label("", 0.0F);
    runtime->debug_thread_entry = gtk_entry_new();

    if (runtime->debug_summary_label != NULL) {
        gtk_widget_add_css_class(
            runtime->debug_summary_label,
            "umicom-runtime-muted");
        gtk_box_append(
            GTK_BOX(panel),
            runtime->debug_summary_label);
    }

    thread_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    if (thread_row != NULL && runtime->debug_thread_entry != NULL) {
        GtkWidget *label = runtime_label("Thread", 0.0F);
        gtk_editable_set_text(
            GTK_EDITABLE(runtime->debug_thread_entry),
            "0");
        gtk_entry_set_placeholder_text(
            GTK_ENTRY(runtime->debug_thread_entry),
            "Thread id");
        gtk_widget_set_hexpand(runtime->debug_thread_entry, TRUE);
        if (label != NULL) gtk_box_append(GTK_BOX(thread_row), label);
        gtk_box_append(
            GTK_BOX(thread_row),
            runtime->debug_thread_entry);
        gtk_box_append(GTK_BOX(panel), thread_row);
    }

    controls = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    start = gtk_button_new_with_label("Start");
    continue_button = gtk_button_new_with_label("Continue");
    pause = gtk_button_new_with_label("Pause");
    next = gtk_button_new_with_label("Step Over");
    step_in = gtk_button_new_with_label("Step In");
    step_out = gtk_button_new_with_label("Step Out");
    stop = gtk_button_new_with_label("Stop");
    watch = gtk_button_new_with_label("Add Watch");

    if (controls != NULL && start != NULL &&
        continue_button != NULL && pause != NULL &&
        next != NULL && step_in != NULL &&
        step_out != NULL && stop != NULL && watch != NULL) {
        g_signal_connect(
            start, "clicked",
            G_CALLBACK(runtime_on_debug_start), runtime);
        g_signal_connect(
            continue_button, "clicked",
            G_CALLBACK(runtime_on_debug_continue), runtime);
        g_signal_connect(
            pause, "clicked",
            G_CALLBACK(runtime_on_debug_pause), runtime);
        g_signal_connect(
            next, "clicked",
            G_CALLBACK(runtime_on_debug_next), runtime);
        g_signal_connect(
            step_in, "clicked",
            G_CALLBACK(runtime_on_debug_step_in), runtime);
        g_signal_connect(
            step_out, "clicked",
            G_CALLBACK(runtime_on_debug_step_out), runtime);
        g_signal_connect(
            stop, "clicked",
            G_CALLBACK(runtime_on_debug_stop), runtime);
        g_signal_connect(
            watch, "clicked",
            G_CALLBACK(runtime_on_debug_add_watch), runtime);

        gtk_box_append(GTK_BOX(controls), start);
        gtk_box_append(GTK_BOX(controls), continue_button);
        gtk_box_append(GTK_BOX(controls), pause);
        gtk_box_append(GTK_BOX(controls), next);
        gtk_box_append(GTK_BOX(controls), step_in);
        gtk_box_append(GTK_BOX(controls), step_out);
        gtk_box_append(GTK_BOX(controls), stop);
        gtk_box_append(GTK_BOX(controls), watch);
        gtk_box_append(GTK_BOX(panel), controls);
    }

    gtk_box_append(
        GTK_BOX(panel),
        runtime_card_new(
            "Debugger state",
            "Breakpoints, threads, frames, scopes, variables and watches are "
            "owned by the existing Framework/Studio DAP services. Phase 2 "
            "adds direct operator controls without creating a second debugger."));

    {
        GtkWidget *details = runtime_debug_details_notebook(runtime);
        if (details != NULL) {
            gtk_box_append(GTK_BOX(panel), details);
        }
    }

    runtime_append_context_actions(runtime, panel, "debug");

    runtime_debug_update_summary(runtime);
    runtime_debug_details_refresh(runtime);
    return runtime_scroll(panel);
}
