/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/first_use_project_controls.inc
 *
 * PURPOSE:
 *   Exercise visible Test and launch-only controls against a real generated C project.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* Included after the existing first-use helpers. Framework remains responsible
 * for project generation, saving, asynchronous jobs and process outcomes. */
static int FirstUseProjectControlsScenario(const char *name)
{
    return !strcmp(name,"visible-test-repair") || !strcmp(name,"visible-run-current");
}
static int ExerciseNativeProjectControls(UmiStudioGtkWorkbench *workbench,
    UmiStudioServices *services, UmiDocumentCoordinator *documents,
    const char *fixture, const char *mode)
{
    int failed=0;
    UmiBuildResult *result=NULL;
    GtkTextBuffer *buffer=NULL;
    UmiStudioBuildService *build=umi_studio_services_build(services);
    GtkWidget *window=GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    char *source=NULL,*saved=NULL,*disk=NULL,*draft=NULL;
    gsize savedLength=0U,diskLength=0U;
    UmiBuildProjectSessionSnapshot before, after;
    UmiDocumentWorkingCopySnapshot active;
    /* This establishes a usable executable through the actual Configure,
     * Build and Run buttons, including Project Settings' trust decision. */
    CHECK(ExerciseNativeBuild(workbench,services,documents,fixture,0)==0);
    CHECK(umi_build_result_create(&result)==UMI_STATUS_OK);
    const int runCurrent=!strcmp(mode,"visible-run-current");
    source=runCurrent
        ? g_build_filename(fixture,"Source project","src","main.c",NULL)
        : g_build_filename(fixture,"Source project","tests","test_main.c",NULL);
    CHECK(g_file_get_contents(source,&saved,&savedLength,NULL));
    CHECK(umi_studio_gtk_workbench_execute_command(workbench,"file.open",source)==UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench)==UMI_STATUS_OK);
    CHECK(umi_document_coordinator_active_snapshot(documents,&active)==UMI_STATUS_OK);
    GtkWidget *editor=FindEditor(window,active.view_id);
    CHECK(Reachable(editor,window));
    buffer=g_object_ref(gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor)));
    const char *rejectedSource=
        "/*-----------------------------------------------------------------------------\n"
        " * Umicom Studio IDE\n"
        " * File: tests/test_main.c\n"
        " *\n"
        " * PURPOSE:\n"
        " *   Deliberately reject this isolated project test to exercise repair.\n"
        " *\n"
        " * AUTHOR AND ORGANISATION:\n"
        " * Sammy Hegab\n"
        " * Umicom Foundation\n"
        " *\n"
        " * LICENCE:\n"
        " * MIT\n"
        " *---------------------------------------------------------------------------*/\n"
        "\n"
        "#include <stdio.h>\n"
        "int main(void) { fputs(\"Studio acceptance test rejected\\n\", stderr); return 1; }\n";
    const char *unsavedSource=
        "/*-----------------------------------------------------------------------------\n"
        " * Umicom Studio IDE\n"
        " * File: src/main.c\n"
        " *\n"
        " * PURPOSE:\n"
        " *   Preserve an invalid draft while launching the existing program.\n"
        " *\n"
        " * AUTHOR AND ORGANISATION:\n"
        " * Sammy Hegab\n"
        " * Umicom Foundation\n"
        " *\n"
        " * LICENCE:\n"
        " * MIT\n"
        " *---------------------------------------------------------------------------*/\n"
        "\n"
        "#error launch_only_must_not_compile_or_save_this_draft\n";
    const char *replacement=runCurrent ? unsavedSource : rejectedSource;
    gtk_text_buffer_begin_user_action(buffer);
    gtk_text_buffer_set_text(buffer,replacement,-1);
    gtk_text_buffer_end_user_action(buffer);
    CHECK(umi_document_coordinator_active_snapshot(documents,&active)==UMI_STATUS_OK && active.dirty);
    CHECK(UmiStudioBuildProgress(build,&before)==UMI_STATUS_OK && !before.active);
    CHECK(FirstUseBuildCommand(workbench,runCurrent ? UMI_STUDIO_COMMAND_BUILD_RUN_CURRENT : "build.test")==UMI_STATUS_OK);
    CHECK(WaitForWorkbenchBuild(workbench,build,result));
    CHECK(UmiStudioBuildProgress(build,&after)==UMI_STATUS_OK);
    /* A disabled or disconnected button cannot pass by exposing an old result. */
    CHECK(after.operation_id>before.operation_id && !after.active);
    CHECK(umi_document_coordinator_active_snapshot(documents,&active)==UMI_STATUS_OK);
    CHECK(g_file_get_contents(source,&disk,&diskLength,NULL));
    if (runCurrent) {
        CHECK(result->phase==UMI_BUILD_PHASE_RUN && result->status==UMI_STATUS_OK && result->exit_code==0);
        CHECK(after.completed_phase_count==1U);
        CHECK(strstr(result->output,"Studio native workflow complete")!=NULL);
        CHECK(active.dirty && diskLength==savedLength && memcmp(disk,saved,savedLength)==0);
        GtkTextIter first,last;
        gtk_text_buffer_get_bounds(buffer,&first,&last);
        draft=gtk_text_buffer_get_text(buffer,&first,&last,FALSE);
        CHECK(draft!=NULL && !strcmp(draft,unsavedSource));
        CHECK(TabJourneyCapture(window,fixture,"launch-only-preserved-draft.png"));
    } else {
        CHECK(result->phase==UMI_BUILD_PHASE_TEST && result->status!=UMI_STATUS_OK && result->exit_code!=0);
        CHECK(strstr(result->output,"Studio acceptance test rejected")!=NULL);
        CHECK(!active.dirty && !strcmp(disk,rejectedSource));
        CHECK(TabJourneyCapture(window,fixture,"test-rejected.png"));
        before=after;
        editor=FindEditor(window,active.view_id);
        CHECK(GTK_IS_TEXT_VIEW(editor) && gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor))==buffer);
        /* Repair the exact test input in the visible editor; no shell-side
         * replacement or fabricated success can stand in for the user journey. */
        gtk_text_buffer_begin_user_action(buffer);
        gtk_text_buffer_set_text(buffer,saved,-1);
        gtk_text_buffer_end_user_action(buffer);
        CHECK(FirstUseBuildCommand(workbench,"build.test")==UMI_STATUS_OK);
        CHECK(WaitForWorkbenchBuild(workbench,build,result));
        CHECK(UmiStudioBuildProgress(build,&after)==UMI_STATUS_OK && after.operation_id>before.operation_id);
        CHECK(result->phase==UMI_BUILD_PHASE_TEST && result->status==UMI_STATUS_OK && result->exit_code==0);
        CHECK(strstr(result->output,"100% tests passed")!=NULL);
        CHECK(umi_document_coordinator_active_snapshot(documents,&active)==UMI_STATUS_OK && !active.dirty);
        g_clear_pointer(&disk,g_free);
        CHECK(g_file_get_contents(source,&disk,&diskLength,NULL));
        CHECK(diskLength==savedLength && memcmp(disk,saved,savedLength)==0);
        CHECK(TabJourneyCapture(window,fixture,"test-repaired.png"));
    }
cleanup:
    if (failed && build!=NULL) umi_studio_build_service_cancel(build);
    umi_build_result_destroy(result);
    g_clear_object(&buffer);
    g_free(source);g_free(saved);g_free(disk);g_free(draft);
    return failed;
}
