/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/first_use_tabs.inc
 *
 * PURPOSE:
 *   Exercise real editor switching and capture isolated visible workflow checkpoints.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/ui/gtk4/interaction_recording.h"
#include "umicom/ui/gtk4/workstation/tab_strip.h"

/* These are native widget integration journeys, not injected operating-system
 * mouse or keyboard events. Stable identity and focus are asserted separately
 * from activation signals, so refresh cannot silently replace a live control. */
static void TabJourneyPumpFor(gint64 duration)
{
    gint64 end=g_get_monotonic_time()+duration;
    while(g_get_monotonic_time()<end) {
        for(unsigned i=0U;i<32U && g_main_context_pending(NULL);++i)
            (void)g_main_context_iteration(NULL,FALSE);
        g_usleep(1000U);
    }
}
static void TabJourneyPump(void) { TabJourneyPumpFor(250000); }
static int TabJourneyCapture(GtkWidget *window, const char *directory, const char *name)
{
    TabJourneyPump();
    char *file=g_build_filename(directory,name,NULL);
    UmiStatus status=UmiGtk4RecordingCapturePng(window,file);
    GdkTexture *texture=status==UMI_STATUS_OK ? gdk_texture_new_from_filename(file,NULL) : NULL;
    int valid=texture!=NULL && gdk_texture_get_width(texture)>0 && gdk_texture_get_height(texture)>0;
    fprintf(stdout,"Visible checkpoint %s: %s\n",file,umi_status_text(status));
    g_clear_object(&texture);g_free(file);
    return valid;
}
static int ExerciseStableTabs(UmiStudioGtkWorkbench **slot, UmiStudioUi *ui,
    const char *directory, const char *mode)
{
    int failed=0;
    int visible=!strcmp(mode,"tab-visible-workflow") || !strcmp(mode,"tab-overflow");
    size_t count=!strcmp(mode,"tab-overflow") ? 16U : 3U;
    GtkWidget *buttons[16]={0},*editors[16]={0};
    GtkTextBuffer *buffers[16]={0};
    char ids[16][UMI_UI_ID_CAPACITY]={{0}},drafts[16][128]={{0}};
    char *paths[16]={0},*text=NULL;
    UmiStudioGtkWorkbench *workbench=*slot;
    GtkWidget *window=GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    UmiDocumentCoordinator *documents=umi_studio_ui_documents(ui);
    UmiUiDocumentViewModel *views=umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
    for(size_t index=0U;index<count;++index) {
        char filename[64];g_snprintf(filename,sizeof filename,"lesson-source-%02zu.c",index);
        paths[index]=g_build_filename(directory,filename,NULL);
        CHECK(g_file_set_contents(paths[index],"/* Saved source */\n",-1,NULL));
        CHECK(umi_studio_gtk_workbench_execute_command(workbench,"file.open",paths[index])==UMI_STATUS_OK);
        UmiDocumentWorkingCopySnapshot active;
        CHECK(umi_document_coordinator_active_snapshot(documents,&active)==UMI_STATUS_OK);
        g_strlcpy(ids[index],active.view_id,sizeof ids[index]);
        GtkWidget *tab=FindSourceTab(window,ids[index]);
        GtkWidget *editor=FindEditor(window,ids[index]);
        CHECK(GTK_IS_BUTTON(tab) && GTK_IS_TEXT_VIEW(editor));
        buttons[index]=g_object_ref(tab);editors[index]=g_object_ref(editor);
        buffers[index]=g_object_ref(gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor)));
        g_snprintf(drafts[index],sizeof drafts[index],"/* Source %zu keeps its own draft. */\nint value_%zu = %zu;\n",index,index,index+1U);
        gtk_text_buffer_begin_user_action(buffers[index]);
        gtk_text_buffer_set_text(buffers[index],drafts[index],-1);
        gtk_text_buffer_end_user_action(buffers[index]);
    }
    CHECK(umi_document_coordinator_count(documents)==count);
    if(visible) {
        gtk_window_set_default_size(GTK_WINDOW(window),900,640);
        gtk_window_present(GTK_WINDOW(window));TabJourneyPump();
        CHECK(gtk_widget_get_mapped(window));
        CHECK(TabJourneyCapture(window,directory,"tabs-open.png"));
    }
    /* Keep focus on an existing tab while real status synchronization runs.
     * A freshly constructed replacement with the same label is not sufficient. */
    CHECK(gtk_widget_grab_focus(buttons[0]));
    for(unsigned tick=0U;tick<8U;++tick)
        CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench)==UMI_STATUS_OK);
    if (visible) TabJourneyPumpFor(1700000); /* At least two ordinary 750 ms ticks. */
    CHECK(FindSourceTab(window,ids[0])==buttons[0]);
    CHECK(gtk_window_get_focus(GTK_WINDOW(window))==buttons[0]);
    for(size_t pass=0U;pass<2U;++pass) for(size_t index=0U;index<count;++index) {
        CHECK(FindSourceTab(window,ids[index])==buttons[index]);
        CHECK(gtk_widget_is_sensitive(buttons[index]));
        g_signal_emit_by_name(buttons[index],"clicked");
        UmiDocumentWorkingCopySnapshot active;
        CHECK(umi_document_coordinator_active_snapshot(documents,&active)==UMI_STATUS_OK);
        CHECK(!strcmp(active.view_id,ids[index]) && active.dirty);
        CHECK(FindEditor(window,ids[index])==editors[index] && Reachable(editors[index],window));
        CHECK(gtk_text_view_get_buffer(GTK_TEXT_VIEW(editors[index]))==buffers[index]);
        size_t length=0U;
        CHECK(UmiUiDocumentViewModelCopyText(views,ids[index],&text,&length)==UMI_STATUS_OK);
        int equal=length==strlen(drafts[index]) && !memcmp(text,drafts[index],length);
        UmiUiDocumentViewModelFreeText(text);text=NULL;
        CHECK(equal);
    }
    if(!strcmp(mode,"tab-retired-owner")) {
        UmiDocumentWorkingCopySnapshot before,after;
        CHECK(umi_document_coordinator_active_snapshot(documents,&before)==UMI_STATUS_OK);
        umi_studio_gtk_workbench_destroy(workbench);*slot=NULL;
        g_signal_emit_by_name(buttons[0],"clicked");
        CHECK(umi_document_coordinator_active_snapshot(documents,&after)==UMI_STATUS_OK);
        CHECK(!strcmp(before.view_id,after.view_id));
        goto cleanup;
    }
    if(visible) {
        CHECK(TabJourneyCapture(window,directory,"tabs-switched.png"));
        GtkWidget *strip=gtk_widget_get_ancestor(buttons[0],GTK_TYPE_SCROLLED_WINDOW);
        CHECK(GTK_IS_SCROLLED_WINDOW(strip));
        if(count>3U) {
            GtkAdjustment *scroll=gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(strip));
            CHECK(gtk_adjustment_get_upper(scroll)>gtk_adjustment_get_page_size(scroll));
            CHECK(gtk_widget_get_width(window)<1600);
        }
        for(size_t index=0U;index<count;++index) {
            g_signal_emit_by_name(buttons[index],"clicked");
            CHECK(umi_studio_gtk_workbench_execute_command(workbench,"file.save",NULL)==UMI_STATUS_OK);
            CHECK(g_file_get_contents(paths[index],&text,NULL,NULL));
            CHECK(!strcmp(text,drafts[index]));g_clear_pointer(&text,g_free);
        }
        CHECK(TabJourneyCapture(window,directory,"tabs-saved.png"));
    }
cleanup:
    if(text!=NULL)g_free(text);
    for(size_t index=0U;index<16U;++index) {
        g_clear_object(&buttons[index]);g_clear_object(&editors[index]);
        g_clear_object(&buffers[index]);g_free(paths[index]);
    }
    return failed;
}
