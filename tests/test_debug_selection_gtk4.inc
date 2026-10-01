/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_debug_selection_gtk4.inc
 * PURPOSE: Exercise real debugger row callbacks, refresh identity and retained native controls.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/debugger.h"
#include "umicom/debug/selection.h"
/* Included by the existing isolated native fixture. No window is presented and
 * the debugger uses its memory transport; no debuggee or adapter is launched. */
static int VerifyDebugSelection(GtkApplication *application,const char *fixturePath)
{
    int failed=0;UmiStudioBootstrap *bootstrap=NULL;UmiStudioGtkWorkbench *workbench=NULL;
    UmiStudioServicesOptions serviceOptions={0};UmiStudioGtkWorkbenchOptions options={0};
    GtkWidget *retained=NULL,*stale=NULL,*threadButton=NULL;
    char *path=g_build_filename(fixturePath,"debug-source.c",NULL);
    REQUIRE(path!=NULL&&g_file_set_contents(path,"first\nA\xc2\xa3" "B\nlast\n",-1,NULL));
    REQUIRE(umi_studio_bootstrap_create_with_options(&serviceOptions,&bootstrap)==UMI_STATUS_OK);
    UmiStudioUi *ui=umi_studio_bootstrap_ui(bootstrap);UmiDocumentCoordinator *documents=umi_studio_ui_documents(ui);
    UmiUiDocumentViewModel *views=umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
    char viewId[UMI_UI_ID_CAPACITY];REQUIRE(umi_document_coordinator_open(documents,path,viewId,sizeof(viewId))==UMI_STATUS_OK);
    UmiUiDocumentViewSnapshot before,after;REQUIRE(umi_ui_document_view_model_find(views,viewId,&before)==UMI_STATUS_OK);
    const char *draft="first\nunsaved draft\nlast\n";before.dirty=1;before.cursor_offset=1;before.selection_length=2;
    REQUIRE(UmiUiDocumentViewModelUpsertText(views,&before,draft,strlen(draft))==UMI_STATUS_OK);
    UmiStudioDebuggerService *debugger=umi_studio_services_debugger(umi_studio_bootstrap_services(bootstrap));
    REQUIRE(umi_studio_debugger_service_initialize(debugger,"memory",NULL)==UMI_STATUS_OK);
    UmiDebugService *model=umi_studio_debugger_service_model(debugger);
    UmiDebugWorkspace *workspace=umi_studio_debugger_service_workspace(debugger);
    UmiDebugThreadSnapshot thread={0};strcpy(thread.id,"thread");strcpy(thread.session_id,"session");thread.stopped=1;
    REQUIRE(umi_debug_thread_registry_upsert(umi_debug_service_thread(model),&thread)==UMI_STATUS_OK);
    UmiDebugSessionSnapshot session={0};strcpy(session.id,"session");strcpy(session.configuration_id,"launch");
    REQUIRE(umi_debug_session_registry_upsert(umi_debug_service_session(model),&session)==UMI_STATUS_OK);
    UmiDebugLaunchConfigurationSnapshot configuration={0};strcpy(configuration.id,"launch");
    g_strlcpy(configuration.working_directory,fixturePath,sizeof(configuration.working_directory));
    REQUIRE(umi_debug_launch_configuration_registry_upsert(umi_debug_service_launch_configuration(model),&configuration)==UMI_STATUS_OK);
    UmiDebugStackFrameSnapshot frame={0};strcpy(frame.id,"frame");strcpy(frame.thread_id,thread.id);
    strcpy(frame.source_uri,"debug-source.c");frame.line=2;frame.column=4;
    REQUIRE(umi_debug_stack_frame_registry_upsert(umi_debug_service_stack_frame(model),&frame)==UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_create_with_options(application,ui,umi_studio_bootstrap_desktop_shell(bootstrap),&options,&workbench)==UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_workspace_open_surface(workbench,UMI_STUDIO_SURFACE_DEBUG,NULL)==UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_refresh(workbench)==UMI_STATUS_OK);
    GtkWidget *root=GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    retained=find_tag(root,"studio.debug.frame.0");REQUIRE(GTK_IS_BUTTON(retained));g_object_ref(retained);
    REQUIRE(umi_studio_gtk_workbench_refresh(workbench)==UMI_STATUS_OK);REQUIRE(find_tag(root,"studio.debug.frame.0")==retained);
    g_signal_emit_by_name(retained,"clicked");
    REQUIRE(umi_ui_document_view_model_find(views,viewId,&after)==UMI_STATUS_OK&&after.cursor_offset==9&&after.selection_length==0&&after.dirty);
    /* A changed target invalidates the still-attached button before a timer
     * refresh. Its stale source cannot move the editor back to line one. */
    stale=find_tag(root,"studio.debug.frame.0");REQUIRE(GTK_IS_BUTTON(stale));g_object_ref(stale);
    frame.line=1;frame.column=1;REQUIRE(umi_debug_stack_frame_registry_upsert(umi_debug_service_stack_frame(model),&frame)==UMI_STATUS_OK);
    g_signal_emit_by_name(stale,"clicked");
    REQUIRE(umi_ui_document_view_model_find(views,viewId,&after)==UMI_STATUS_OK&&after.cursor_offset==9);
    char *missingPath=g_build_filename(fixturePath,"missing-debug-source.c",NULL);
    int missingAbsent=missingPath!=NULL&&!g_file_test(missingPath,G_FILE_TEST_EXISTS);g_free(missingPath);REQUIRE(missingAbsent);
    strcpy(frame.source_uri,"missing-debug-source.c");frame.line=1;
    REQUIRE(umi_debug_stack_frame_registry_upsert(umi_debug_service_stack_frame(model),&frame)==UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_refresh(workbench)==UMI_STATUS_OK);
    GtkWidget *missing=find_tag(root,"studio.debug.frame.0");REQUIRE(GTK_IS_BUTTON(missing));g_signal_emit_by_name(missing,"clicked");
    REQUIRE(umi_ui_document_view_model_find(views,viewId,&after)==UMI_STATUS_OK&&after.cursor_offset==9);
    g_signal_emit_by_name(retained,"clicked");g_signal_emit_by_name(stale,"clicked");
    REQUIRE(umi_ui_document_view_model_find(views,viewId,&after)==UMI_STATUS_OK&&after.cursor_offset==9);
    REQUIRE(umi_debug_stack_frame_registry_remove(umi_debug_service_stack_frame(model),"frame")==UMI_STATUS_OK);
    strcpy(frame.id,"replacement");strcpy(frame.source_uri,"debug-source.c");
    REQUIRE(umi_debug_stack_frame_registry_upsert(umi_debug_service_stack_frame(model),&frame)==UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_refresh(workbench)==UMI_STATUS_OK);
    UmiDebugWorkspaceSnapshot selected;REQUIRE(umi_debug_workspace_snapshot(workspace,&selected)==UMI_STATUS_OK);
    REQUIRE(strcmp(selected.selected_frame_id,"replacement")==0);
    threadButton=find_tag(root,"studio.debug.thread.0");REQUIRE(GTK_IS_BUTTON(threadButton));g_object_ref(threadButton);
    char *text=NULL;size_t length=0;REQUIRE(UmiUiDocumentViewModelCopyText(views,viewId,&text,&length)==UMI_STATUS_OK);
    int same=length==strlen(draft)&&strcmp(text,draft)==0;UmiUiDocumentViewModelFreeText(text);REQUIRE(same);
    umi_studio_gtk_workbench_destroy(workbench);workbench=NULL;umi_studio_bootstrap_destroy(bootstrap);bootstrap=NULL;
    g_signal_emit_by_name(retained,"clicked");g_signal_emit_by_name(stale,"clicked");g_signal_emit_by_name(threadButton,"clicked");
    REQUIRE(drain_context());REQUIRE(all_windows_unpresented());
cleanup:
    umi_studio_gtk_workbench_destroy(workbench);umi_studio_bootstrap_destroy(bootstrap);
    if(retained!=NULL)g_object_unref(retained);
    if(stale!=NULL)g_object_unref(stale);
    if(threadButton!=NULL)g_object_unref(threadButton);
    g_free(path);return failed;
}
