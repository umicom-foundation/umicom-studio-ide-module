/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/debug.inc
 *
 * PURPOSE:
 *   Build the operational DAP-backed Run and Debug surface.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
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

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (panel == NULL || runtime == NULL) return panel;

    const char *adapterKinds[] = {"GDB (DAP)", "LLDB DAP", NULL};
    GtkWidget *adapterRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    runtime->debug_adapter_kind = gtk_drop_down_new_from_strings(adapterKinds);
#ifndef _WIN32
    gtk_drop_down_set_selected(GTK_DROP_DOWN(runtime->debug_adapter_kind), 1U);
#endif
    runtime->debug_adapter_executable = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(runtime->debug_adapter_executable),
        "Adapter executable path (empty uses PATH)");
    gtk_widget_set_hexpand(runtime->debug_adapter_executable, TRUE);
    /* Reopening the panel displays the actual per-window selection. Saved
     * preferences remain an explicit Load action, never project-supplied code. */
    UmiDebugAdapterPreferences selectedAdapter;
    if (UmiStudioDebuggerNativeChoice(umi_studio_services_debugger(umi_studio_ui_services(runtime->owner->ui)),
        selectedAdapter.kind, sizeof(selectedAdapter.kind), selectedAdapter.executable, sizeof(selectedAdapter.executable)) == UMI_STATUS_OK) {
        gtk_drop_down_set_selected(GTK_DROP_DOWN(runtime->debug_adapter_kind), strcmp(selectedAdapter.kind, "gdb") == 0 ? 0U : 1U);
        gtk_editable_set_text(GTK_EDITABLE(runtime->debug_adapter_executable), selectedAdapter.executable);
    }
    (void)umi_gtk4_automation_tag_widget(runtime->debug_adapter_kind, "studio.debug.adapter.kind");
    (void)umi_gtk4_automation_tag_widget(runtime->debug_adapter_executable, "studio.debug.adapter.executable");
    GtkWidget *applyAdapter = gtk_button_new_with_label("Use adapter");
    gtk_widget_set_tooltip_text(applyAdapter,
        "Select installed gdb with DAP support or lldb-dap. No debugger is downloaded.");
    runtime_bind_editor_action(runtime, applyAdapter, RuntimeDebugConfigure);
    (void)umi_gtk4_automation_tag_widget(applyAdapter, "studio.debug.adapter.use");
    gtk_box_append(GTK_BOX(adapterRow), runtime->debug_adapter_kind);
    gtk_box_append(GTK_BOX(adapterRow), runtime->debug_adapter_executable);
    gtk_box_append(GTK_BOX(adapterRow), applyAdapter);
    gtk_box_append(GTK_BOX(panel), adapterRow);
    GtkWidget *preferenceRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    GtkWidget *saveAdapter = gtk_button_new_with_label("Save adapter choice locally");
    GtkWidget *loadAdapter = gtk_button_new_with_label("Load saved adapter choice");
    g_object_set_data(G_OBJECT(saveAdapter), "umicom-debug-preference-save", GINT_TO_POINTER(1));
    gtk_widget_set_tooltip_text(saveAdapter, "Save this draft in your user configuration folder. It does not change a running debugger; the write may finish after closing this window.");
    gtk_widget_set_tooltip_text(loadAdapter, "Load a saved choice into these fields. Review it and choose Use adapter separately.");
    (void)umi_gtk4_automation_tag_widget(saveAdapter, "studio.debug.adapter.save");
    (void)umi_gtk4_automation_tag_widget(loadAdapter, "studio.debug.adapter.load");
    runtime_bind_editor_action(runtime, saveAdapter, RuntimeDebugPreferenceClicked);
    runtime_bind_editor_action(runtime, loadAdapter, RuntimeDebugPreferenceClicked);
    gtk_box_append(GTK_BOX(preferenceRow), saveAdapter);
    gtk_box_append(GTK_BOX(preferenceRow), loadAdapter);
    gtk_box_append(GTK_BOX(panel), preferenceRow);
    /* File review and restoration use Framework's shared setup owner. */
    gtk_box_append(GTK_BOX(panel), RuntimeDebugSetupCreate(runtime));
    gtk_box_append(GTK_BOX(panel), RuntimeDebugAttachCreate(runtime));
    gtk_box_append(GTK_BOX(panel), RuntimeDebugExceptionFiltersCreate(runtime));
    gtk_box_append(GTK_BOX(panel), RuntimeDebugFunctionBreakpointsCreate(runtime));
    gtk_box_append(GTK_BOX(panel), RuntimeDebugModulesCreate(runtime));
    gtk_box_append(GTK_BOX(panel), RuntimeDebugSourcesCreate(runtime));
    gtk_box_append(GTK_BOX(panel), RuntimeDebugExceptionInformationCreate(runtime));

    runtime->debug_summary_label = runtime_label("", 0.0F);
    runtime->debug_thread_entry = gtk_entry_new();

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->debug_summary_label != NULL) {
        gtk_widget_add_css_class(
            runtime->debug_summary_label,
            "umicom-runtime-muted");
        gtk_box_append(
            GTK_BOX(panel),
            runtime->debug_summary_label);
    }

    thread_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (thread_row != NULL && runtime->debug_thread_entry != NULL) {
        GtkWidget *label = runtime_label("Thread", 0.0F);
        gtk_editable_set_text(
            GTK_EDITABLE(runtime->debug_thread_entry),
            "0");
        gtk_entry_set_placeholder_text(
            GTK_ENTRY(runtime->debug_thread_entry),
            "Thread id");
        gtk_widget_set_hexpand(runtime->debug_thread_entry, TRUE);
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
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

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (controls != NULL && start != NULL &&
        continue_button != NULL && pause != NULL &&
        next != NULL && step_in != NULL &&
        step_out != NULL && stop != NULL && watch != NULL) {
        /* Reuse the weak window-owner binding in interaction/editor_controls.inc.
         * A retained button cannot call a released Studio runtime. */
        // g_signal_connect(
        // start, "clicked",
        // G_CALLBACK(runtime_on_debug_start), runtime);
        runtime_bind_editor_action(runtime, start, runtime_on_debug_start);
        /* Reuse the weak window-owner binding in interaction/editor_controls.inc.
         * A retained button cannot call a released Studio runtime. */
        // g_signal_connect(
        // continue_button, "clicked",
        // G_CALLBACK(runtime_on_debug_continue), runtime);
        runtime_bind_editor_action(runtime, continue_button, runtime_on_debug_continue);
        /* Reuse the weak window-owner binding in interaction/editor_controls.inc.
         * A retained button cannot call a released Studio runtime. */
        // g_signal_connect(
        // pause, "clicked",
        // G_CALLBACK(runtime_on_debug_pause), runtime);
        runtime_bind_editor_action(runtime, pause, runtime_on_debug_pause);
        /* Reuse the weak window-owner binding in interaction/editor_controls.inc.
         * A retained button cannot call a released Studio runtime. */
        // g_signal_connect(
        // next, "clicked",
        // G_CALLBACK(runtime_on_debug_next), runtime);
        runtime_bind_editor_action(runtime, next, runtime_on_debug_next);
        /* Reuse the weak window-owner binding in interaction/editor_controls.inc.
         * A retained button cannot call a released Studio runtime. */
        // g_signal_connect(
        // step_in, "clicked",
        // G_CALLBACK(runtime_on_debug_step_in), runtime);
        runtime_bind_editor_action(runtime, step_in, runtime_on_debug_step_in);
        /* Reuse the weak window-owner binding in interaction/editor_controls.inc.
         * A retained button cannot call a released Studio runtime. */
        // g_signal_connect(
        // step_out, "clicked",
        // G_CALLBACK(runtime_on_debug_step_out), runtime);
        runtime_bind_editor_action(runtime, step_out, runtime_on_debug_step_out);
        /* Reuse the weak window-owner binding in interaction/editor_controls.inc.
         * A retained button cannot call a released Studio runtime. */
        // g_signal_connect(
        // stop, "clicked",
        // G_CALLBACK(runtime_on_debug_stop), runtime);
        runtime_bind_editor_action(runtime, stop, runtime_on_debug_stop);
        /* Reuse the weak window-owner binding in interaction/editor_controls.inc.
         * A retained button cannot call a released Studio runtime. */
        // g_signal_connect(
        // watch, "clicked",
        // G_CALLBACK(runtime_on_debug_add_watch), runtime);
        runtime_bind_editor_action(runtime, watch, runtime_on_debug_add_watch);

        gtk_box_append(GTK_BOX(controls), start);
        gtk_box_append(GTK_BOX(controls), continue_button);
        gtk_box_append(GTK_BOX(controls), pause);
        gtk_box_append(GTK_BOX(controls), next);
        gtk_box_append(GTK_BOX(controls), step_in);
        gtk_box_append(GTK_BOX(controls), step_out);
        gtk_box_append(GTK_BOX(controls), stop);
        GtkWidget *restart = gtk_button_new_with_label("Restart");
        gtk_widget_set_tooltip_text(restart,
            "Restart the active session without rebuilding. The adapter must advertise restart support.");
        (void)umi_gtk4_automation_tag_widget(restart, "studio.debug.restart");
        runtime_bind_editor_action(runtime, restart, RuntimeDebugRestart);
        gtk_box_append(GTK_BOX(controls), restart);
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
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (details != NULL) {
            gtk_box_append(GTK_BOX(panel), details);
        }
    }

    runtime_append_context_actions(runtime, panel, "debug");

    runtime_debug_update_summary(runtime);
    runtime_debug_details_refresh(runtime);
    return runtime_scroll(panel);
}
