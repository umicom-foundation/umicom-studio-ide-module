/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/debug_watches.inc
 * PURPOSE: Keep watch drafts stable and separate local properties from explicit execution.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

typedef struct RuntimeWatchRow {
    UmiDebugWatchEdit *edit;
    GtkEntry *expression;
    GtkCheckButton *enabled;
} RuntimeWatchRow;
static void RuntimeWatchFree(gpointer data)
{
    RuntimeWatchRow *row = data;
    if (row != NULL) { UmiDebugWatchEditDestroy(row->edit); g_free(row); }
}
static void RuntimeWatchAction(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    RuntimeWatchRow *row = g_object_get_data(G_OBJECT(button), "umicom-watch-row");
    if (row == NULL) return;
    int action = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button), "umicom-watch-action"));
    UmiStudioDebuggerService *service = umi_studio_services_debugger(umi_studio_ui_services(runtime->owner->ui));
    UmiStatus status = UmiDebugWatchEditValidate(runtime_debug_workspace(runtime), row->edit);
    UmiDebugWatchSnapshot saved;
    if (status == UMI_STATUS_OK) status = UmiDebugWatchEditRead(row->edit, &saved);
    if (status == UMI_STATUS_OK && action == 1) {
        gtk_editable_set_text(GTK_EDITABLE(row->expression), saved.expression);
        gtk_check_button_set_active(row->enabled, saved.enabled != 0);
        return;
    }
    if (status == UMI_STATUS_OK && action == 3) {
        /* Never silently discard a draft or evaluate an unapplied expression. */
        if (strcmp(saved.expression, gtk_editable_get_text(GTK_EDITABLE(row->expression))) != 0 ||
            saved.enabled != gtk_check_button_get_active(row->enabled)) {
            runtime_show_notification(runtime, "Apply or reset the watch fields before choosing Evaluate."); return;
        }
        status = UmiStudioDebuggerEvaluateWatch(service, row->edit);
    } else if (status == UMI_STATUS_OK) {
        UmiDebugWatchSettings settings; UmiDebugWatchChange change;
        if (action != 2) status = UmiDebugWatchSettingsInit(&settings, gtk_check_button_get_active(row->enabled),
            gtk_editable_get_text(GTK_EDITABLE(row->expression)));
        if (status == UMI_STATUS_OK) status = UmiStudioDebuggerEditWatch(service, row->edit,
            action == 2 ? NULL : &settings, action == 2, &change);
    }
    /* Refresh may destroy this row. Only copied scalars are used afterwards. */
    (void)runtime_refresh_services(runtime);
    if (status == UMI_STATUS_OK) runtime_show_notification(runtime, action == 3 ?
        "Watch evaluated in the selected paused frame. The shown value is a capture." :
        action == 2 ? "Watch removed." : "Watch settings applied. No expression was evaluated.");
    else {
        char message[360];
        (void)snprintf(message, sizeof message, "%s: %s.",
            action == 3 ? "Evaluation did not publish a value; review debugger state before another attempt" :
            "Watch was not changed; refresh a stale row or correct its expression", umi_status_text(status));
        runtime_show_notification(runtime, message);
    }
}
static void RuntimeWatchTag(GtkWidget *widget, const char *kind, size_t index)
{
    char tag[96]; (void)snprintf(tag, sizeof tag, "studio.debug.watch.%s.%zu", kind, index);
    (void)umi_gtk4_automation_tag_widget(widget, tag);
}
static void RuntimeDebugWatchesRefresh(UmiStudioGtkRuntimeChrome *runtime, UmiDebugWorkspace *workspace,
    const UmiDebugViewStamp *stamp)
{
    if (runtime->debug_watches_list == NULL) return;
    const UmiDebugViewStamp *old = g_object_get_data(G_OBJECT(runtime->debug_watches_list), "umicom-watch-stamp");
    if (old != NULL && old->owner == stamp->owner && old->watches == stamp->watches &&
        old->sessions == stamp->sessions && old->configurations == stamp->configurations) return;
    UmiDebugViewStamp *stored = g_new(UmiDebugViewStamp, 1); *stored = *stamp;
    g_object_set_data_full(G_OBJECT(runtime->debug_watches_list), "umicom-watch-stamp", stored, g_free);
    runtime_clear_list_box(runtime->debug_watches_list);
    UmiDebugWorkspaceSnapshot snapshot;
    if (umi_debug_workspace_snapshot(workspace, &snapshot) != UMI_STATUS_OK) return;
    gtk_list_box_append(GTK_LIST_BOX(runtime->debug_watches_list), runtime_label(
        "Apply saves local fields. Evaluate uses the selected paused frame and may run target code. Values do not refresh automatically.", 0.0F));
    for (size_t i = 0U; i < snapshot.watch_count; ++i) {
        UmiDebugWatchSnapshot value;
        if (umi_debug_workspace_watch_at(workspace, i, &value) != UMI_STATUS_OK) continue;
        GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
        RuntimeWatchRow *row = g_new0(RuntimeWatchRow, 1);
        UmiStatus status = UmiDebugWatchEditCapture(workspace, i, &row->edit);
        g_object_set_data_full(G_OBJECT(box), "umicom-watch-row-owner", row, RuntimeWatchFree);
        GtkWidget *expression = gtk_entry_new(); row->expression = GTK_ENTRY(expression);
        char *safe = g_utf8_make_valid(value.expression, -1);
        gtk_editable_set_text(GTK_EDITABLE(expression), safe); g_free(safe);
        gtk_box_append(GTK_BOX(box), runtime_label("Watch expression", 0.0F));
        gtk_box_append(GTK_BOX(box), expression); RuntimeWatchTag(expression, "expression", i);
        gtk_accessible_update_property(GTK_ACCESSIBLE(expression), GTK_ACCESSIBLE_PROPERTY_LABEL, "Watch expression", -1);
        gtk_widget_set_tooltip_text(expression, "Nonempty expression, at most 1023 UTF-8 bytes. Apply never evaluates it.");
        char text[1536]; (void)snprintf(text, sizeof text, "%s: %s [%s]",
            !value.valid ? "Not evaluated" : "Last stored value", value.value, value.type);
        safe = g_utf8_make_valid(text, -1);
        GtkWidget *label = runtime_label(safe, 0.0F); g_free(safe);
        gtk_label_set_selectable(GTK_LABEL(label), TRUE); RuntimeWatchTag(label, "value", i);
        gtk_box_append(GTK_BOX(box), label);
        GtkWidget *actions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
        row->enabled = GTK_CHECK_BUTTON(gtk_check_button_new_with_label("Enabled"));
        gtk_check_button_set_active(row->enabled, value.enabled != 0);
        gtk_box_append(GTK_BOX(actions), GTK_WIDGET(row->enabled)); RuntimeWatchTag(GTK_WIDGET(row->enabled), "enabled", i);
        const char *labels[] = {"Apply", "Reset fields", "Remove", "Evaluate"};
        const char *tags[] = {"apply", "reset", "remove", "evaluate"};
        for (size_t n = 0U; n < 4U; ++n) {
            GtkWidget *button = gtk_button_new_with_label(labels[n]);
            g_object_set_data(G_OBJECT(button), "umicom-watch-row", row);
            g_object_set_data(G_OBJECT(button), "umicom-watch-action", GINT_TO_POINTER((int)n));
            RuntimeWatchTag(button, tags[n], i); gtk_widget_set_sensitive(button, status == UMI_STATUS_OK);
            runtime_bind_editor_action(runtime, button, RuntimeWatchAction);
            gtk_box_append(GTK_BOX(actions), button);
        }
        gtk_box_append(GTK_BOX(box), actions);
        gtk_list_box_append(GTK_LIST_BOX(runtime->debug_watches_list), box);
    }
    if (snapshot.watch_count == 0U) gtk_list_box_append(GTK_LIST_BOX(runtime->debug_watches_list),
        runtime_label("No watches. Use Add Watch to enter an expression.", 0.0F));
}
