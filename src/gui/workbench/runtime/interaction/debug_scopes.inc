/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/debug_scopes.inc
 * PURPOSE: Expose explicit scope capture through the shared variable branch controls.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/debug_runtime/scope_inspection.h"

static void RuntimeDebugScopesRender(UmiStudioGtkRuntimeChrome *runtime,
    UmiDebugWorkspace *workspace, const UmiDebugWorkspaceSnapshot *snapshot)
{
    if (runtime->debug_variables_list == NULL) return;
    gtk_list_box_append(GTK_LIST_BOX(runtime->debug_variables_list), runtime_label(
        "Scopes in this frame. Inspect scope requests its values only when you click. Expensive scopes may take longer or run adapter visualizers.", 0.0F));
    for (size_t i = 0U; i < snapshot->visible_scope_count; ++i) {
        UmiDebugScopeSnapshot scope;
        UmiDebugVariableTarget *target = NULL;
        if (umi_debug_workspace_scope_at(workspace, i, &scope) != UMI_STATUS_OK) continue;
        UmiStatus status = UmiDebugScopeInspectionCapture(workspace, i, &scope, &target);
        UmiDebugRuntimeVariable display = {0};
        memcpy(display.name, scope.name, sizeof scope.name);
        char path[48]; (void)snprintf(path, sizeof path, "scope.%zu", i);
        /* Framework owns reference validity and page lifetime. The existing
         * branch controls own the window budget. This container uses their action
         * as variables without selecting a different scope or refreshing roots. */
        GtkWidget *box = RuntimeVariableBuild(runtime, target, &display, path, 0, 0);
        RuntimeVariableRow *row = g_object_get_data(G_OBJECT(box), "umicom-variable-owner");
        char *title = g_strdup_printf("%s%s", scope.name,
            scope.expensive ? " — expensive scope (loaded on request)" : " — scope");
        char *safe = g_utf8_make_valid(title, -1); g_free(title);
        gtk_label_set_text(GTK_LABEL(row->label), safe); g_free(safe);
        gtk_button_set_label(GTK_BUTTON(row->inspect), "Inspect scope");
        if (status == UMI_STATUS_OK && UmiDebugVariableTargetExpandable(target) == UMI_STATUS_OK)
            gtk_widget_set_tooltip_text(row->inspect,
                "Capture this scope in the unchanged paused frame. Existing roots remain unchanged. Adapter visualizers may execute target code.");
        gtk_list_box_append(GTK_LIST_BOX(runtime->debug_variables_list), box);
    }
    if (snapshot->visible_scope_count == 0U) gtk_list_box_append(GTK_LIST_BOX(runtime->debug_variables_list),
        runtime_label("No scopes in this frame. Pause the program and select a retained stack frame.", 0.0F));
}
