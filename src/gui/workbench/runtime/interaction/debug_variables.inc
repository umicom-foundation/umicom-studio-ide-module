/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/debug_variables.inc
 * PURPOSE: Present owned debugger child captures with explicit expand, refresh and collapse actions.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/


typedef struct RuntimeVariableRow {
    UmiDebugVariableTarget *target;
    GtkWidget *children;
    GtkWidget *collapse;
    GtkWidget *inspect;
    GtkWidget *label;
    char path[128];
    GtkEntry *assignment;
    GtkCheckButton *confirmAssignment;
    int assigning;
    /* These controls belong to this row. Hold the row box through callbacks so
     * a text-change observer can close the window without releasing the row. */
    GtkWidget *owner;
    GtkEntry *memoryOffset;
    GtkSpinButton *memoryCount;
    GtkTextView *memoryOutput;
    GtkLabel *memoryStatus;
    int readingMemory;
} RuntimeVariableRow;
static void RuntimeVariableFree(gpointer data)
{
    RuntimeVariableRow *row = data;
    if (row == NULL) return;
    UmiDebugVariableTargetDestroy(row->target); g_free(row);
}
static void RuntimeVariableTag(GtkWidget *widget, const char *kind, const char *path)
{
    char tag[192]; (void)snprintf(tag, sizeof tag, "studio.debug.variable.%s.%s", kind, path);
    (void)umi_gtk4_automation_tag_widget(widget, tag);
}
#include "debug_memory.inc"

static void RuntimeVariableClear(GtkWidget *box)
{
    GtkWidget *child;
    while ((child = gtk_widget_get_first_child(box)) != NULL) gtk_box_remove(GTK_BOX(box), child);
}
/* Bound the number of expanded child rows across this window. Collapsing a
 * branch frees its budget; roots retain the existing bounded registry limit. */
static size_t RuntimeVariableChildCount(GtkWidget *widget)
{
    size_t count = g_object_get_data(G_OBJECT(widget), "umicom-variable-child") != NULL ? 1U : 0U;
    for (GtkWidget *child = gtk_widget_get_first_child(widget); child != NULL; child = gtk_widget_get_next_sibling(child))
        count += RuntimeVariableChildCount(child);
    return count;
}
/* Editing the draft revokes its confirmation. Object-bound signals disconnect
 * automatically when a removed row loses its confirmation control. */
static void RuntimeVariableDraftChanged(GtkEditable *editable, gpointer data)
{
    (void)editable;
    gtk_check_button_set_active(GTK_CHECK_BUTTON(data), FALSE);
}
/* Rebuild debugger detail rows after capture invalidation without pumping a
 * second native operation or refreshing unrelated product services. */
static void runtime_debug_details_refresh(UmiStudioGtkRuntimeChrome *runtime);
static void RuntimeVariableAssign(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    RuntimeVariableRow *row = g_object_get_data(G_OBJECT(button), "umicom-variable-row");
    if (row == NULL || row->assigning || !gtk_check_button_get_active(row->confirmAssignment)) {
        runtime_show_notification(runtime, "Review the value and confirm that you want to change the paused program."); return;
    }
    UmiStudioDebuggerService *service = umi_studio_services_debugger(umi_studio_ui_services(runtime->owner->ui));
    /* Copy the draft before changing controls. Signal handlers and the following
     * refresh can remove a row; nothing below refresh borrows its storage. */
    char *value = g_strdup(gtk_editable_get_text(GTK_EDITABLE(row->assignment)));
    row->assigning = 1;
    UmiDebugVariableAssignment result;
    UmiStatus status = UmiStudioDebuggerAssignVariable(service, row->target, value, &result);
    g_free(value);
    row->assigning = 0;
    gtk_check_button_set_active(row->confirmAssignment, FALSE);
    if (result.attempted) runtime_debug_details_refresh(runtime);
    if (status == UMI_STATUS_OK && result.confirmed)
        runtime_show_notification(runtime, "Adapter confirmed the assignment. Choose Inspect scope to read current values before another edit.");
    else {
        char message[384];
        (void)snprintf(message, sizeof message, "%s: %s. %s",
            result.attempted ? "Assignment outcome is not confirmed" : "Assignment was not sent", umi_status_text(status),
            result.attempted ? "The program may have changed. Inspect scope before deciding what to do next; do not repeat blindly." :
                "Keep the draft, correct the value or inspect the current scope again.");
        runtime_show_notification(runtime, message);
    }
}
static void RuntimeVariableAssignmentControls(UmiStudioGtkRuntimeChrome *runtime, RuntimeVariableRow *row,
    GtkWidget *box)
{
    GtkWidget *expander = gtk_expander_new("Change value in paused program");
    GtkWidget *editor = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *entry = gtk_entry_new(); row->assignment = GTK_ENTRY(entry);
    gtk_entry_set_placeholder_text(row->assignment, "New value or expression in the program's language");
    gtk_accessible_update_property(GTK_ACCESSIBLE(entry), GTK_ACCESSIBLE_PROPERTY_LABEL, "New runtime value", -1);
    gtk_widget_set_tooltip_text(entry, "At most 1023 UTF-8 bytes. An empty value is sent only when explicitly confirmed. The adapter may execute target code.");
    GtkWidget *confirm = gtk_check_button_new_with_label("Apply this value to the paused program");
    row->confirmAssignment = GTK_CHECK_BUTTON(confirm);
    GtkWidget *apply = gtk_button_new_with_label("Set value");
    RuntimeVariableTag(entry, "assignment", row->path);
    RuntimeVariableTag(confirm, "confirm-assignment", row->path);
    RuntimeVariableTag(apply, "set-value", row->path);
    g_object_set_data(G_OBJECT(apply), "umicom-variable-row", row);
    UmiStudioDebuggerService *service = umi_studio_services_debugger(umi_studio_ui_services(runtime->owner->ui));
    UmiStatus available = UmiStudioDebuggerCheckVariableAssignment(service, row->target);
    gtk_widget_set_sensitive(apply, available == UMI_STATUS_OK);
    gtk_widget_set_tooltip_text(apply, available == UMI_STATUS_OK ?
        "Send one assignment. Existing captured values are discarded after an attempt; source files are unchanged." :
        available == UMI_STATUS_ALREADY_EXISTS ? "Several siblings have this name; the adapter cannot identify this row safely." :
        available == UMI_STATUS_NOT_IMPLEMENTED ? "This debug adapter does not advertise variable assignment." :
        "A current inspected native stop and an unambiguous captured variable are required.");
    g_signal_connect_object(entry, "changed", G_CALLBACK(RuntimeVariableDraftChanged), confirm, 0);
    runtime_bind_editor_action(runtime, apply, RuntimeVariableAssign);
    gtk_box_append(GTK_BOX(editor), entry); gtk_box_append(GTK_BOX(editor), confirm);
    gtk_box_append(GTK_BOX(editor), apply); gtk_expander_set_child(GTK_EXPANDER(expander), editor);
    gtk_box_append(GTK_BOX(box), expander);
}
static GtkWidget *RuntimeVariableBuild(UmiStudioGtkRuntimeChrome *runtime,
    UmiDebugVariableTarget *target, const UmiDebugRuntimeVariable *value,
    const char *path, int nested, int changed);
