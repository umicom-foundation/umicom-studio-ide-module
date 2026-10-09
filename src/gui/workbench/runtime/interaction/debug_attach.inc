/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/debug_attach.inc
 * PURPOSE: Expose explicit process attachment through the shared native debugger owner.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/debug_runtime/native_attach.h"
#include "debug_process_picker.inc"
static void RuntimeDebugAttach(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    if (!gtk_widget_get_mapped(GTK_WIDGET(button)) ||
        g_object_get_data(G_OBJECT(button), "umicom-attach-pending") != NULL)
        return;
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL)
        return;
    GObject *window = NULL;
    runtime = runtime_file_dialog_owner(request, &window);
    if (runtime == NULL || !UmiGtk4WindowIsOpen(GTK_WINDOW(window)))
        goto done;
    GtkWidget *pid = g_object_get_data(G_OBJECT(button), "umicom-attach-pid");
    GtkWidget *program = g_object_get_data(G_OBJECT(button), "umicom-attach-program");
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    UmiStudioWorkspaceSnapshot workspace;
    UmiStatus status = umi_studio_workspace_snapshot(services, &workspace);
    if (status == UMI_STATUS_OK && (!workspace.graph.open || !workspace.graph.trusted))
        status = UMI_STATUS_PERMISSION_DENIED;
    UmiStudioBuildService *build = umi_studio_services_build(services);
    const UmiBuildProfile *profile = umi_studio_build_service_profile(build);
    if (status == UMI_STATUS_OK &&
        (profile == NULL || !umi_path_equal(profile->source_directory, workspace.graph.root)))
        status = UMI_STATUS_INVALID_STATE;
    if (status == UMI_STATUS_OK && RuntimeProgramConsoleBusy(runtime))
        status = UMI_STATUS_BUSY;
    uint64_t process_id = 0U;
    if (status == UMI_STATUS_OK)
        status = UmiDebugNativeProcessIdRead(gtk_editable_get_text(GTK_EDITABLE(pid)), &process_id);
    if(status==UMI_STATUS_OK)
        status=RuntimeProcessSelectionValidate(pid,&workspace,process_id);
    /* The explicit button authorizes this PID once. No remembered process ID
     * auto-attaches when a workspace, panel or application is reopened. */
    if (status == UMI_STATUS_OK)
    {
        g_object_set_data(G_OBJECT(button), "umicom-attach-pending", GINT_TO_POINTER(1));
        status =
            UmiStudioDebuggerAttachNative(umi_studio_services_debugger(services), build, process_id,
                                          gtk_editable_get_text(GTK_EDITABLE(program)), 1);
        g_object_set_data(G_OBJECT(button), "umicom-attach-pending", NULL);
    }
    g_clear_object(&window);
    runtime = runtime_file_dialog_owner(request, &window);
    if (runtime != NULL)
    {
        runtime_show_notification(
            runtime, status == UMI_STATUS_OK ? "Attached to the selected process. Stop requests "
                                               "detachment; no build or source save was performed."
                                             : umi_status_text(status));
        runtime_debug_update_summary(runtime);
    }
done:
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
}
static GtkWidget *RuntimeDebugAttachCreate(UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *expander = gtk_expander_new("Attach to a running application");
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    GtkWidget *explanation = gtk_label_new(
        "Use the process ID of an application you started and are authorised to debug. "
        "The debugger may pause it. Select Use adapter above first. Stop requests detachment; "
        "operating-system permissions still apply.");
    gtk_label_set_wrap(GTK_LABEL(explanation), TRUE);
    gtk_label_set_xalign(GTK_LABEL(explanation), 0.0F);
    gtk_box_append(GTK_BOX(box), explanation);
    GtkWidget *pid = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(pid), "Decimal process ID");
    gtk_entry_set_input_purpose(GTK_ENTRY(pid), GTK_INPUT_PURPOSE_DIGITS);
    GtkWidget *program = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(program),
                                   "Optional absolute executable / symbols path");
    GtkWidget *attach = gtk_button_new_with_label("Attach to process");
    (void)umi_gtk4_automation_tag_widget(pid, "studio.debug.attach.pid");
    (void)umi_gtk4_automation_tag_widget(program, "studio.debug.attach.program");
    (void)umi_gtk4_automation_tag_widget(attach, "studio.debug.attach");
    /* A retained action owns its field objects, but only the weak workbench
     * binding authorizes access to the debugger service. */
    g_object_set_data_full(G_OBJECT(attach), "umicom-attach-pid", g_object_ref_sink(pid),
                           g_object_unref);
    g_object_set_data_full(G_OBJECT(attach), "umicom-attach-program", g_object_ref_sink(program),
                           g_object_unref);
    runtime_bind_editor_action(runtime, attach, RuntimeDebugAttach);
    gtk_box_append(GTK_BOX(box), pid);
    gtk_box_append(GTK_BOX(box), program);
    gtk_box_append(GTK_BOX(box), attach);
    g_signal_connect(pid,"changed",G_CALLBACK(RuntimeProcessManualEdit),NULL);
    GtkWidget *picker=RuntimeProcessPickerCreate(runtime,pid);
    if(picker!=NULL){
        GtkWidget *processes=gtk_expander_new("Find a running process");
        gtk_expander_set_child(GTK_EXPANDER(processes),picker);
        gtk_box_append(GTK_BOX(box),processes);
    }
    gtk_expander_set_child(GTK_EXPANDER(expander), box);
    return expander;
}
