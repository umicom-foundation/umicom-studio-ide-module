/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/debug_breakpoints.inc
 * PURPOSE: Present stable source breakpoint property rows over owned Framework edits.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/


typedef struct RuntimeBreakpointRow {
    UmiDebugBreakpointEdit *edit;
    GtkEntry *condition, *log;
    GtkCheckButton *enabled;
} RuntimeBreakpointRow;

static void RuntimeBreakpointRowFree(gpointer data)
{
    RuntimeBreakpointRow *row = data;
    if (row != NULL) { UmiDebugBreakpointEditDestroy(row->edit); g_free(row); }
}
/* Entry widgets are owned by the attached row. The existing weak window
 * action binding rejects removed rows and retained controls after close. */
static void RuntimeBreakpointApply(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    RuntimeBreakpointRow *row = g_object_get_data(G_OBJECT(button), "umicom-breakpoint-row");
    if (row == NULL) return;
    bool remove = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button), "umicom-breakpoint-remove")) != 0;
    UmiDebugBreakpointSettings settings; UmiStudioBreakpointEditResult result = {0};
    UmiStatus status = UMI_STATUS_OK;
    if (!remove) status = UmiDebugBreakpointSettingsInit(&settings, gtk_check_button_get_active(row->enabled),
        gtk_editable_get_text(GTK_EDITABLE(row->condition)), gtk_editable_get_text(GTK_EDITABLE(row->log)));
    if (status == UMI_STATUS_OK) status = UmiStudioDebuggerEditBreakpoint(
        umi_studio_services_debugger(umi_studio_ui_services(runtime->owner->ui)), row->edit,
        remove ? NULL : &settings, remove, &result);
    /* Refresh may release this row. Retain only copied result values below. */
    (void)runtime_refresh_services(runtime);
    if (status == UMI_STATUS_OK) runtime_show_notification(runtime,
        result.adapterSynchronized ? "Breakpoint settings sent. Check each row for adapter verification." :
        "Breakpoint settings retained for the next native debug session.");
    else {
        char message[320]; (void)snprintf(message, sizeof message, "%s: %s.",
            result.desiredApplied ? "Local settings changed; adapter synchronization did not complete. Inspect before retrying" :
            "Breakpoint was not changed; refresh a stale row or correct its fields", umi_status_text(status));
        runtime_show_notification(runtime, message);
    }
}
static void RuntimeBreakpointReset(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    RuntimeBreakpointRow *row = g_object_get_data(G_OBJECT(button), "umicom-breakpoint-row");
    UmiDebugBreakpointSnapshot value;
    if (row == NULL) return;
    if (UmiDebugBreakpointEditValidate(runtime_debug_workspace(runtime), row->edit) != UMI_STATUS_OK ||
        UmiDebugBreakpointEditRead(row->edit, &value) != UMI_STATUS_OK) {
        (void)runtime_refresh_services(runtime);
        runtime_show_notification(runtime, "Breakpoint changed. Use its refreshed row."); return;
    }
    gtk_editable_set_text(GTK_EDITABLE(row->condition), value.condition);
    gtk_editable_set_text(GTK_EDITABLE(row->log), value.log_message);
    gtk_check_button_set_active(row->enabled, value.enabled != 0);
}
static void RuntimeBreakpointTag(GtkWidget *widget, const char *kind, size_t index)
{
    char tag[96]; (void)snprintf(tag, sizeof tag, "studio.debug.breakpoint.%s.%zu", kind, index);
    (void)umi_gtk4_automation_tag_widget(widget, tag);
}
/* Inspecting variables or selecting a stack frame must not discard typed
 * breakpoint settings. Rebuild only when this collection or its owner changes. */