static void RuntimeVariableAction(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    RuntimeVariableRow *row = g_object_get_data(G_OBJECT(button), "umicom-variable-row");
    if (row == NULL) return;
    if (g_object_get_data(G_OBJECT(button), "umicom-variable-collapse") != NULL) {
        RuntimeVariableClear(row->children); gtk_widget_set_sensitive(row->collapse, FALSE); return;
    }
    const size_t budget = 1024U;
    size_t retained = RuntimeVariableChildCount(runtime->debug_variables_list) - RuntimeVariableChildCount(row->children);
    if (retained >= budget) {
        runtime_show_notification(runtime, "Collapse another variable branch before inspecting more children."); return;
    }
    UmiStudioDebuggerService *service = umi_studio_services_debugger(umi_studio_ui_services(runtime->owner->ui));
    UmiDebugVariablePage *page = NULL;
    UmiStatus status = UmiStudioDebuggerInspectVariable(service, row->target, &page);
    size_t count = UmiDebugVariablePageCount(page);
    if (status == UMI_STATUS_OK && count > budget - retained) status = UMI_STATUS_CAPACITY_EXCEEDED;
    if (status != UMI_STATUS_OK) {
        UmiDebugVariablePageDestroy(page);
        char message[384]; (void)snprintf(message, sizeof message,
            "Children were not replaced: %s. Existing values are earlier captures. Refresh debugger state or collapse another branch before retrying.",
            umi_status_text(status));
        runtime_show_notification(runtime, message); return;
    }
    /* Prepare all owned rows off-tree first. A failed allocation/capture leaves
     * the previous branch intact. No row identity is derived from its name. */
    GtkWidget *replacement = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    g_object_ref_sink(replacement);
    for (size_t i = 0U; status == UMI_STATUS_OK && i < count; ++i) {
        UmiDebugRuntimeVariable value; UmiDebugVariableTarget *target = NULL;
        status = UmiDebugVariablePageAt(page, i, &value);
        if (status == UMI_STATUS_OK) status = UmiDebugVariablePageTarget(page, i, &target);
        if (status != UMI_STATUS_OK) break;
        char path[128]; int written = snprintf(path, sizeof path, "%s.%zu", row->path, i);
        if (written < 0 || (size_t)written >= sizeof path) {
            UmiDebugVariableTargetDestroy(target); status = UMI_STATUS_CAPACITY_EXCEEDED; break;
        }
        gtk_box_append(GTK_BOX(replacement), RuntimeVariableBuild(runtime, target, &value, path, 1, 0));
    }
    UmiDebugVariablePageDestroy(page);
    if (status == UMI_STATUS_OK) {
        RuntimeVariableClear(row->children);
        if (count == 0U) gtk_box_append(GTK_BOX(replacement), runtime_label("The adapter returned no children.", 0.0F));
        gtk_box_append(GTK_BOX(row->children), replacement);
        gtk_widget_set_sensitive(row->collapse, TRUE);
        runtime_show_notification(runtime, "Children captured. Inspect children again to refresh this branch; values do not update automatically.");
    } else runtime_show_notification(runtime, "Could not prepare the child controls. The previous capture was retained.");
    g_object_unref(replacement);
}
static GtkWidget *RuntimeVariableBuild(UmiStudioGtkRuntimeChrome *runtime,
    UmiDebugVariableTarget *target, const UmiDebugRuntimeVariable *value,
    const char *path, int nested, int changed)
{
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    RuntimeVariableRow *row = g_new0(RuntimeVariableRow, 1);
    row->owner = box;
    row->target = target; (void)snprintf(row->path, sizeof row->path, "%s", path);
    g_object_set_data_full(G_OBJECT(box), "umicom-variable-owner", row, RuntimeVariableFree);
    if (nested) g_object_set_data(G_OBJECT(box), "umicom-variable-child", GINT_TO_POINTER(1));
    char *text = g_strdup_printf("%s%s = %s  [%s]", changed ? "• " : "", value->name, value->value, value->type);
    char *safe = g_utf8_make_valid(text, -1); g_free(text);
    GtkWidget *label = runtime_label(safe, 0.0F); g_free(safe);
    row->label = label;
    gtk_label_set_selectable(GTK_LABEL(label), TRUE);
    gtk_label_set_wrap(GTK_LABEL(label), TRUE); RuntimeVariableTag(label, "value", path);
    gtk_box_append(GTK_BOX(box), label);
    GtkWidget *actions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *inspect = gtk_button_new_with_label("Inspect children");
    row->inspect = inspect;
    row->collapse = gtk_button_new_with_label("Collapse");
    row->children = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_margin_start(row->children, 12);
    UmiStatus expandable = UmiDebugVariableTargetExpandable(target);
    gtk_widget_set_sensitive(inspect, expandable == UMI_STATUS_OK);
    gtk_widget_set_sensitive(row->collapse, FALSE);
    gtk_widget_set_tooltip_text(inspect, expandable == UMI_STATUS_OK ?
        "Request this object's children in the same paused frame. Adapter visualizers may run target code." :
        expandable == UMI_STATUS_ALREADY_EXISTS ? "This reference points to an ancestor; recursion is stopped here." :
        expandable == UMI_STATUS_CAPACITY_EXCEEDED ? "This reference or nesting depth exceeds the supported bound." :
        "No expandable reference in the current retained stopped frame.");
    GtkWidget *buttons[] = {inspect, row->collapse};
    for (size_t i = 0U; i < 2U; ++i) {
        g_object_set_data(G_OBJECT(buttons[i]), "umicom-variable-row", row);
        RuntimeVariableTag(buttons[i], i == 0U ? "inspect" : "collapse", path);
        runtime_bind_editor_action(runtime, buttons[i], RuntimeVariableAction);
        gtk_box_append(GTK_BOX(actions), buttons[i]);
    }
    g_object_set_data(G_OBJECT(row->collapse), "umicom-variable-collapse", GINT_TO_POINTER(1));
    gtk_box_append(GTK_BOX(box), actions); gtk_box_append(GTK_BOX(box), row->children);
    RuntimeVariableAssignmentControls(runtime, row, box);
    RuntimeMemoryControls(runtime, row, value, box);
    return box;
}
static void RuntimeDebugVariablesRender(UmiStudioGtkRuntimeChrome *runtime, UmiDebugWorkspace *workspace,
    const UmiDebugWorkspaceSnapshot *snapshot)
{
    if (runtime->debug_variables_list == NULL) return;
    gtk_list_box_append(GTK_LIST_BOX(runtime->debug_variables_list), runtime_label(
        "Variables are captured values. Inspect children requests one level; changing debugger state discards expanded branches.", 0.0F));
    for (size_t i = 0U; i < snapshot->visible_variable_count; ++i) {
        UmiDebugVariableSnapshot root; UmiDebugRuntimeVariable value = {0}; UmiDebugVariableTarget *target = NULL;
        if (umi_debug_workspace_variable_at(workspace, i, &root) != UMI_STATUS_OK) continue;
        UmiStatus status = UmiDebugVariableTargetCapture(workspace, i, &target);
        if (status == UMI_STATUS_OK) (void)UmiDebugVariableTargetRead(target, &value);
        else {
            /* Keep the previous root presentation even when there is no safe
             * expandable target, such as an idle retained session. */
            memcpy(value.name, root.name, sizeof root.name); memcpy(value.value, root.value, sizeof root.value);
            memcpy(value.type, root.type, sizeof root.type);
        }
        char path[32]; (void)snprintf(path, sizeof path, "%zu", i);
        gtk_list_box_append(GTK_LIST_BOX(runtime->debug_variables_list),
            RuntimeVariableBuild(runtime, target, &value, path, 0, root.changed));
    }
    if (snapshot->visible_variable_count == 0U) gtk_list_box_append(GTK_LIST_BOX(runtime->debug_variables_list),
        runtime_label("No variables in the selected scope. Pause and select a stack frame to inspect it.", 0.0F));
}
