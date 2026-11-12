/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_live_preview_reading_gtk4.c
 *
 * PURPOSE:
 *   Exercise structured live previews through real Studio controls without modifying editor drafts.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include "umicom/ui/gtk4/automation.h"
#include "umicom/ui/appearance.h"
#include "umicom/document/coordinator.h"
#include "workbench_window.h"
#include "support/widget_diagnostics.h"
#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); failed=1; goto cleanup; } } while (0)
typedef struct PreviewCallback {
    GtkWindow *window;
    GtkWidget *refresh;
    UmiUiDocumentViewModel *views;
    UmiUiDocumentViewSnapshot view;
    unsigned int calls;
    int close, change;
} PreviewCallback;
static void PreviewNotified(GObject *object, GParamSpec *spec, gpointer data)
{
    (void)object; (void)spec;
    PreviewCallback *state = data;
    if (++state->calls != 1U) return;
    if (state->close) gtk_window_destroy(state->window);
    else if (state->change) {
        state->view.dirty = 1;
        UmiStatus status = UmiUiDocumentViewModelUpsertText(state->views, &state->view,
            "# Changed during publication\n", strlen("# Changed during publication\n"));
        if (status != UMI_STATUS_OK) g_error("Cannot prepare changed draft: %d", (int)status);
    } else g_signal_emit_by_name(state->refresh, "clicked");
}
static char *BufferText(GtkTextBuffer *buffer)
{
    GtkTextIter first,last; gtk_text_buffer_get_bounds(buffer,&first,&last);
    return gtk_text_buffer_get_text(buffer,&first,&last,TRUE);
}
int main(int argc,char **argv)
{
    if (argc != 2) return 2;
    g_test_init(&argc,&argv,NULL);
    UmiTestWidgetDiagnosticsInstall();
    if (!gtk_init_check()) return 77;
    const char *name=argv[1];
    static const char *const known[] = {"markdown", "unicode", "code-font", "appearance", "source-preserved", "empty", "plain", "html", "retained-refresh", "close-summary", "close-buffer", "nested-refresh", "changed-source"};
    int recognized=0;
    for(size_t i=0U;i<sizeof(known)/sizeof(known[0]);++i)
        if(strcmp(name,known[i])==0)recognized=1;
    if(!recognized)return 2;
    int failed=0,changed=0,isolated=0;
    GError *error=NULL;
    char *original=g_get_current_dir(),*old_data=g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    char *directory=g_dir_make_tmp("umicom-reading-preview-XXXXXX",&error);
    GtkApplication *application=NULL;
    UmiStudioBootstrap *bootstrap=NULL;
    UmiStudioGtkWorkbench *workbench=NULL;
    UmiSettings *settings=NULL;
    GtkWidget *refresh=NULL,*text_view=NULL,*summary=NULL;
    GtkTextBuffer *prior=NULL;
    char *display=NULL,*source_copy=NULL;
    UmiStudioServicesOptions services_options={0};
    UmiStudioGtkWorkbenchOptions options={0};
    PreviewCallback callback={0};
    gulong handler=0;
    CHECK(original && directory && g_chdir(directory)==0); changed=1;
    g_unsetenv("UMICOM_STUDIO_DATA_PATH"); isolated=1;
    CHECK(g_mkdir_with_parents("config",0700)==0);
    CHECK(umi_studio_settings_create(&settings)==UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings,UMI_STUDIO_SETTING_AI_ALLOW_REMOTE,0)==UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings,UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS,0)==UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings,UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION,0)==UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings,UMI_STUDIO_SETTING_AUTO_SAVE,0)==UMI_STATUS_OK);
    CHECK(umi_studio_settings_save(settings,umi_studio_settings_default_path())==UMI_STATUS_OK);
    CHECK(umi_studio_bootstrap_create_with_options(&services_options,&bootstrap)==UMI_STATUS_OK);
    application=gtk_application_new("org.umicom.studio.reading-preview-test",G_APPLICATION_NON_UNIQUE);
    CHECK(g_application_register(G_APPLICATION(application),NULL,&error));
    UmiStudioUi *ui=umi_studio_bootstrap_ui(bootstrap);
    CHECK(umi_studio_gtk_workbench_create_with_options(application,ui,
        umi_studio_bootstrap_desktop_shell(bootstrap),&options,&workbench)==UMI_STATUS_OK);
    UmiDocumentCoordinator *documents=umi_studio_ui_documents(ui);
    UmiUiDocumentViewModel *views=umi_ui_workbench_documents(umi_studio_ui_workbench(ui));
    char id[UMI_UI_ID_CAPACITY];
    const char *filename=strcmp(name,"plain")==0?"sample.txt":strcmp(name,"html")==0?"sample.htm":"sample.md";
    CHECK(umi_document_coordinator_new(documents,filename,id,sizeof(id))==UMI_STATUS_OK);
    UmiUiDocumentViewSnapshot view;
    CHECK(umi_ui_document_view_model_find(views,id,&view)==UMI_STATUS_OK);
    /* Hex escapes consume following hexadecimal characters. Retain the old
     * fixture for review; separate literals preserve the intended code span. */