static void RuntimeDebugBreakpointsRefresh(UmiStudioGtkRuntimeChrome *runtime,
    UmiDebugWorkspace *workspace, const UmiDebugViewStamp *stamp)
{
    if (runtime->debug_breakpoints_list == NULL) return;
    const UmiDebugViewStamp *old = g_object_get_data(G_OBJECT(runtime->debug_breakpoints_list), "umicom-breakpoint-stamp");
    if (old != NULL && old->owner == stamp->owner && old->breakpoints == stamp->breakpoints &&
        old->sessions == stamp->sessions && old->configurations == stamp->configurations) return;
    UmiDebugViewStamp *stored = g_new(UmiDebugViewStamp, 1); *stored = *stamp;
    g_object_set_data_full(G_OBJECT(runtime->debug_breakpoints_list), "umicom-breakpoint-stamp", stored, g_free);
    runtime_clear_list_box(runtime->debug_breakpoints_list);
    UmiDebugWorkspaceSnapshot snapshot;
    if (umi_debug_workspace_snapshot(workspace, &snapshot) != UMI_STATUS_OK) return;
    for (size_t i = 0U; i < snapshot.breakpoint_count; ++i) {
        UmiDebugBreakpointSnapshot value;
        if (umi_debug_workspace_breakpoint_at(workspace, i, &value) != UMI_STATUS_OK) continue;
        GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
        RuntimeBreakpointRow *row = g_new0(RuntimeBreakpointRow, 1);
        UmiStatus status = UmiDebugBreakpointEditCapture(workspace, i, &row->edit);
        g_object_set_data_full(G_OBJECT(box), "umicom-breakpoint-row-owner", row, RuntimeBreakpointRowFree);
        char title[1280]; (void)snprintf(title, sizeof title, "%s:%u:%u - %s",
            value.uri, value.line, value.column, !value.enabled ? "disabled" :
            value.verified ? "adapter verified" : "not verified");
        gtk_box_append(GTK_BOX(box), runtime_label(title, 0.0F));
        GtkWidget *condition = gtk_entry_new(), *log = gtk_entry_new();
        row->condition = GTK_ENTRY(condition); row->log = GTK_ENTRY(log);
        gtk_box_append(GTK_BOX(box), runtime_label("Condition (empty means always)", 0.0F));
        gtk_box_append(GTK_BOX(box), condition);
        gtk_box_append(GTK_BOX(box), runtime_label("Log message (empty means stop)", 0.0F));
        gtk_box_append(GTK_BOX(box), log);
        gtk_accessible_update_property(GTK_ACCESSIBLE(condition), GTK_ACCESSIBLE_PROPERTY_LABEL, "Breakpoint condition", -1);
        gtk_accessible_update_property(GTK_ACCESSIBLE(log), GTK_ACCESSIBLE_PROPERTY_LABEL, "Breakpoint log message", -1);
        gtk_widget_set_tooltip_text(condition, "Adapter expression, up to 511 UTF-8 bytes. Applied only when you choose Apply.");
        gtk_widget_set_tooltip_text(log, "Adapter log message, up to 511 UTF-8 bytes. Requires advertised logpoint support.");
        gtk_editable_set_text(GTK_EDITABLE(condition), value.condition); gtk_editable_set_text(GTK_EDITABLE(log), value.log_message);
        RuntimeBreakpointTag(condition, "condition", i); RuntimeBreakpointTag(log, "log", i);
        GtkWidget *actions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
        row->enabled = GTK_CHECK_BUTTON(gtk_check_button_new_with_label("Enabled"));
        gtk_check_button_set_active(row->enabled, value.enabled != 0);
        RuntimeBreakpointTag(GTK_WIDGET(row->enabled), "enabled", i);
        gtk_box_append(GTK_BOX(actions), GTK_WIDGET(row->enabled));
        const char *labels[] = {"Apply", "Reset fields", "Remove"};
        const char *tags[] = {"apply", "reset", "remove"};
        for (size_t n = 0U; n < 3U; ++n) {
            GtkWidget *button = gtk_button_new_with_label(labels[n]);
            g_object_set_data(G_OBJECT(button), "umicom-breakpoint-row", row);
            if (n == 2U) g_object_set_data(G_OBJECT(button), "umicom-breakpoint-remove", GINT_TO_POINTER(1));
            RuntimeBreakpointTag(button, tags[n], i);
            gtk_widget_set_sensitive(button, status == UMI_STATUS_OK);
            runtime_bind_editor_action(runtime, button, n == 1U ? RuntimeBreakpointReset : RuntimeBreakpointApply);
            gtk_box_append(GTK_BOX(actions), button);
        }
        gtk_box_append(GTK_BOX(box), actions);
        gtk_list_box_append(GTK_LIST_BOX(runtime->debug_breakpoints_list), box);
    }
    if (snapshot.breakpoint_count == 0U)
        gtk_list_box_append(GTK_LIST_BOX(runtime->debug_breakpoints_list), runtime_label(
            "No source breakpoints. Use Add Breakpoint to choose a file and line.", 0.0F));
}
