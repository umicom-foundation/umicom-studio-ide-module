/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_test_source_navigation_gtk4.inc
 * PURPOSE: Exercise real Studio failure-location controls and retained callback lifetime.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

/* Included by the isolated native workspace fixture so its real bootstrap,
 * selection helpers and unpresented window policy remain authoritative. */
static GtkWidget *FindDiscoveredSource(GtkWidget *root, const char *itemId)
{
    if (root == NULL) return NULL;
    const UmiTestPlatformItemSnapshot *item = g_object_get_data(G_OBJECT(root), "umicom-test-source-item");
    if (GTK_IS_BUTTON(root) && item != NULL && strcmp(item->id, itemId) == 0) return root;
    for (GtkWidget *child=gtk_widget_get_first_child(root); child!=NULL; child=gtk_widget_get_next_sibling(child)) {
        GtkWidget *found=FindDiscoveredSource(child,itemId); if (found!=NULL) return found;
    }
    return NULL;
}
static int VerifyTestSourceLocations(GtkApplication *application, const char *fixturePath)
{
    int failed=0;
    UmiStudioBootstrap *bootstrap=NULL; UmiStudioGtkWorkbench *workbench=NULL;
    UmiStudioServicesOptions services_options={0}; UmiStudioGtkWorkbenchOptions options={0};
    GtkWidget *retained=NULL, *stale=NULL, *sourceRetained=NULL;
    char *path=g_build_filename(fixturePath,"failure-location-source.c",NULL);
    char *missing=g_build_filename(fixturePath,"failure-location-missing.c",NULL);
    REQUIRE(path!=NULL && missing!=NULL && !g_file_test(missing,G_FILE_TEST_EXISTS));
    REQUIRE(g_file_set_contents(path,"first\nA\xc2\xa3" "B\nlast\n",-1,NULL));
    REQUIRE(umi_studio_bootstrap_create_with_options(&services_options,&bootstrap)==UMI_STATUS_OK);
    UmiStudioUi *ui=umi_studio_bootstrap_ui(bootstrap);
    UmiDocumentCoordinator *documents=umi_studio_ui_documents(ui);
    UmiUiDocumentViewModel *views=umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
    char viewId[UMI_UI_ID_CAPACITY];
    REQUIRE(umi_document_coordinator_open(documents,path,viewId,sizeof(viewId))==UMI_STATUS_OK);
    UmiUiDocumentViewSnapshot before,after;
    REQUIRE(umi_ui_document_view_model_find(views,viewId,&before)==UMI_STATUS_OK);
    const char *draft="first\nunsaved draft\nlast\n";
    before.dirty=1; before.cursor_offset=1; before.selection_length=2;
    REQUIRE(UmiUiDocumentViewModelUpsertText(views,&before,draft,strlen(draft))==UMI_STATUS_OK);
    UmiTestPlatformService *platform=umi_studio_test_service_platform(
        umi_studio_services_tests(umi_studio_bootstrap_services(bootstrap)));
    REQUIRE(platform!=NULL);
    UmiTestPlatformItemRegistry *items=umi_test_platform_service_item(platform);
    UmiTestPlatformResultRegistry *results=umi_test_platform_service_result(platform);
    const char *ids[]={"native.source.a","native.source.b"};
    for(size_t i=0;i<2;++i) {
        UmiTestPlatformItemSnapshot item={0};
        g_strlcpy(item.id,ids[i],sizeof(item.id)); g_strlcpy(item.name,ids[i],sizeof(item.name));
        g_strlcpy(item.framework,"ctest",sizeof(item.framework)); g_strlcpy(item.kind,"test",sizeof(item.kind));
        g_strlcpy(item.working_directory,fixturePath,sizeof(item.working_directory));
        g_strlcpy(item.source_uri,i==0?path:missing,sizeof(item.source_uri));
        item.source_line=2; item.enabled=1; item.discovered=1;
        REQUIRE(umi_test_platform_item_registry_upsert(items,&item)==UMI_STATUS_OK);
        UmiTestPlatformResultSnapshot result={0};
        g_strlcpy(result.id,ids[i],sizeof(result.id)); g_strlcpy(result.item_id,ids[i],sizeof(result.item_id));
        g_strlcpy(result.session_id,"source-run",sizeof(result.session_id)); result.sequence=i+1;
        result.outcome=UMI_TEST_PLATFORM_OUTCOME_FAILED;
        int n=snprintf(result.failure_details,sizeof(result.failure_details),"%s:2:4: expected value",i==0?path:missing);
        REQUIRE(n>0 && (size_t)n<sizeof(result.failure_details));
        REQUIRE(umi_test_platform_result_registry_upsert(results,&result)==UMI_STATUS_OK);
    }
    REQUIRE(umi_studio_gtk_workbench_create_with_options(application,ui,
        umi_studio_bootstrap_desktop_shell(bootstrap),&options,&workbench)==UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_workspace_open_surface(workbench,UMI_STUDIO_SURFACE_TEST_EXPLORER,NULL)==UMI_STATUS_OK);
    REQUIRE(umi_studio_gtk_workbench_refresh(workbench)==UMI_STATUS_OK);
    GtkWidget *root=GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    GtkWidget *select=FindTestSelect(root,ids[0]); REQUIRE(GTK_IS_BUTTON(select));
    g_signal_emit_by_name(select,"clicked");
    retained=find_tag(root,"studio.tests.source-location.0"); REQUIRE(GTK_IS_BUTTON(retained)); g_object_ref(retained);
    REQUIRE(umi_studio_gtk_workbench_refresh(workbench)==UMI_STATUS_OK);
    REQUIRE(find_tag(root,"studio.tests.source-location.0")==retained);
    g_signal_emit_by_name(retained,"clicked");
    REQUIRE(umi_ui_document_view_model_find(views,viewId,&after)==UMI_STATUS_OK);
    REQUIRE(after.cursor_offset==9 && after.selection_length==0 && after.dirty);
    select=FindTestSelect(root,ids[1]); REQUIRE(GTK_IS_BUTTON(select)); g_signal_emit_by_name(select,"clicked");
    /* Removed rows and failed opens must never move the prior draft's caret. */
    g_signal_emit_by_name(retained,"clicked");
    stale=find_tag(root,"studio.tests.source-location.0"); REQUIRE(GTK_IS_BUTTON(stale)); g_object_ref(stale);
    g_signal_emit_by_name(stale,"clicked");
    REQUIRE(umi_ui_document_view_model_find(views,viewId,&after)==UMI_STATUS_OK && after.cursor_offset==9);
    GtkWidget *source=FindDiscoveredSource(root,ids[1]); REQUIRE(GTK_IS_BUTTON(source));
    g_signal_emit_by_name(source,"clicked");
    REQUIRE(umi_ui_document_view_model_find(views,viewId,&after)==UMI_STATUS_OK && after.cursor_offset==9);
    /* Mutating the result before the next UI refresh invalidates its old action. */
    UmiTestPlatformResultSnapshot changed;
    REQUIRE(umi_test_platform_result_registry_find(results,ids[1],&changed)==UMI_STATUS_OK);
    (void)snprintf(changed.failure_details,sizeof(changed.failure_details),"%s:1: changed",path);
    REQUIRE(umi_test_platform_result_registry_upsert(results,&changed)==UMI_STATUS_OK);
    g_signal_emit_by_name(stale,"clicked");
    REQUIRE(umi_ui_document_view_model_find(views,viewId,&after)==UMI_STATUS_OK && after.cursor_offset==9);
    sourceRetained=FindDiscoveredSource(root,ids[0]); REQUIRE(GTK_IS_BUTTON(sourceRetained)); g_object_ref(sourceRetained);
    REQUIRE(umi_test_platform_item_registry_remove(items,ids[0])==UMI_STATUS_OK);
    g_signal_emit_by_name(sourceRetained,"clicked");
    REQUIRE(umi_ui_document_view_model_find(views,viewId,&after)==UMI_STATUS_OK && after.cursor_offset==9);
    char *text=NULL; size_t length=0;
    REQUIRE(UmiUiDocumentViewModelCopyText(views,viewId,&text,&length)==UMI_STATUS_OK);
    int same=length==strlen(draft) && strcmp(text,draft)==0; UmiUiDocumentViewModelFreeText(text); REQUIRE(same);
    umi_studio_gtk_workbench_destroy(workbench); workbench=NULL;
    umi_studio_bootstrap_destroy(bootstrap); bootstrap=NULL;
    g_signal_emit_by_name(retained,"clicked"); g_signal_emit_by_name(stale,"clicked");
    g_signal_emit_by_name(sourceRetained,"clicked"); REQUIRE(drain_context()); REQUIRE(all_windows_unpresented());
cleanup:
    umi_studio_gtk_workbench_destroy(workbench); umi_studio_bootstrap_destroy(bootstrap);
    if(retained!=NULL) g_object_unref(retained);
    if(stale!=NULL) g_object_unref(stale);
    if(sourceRetained!=NULL) g_object_unref(sourceRetained);
    g_free(path); g_free(missing); return failed;
}