#if 0
    const char *draft="# Reading\n\nA **strong** word and \x60code\x60.\n";
#endif
    const char *draft="# Reading\n\nA **strong** word and \x60" "code" "\x60.\n";
    CHECK(strstr(draft, "`code`") != NULL);
    if(strcmp(name,"unicode")==0) draft="# \xc3\x85\xf0\x9f\x98\x80\n\nA **word**.\n";
    if(strcmp(name,"empty")==0) draft="";
    if(strcmp(name,"html")==0) draft="<h1>Reading</h1><p>HTML text</p>";
    view.dirty=1; view.cursor_offset=0U; view.selection_length=0U;
    CHECK(UmiUiDocumentViewModelUpsertText(views,&view,draft,strlen(draft))==UMI_STATUS_OK);
    CHECK(umi_document_coordinator_sync_active(documents)==UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_execute_command(workbench,"view.live-preview",NULL)==UMI_STATUS_OK);
    GtkWidget *root=GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    /* Give the existing weak diagnostic observer the actual window. This does
     * not retain the window or alter fatal-warning handling. */
    UmiTestWidgetDiagnosticsRoot(root);
    GtkWidget *found=umi_gtk4_automation_find_tagged_widget(root,"studio.live-preview.refresh");
    CHECK(GTK_IS_BUTTON(found));refresh=g_object_ref(found);
    found=umi_gtk4_automation_find_tagged_widget(root,"studio.live-preview.text");
    CHECK(GTK_IS_TEXT_VIEW(found));text_view=g_object_ref(found);
    found=umi_gtk4_automation_find_tagged_widget(root,"studio.live-preview.summary");
    CHECK(GTK_IS_LABEL(found));summary=g_object_ref(found);
    prior=g_object_ref(gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view)));
    UmiDocumentWorkingCopySnapshot before,after;
    CHECK(umi_document_coordinator_active_snapshot(documents,&before)==UMI_STATUS_OK);
    if(strcmp(name,"retained-refresh")==0){
        umi_studio_gtk_workbench_destroy(workbench);workbench=NULL;
        umi_studio_bootstrap_destroy(bootstrap);bootstrap=NULL;
        guint windows=g_list_model_get_n_items(gtk_window_get_toplevels());
        g_signal_emit_by_name(refresh,"clicked");
        CHECK(g_list_model_get_n_items(gtk_window_get_toplevels())==windows);
        CHECK(gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view))==prior);
        goto cleanup;
    }
    callback.window=GTK_WINDOW(root);callback.refresh=refresh;callback.views=views;
    CHECK(umi_ui_document_view_model_find(views,id,&callback.view)==UMI_STATUS_OK);
    if(strcmp(name,"close-summary")==0||strcmp(name,"close-buffer")==0||
        strcmp(name,"nested-refresh")==0||strcmp(name,"changed-source")==0){
        callback.close=g_str_has_prefix(name,"close-");
        callback.change=strcmp(name,"changed-source")==0;
        /* Buffer changes on each forced refresh. Summary notification needs a
         * changed source revision so GTK has a distinct label to publish. */
        if(strcmp(name,"close-summary")==0){
            CHECK(UmiUiDocumentViewModelUpsertText(views,&callback.view,"# Different\n",12U)==UMI_STATUS_OK);
            handler=g_signal_connect(summary,"notify::label",G_CALLBACK(PreviewNotified),&callback);
        }else handler=g_signal_connect(text_view,"notify::buffer",G_CALLBACK(PreviewNotified),&callback);
    }
    if(strcmp(name,"appearance")==0){
        UmiUiAppearanceModel *appearance=umi_ui_workbench_appearance(umi_studio_ui_workbench(ui));
        UmiUiAppearanceProfile profile;
        CHECK(umi_ui_appearance_model_active(appearance,&profile)==UMI_STATUS_OK);
        CHECK(umi_ui_appearance_model_clone(appearance,profile.profile_id,"preview-custom","Preview custom")==UMI_STATUS_OK);
        CHECK(umi_ui_appearance_model_find(appearance,"preview-custom",&profile)==UMI_STATUS_OK);
        g_strlcpy(profile.interface_font,"Sans",sizeof(profile.interface_font));
        g_strlcpy(profile.editor_font,"Monospace",sizeof(profile.editor_font));
        profile.interface_font_size=15.0;
        CHECK(umi_ui_appearance_model_upsert(appearance,&profile)==UMI_STATUS_OK);
        CHECK(umi_ui_appearance_model_set_active(appearance,profile.profile_id)==UMI_STATUS_OK);
        /* Ordinary synchronization must notice a profile change without a text edit. */
        CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench)==UMI_STATUS_OK);
    }else g_signal_emit_by_name(refresh,"clicked");
    if(callback.close){
        CHECK(callback.calls==1U);
        if(strcmp(name,"close-summary")==0) CHECK(gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view))==prior);
        goto cleanup;
    }
    if(handler!=0U){
        g_signal_handler_disconnect(text_view,handler);handler=0;
        CHECK(callback.calls==1U);
    }
    if(callback.change){
        CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench)==UMI_STATUS_OK);
    }
    GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    display=BufferText(buffer);CHECK(display!=NULL);
    if(callback.change)CHECK(strcmp(display,"Changed during publication\n")==0);
    else if(strcmp(name,"plain")==0)CHECK(strcmp(display,draft)==0);
    else if(strcmp(name,"html")==0)CHECK(strstr(display,"HTML text")!=NULL && strstr(display,"<h1>")==NULL);
    else if(strcmp(name,"empty")==0)CHECK(display[0]=='\0');
    else {
        CHECK(strstr(display,"# ")==NULL);
        GtkTextTag *heading=gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(buffer),"umicom-reading-heading-1");
        CHECK(heading!=NULL);
        GtkTextIter start;gtk_text_buffer_get_start_iter(buffer,&start);
        CHECK(gtk_text_iter_has_tag(&start,heading));
        if(strcmp(name,"unicode")!=0){
            GtkTextTag *code=gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(buffer),"umicom-reading-code");
            CHECK(code!=NULL);
        }
    }
    if(strcmp(name,"appearance")==0||strcmp(name,"code-font")==0){
        UmiUiAppearanceProfile profile;
        CHECK(umi_ui_appearance_model_active(umi_ui_workbench_appearance(umi_studio_ui_workbench(ui)),&profile)==UMI_STATUS_OK);
        GtkTextTag *tag=gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(buffer),
            strcmp(name,"appearance")==0?"umicom-reading-body":"umicom-reading-code");
        char *family=NULL;g_object_get(tag,"family",&family,NULL);
        int same=strcmp(family,strcmp(name,"appearance")==0?profile.interface_font:profile.editor_font)==0;
        g_free(family);CHECK(same);
    }
    size_t source_bytes=0U;
    CHECK(UmiUiDocumentViewModelCopyText(views,id,&source_copy,&source_bytes)==UMI_STATUS_OK);
    const char *expected_source=callback.change?"# Changed during publication\n":draft;
    CHECK(source_bytes==strlen(expected_source)&&memcmp(source_copy,expected_source,source_bytes)==0);
    CHECK(umi_document_coordinator_active_snapshot(documents,&after)==UMI_STATUS_OK);
    if(!callback.change)CHECK(before.document_id==after.document_id&&before.revision==after.revision&&
        before.dirty==after.dirty&&before.undo_count==after.undo_count&&before.redo_count==after.redo_count);
cleanup:
    if(handler!=0U){
        GObject *target=strcmp(name,"close-summary")==0?G_OBJECT(summary):G_OBJECT(text_view);
        if(target!=NULL)g_signal_handler_disconnect(target,handler);
    }
    g_clear_object(&prior);
    g_clear_object(&refresh);g_clear_object(&text_view);g_clear_object(&summary);
    umi_studio_gtk_workbench_destroy(workbench);
    umi_studio_bootstrap_destroy(bootstrap);umi_settings_destroy(settings);
    g_clear_object(&application);
    if(changed&&g_chdir(original)!=0)failed=1;
    if(isolated){if(old_data!=NULL){if(!g_setenv("UMICOM_STUDIO_DATA_PATH",old_data,TRUE))failed=1;}
        else g_unsetenv("UMICOM_STUDIO_DATA_PATH");}
    UmiUiDocumentViewModelFreeText(source_copy);g_free(display);
    if(error!=NULL)fprintf(stderr,"%s\n",error->message);
    if(directory!=NULL)printf("Retained isolated preview fixture: %s\n",directory);
    g_clear_error(&error);g_free(directory);g_free(original);g_free(old_data);
    return failed;
}
