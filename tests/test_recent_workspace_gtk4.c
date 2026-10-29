/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_recent_workspace_gtk4.c
 *
 * PURPOSE:
 *   Verify Welcome history removal, refreshed identities and retained controls.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include "umicom/studio/workspace.h"
#include "umicom/ui/gtk4/automation.h"
#include "umicom/platform/recent_items.h"
#include "workbench_window.h"
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#x); failed=1; goto cleanup; } } while (0)
static GtkWidget *Find(GtkWidget *root,const char *tag)
{ return umi_gtk4_automation_find_tagged_widget(root,tag); }

int main(int argc,char **argv)
{
    if(argc!=2)return 2;
    if(!gtk_init_check())return 77;
    int failed=0,changed=0,loaded=0;
    const char *mode=argv[1];
    GError *error=NULL;
    char *cwd=g_get_current_dir(),*data=g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    char *directory=g_dir_make_tmp("umicom-recent-workspace-XXXXXX",&error);
    char *state_dir=NULL,*metadata=NULL,*backup=NULL,*other_dir=NULL,*project=NULL,*source=NULL;
    char *contents=NULL;
    gsize length=0U;
    UmiSettings *settings=NULL;
    UmiStudioBootstrap *bootstrap=NULL;
    UmiStudioGtkWorkbench *workbench=NULL;
    UmiRecentItemRegistry *disk=NULL;
    GtkApplication *application=NULL;
    GtkWidget *remove=NULL,*refresh=NULL,*pin=NULL;
    CHECK(cwd && directory && g_chdir(directory)==0);changed=1;
    g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    CHECK(g_mkdir_with_parents("config",0700)==0);
    state_dir=g_build_filename(directory,".umicom",NULL);
    metadata=g_build_filename(state_dir,"studio-recent-items",NULL);
    backup=g_build_filename(state_dir,"history-before",NULL);
    other_dir=g_build_filename(directory,"other-directory",NULL);
    project=g_build_filename(directory,"project",NULL);
    source=g_build_filename(project,"learner.c",NULL);
    CHECK(g_mkdir(state_dir,0700)==0 && g_mkdir(project,0700)==0 && g_mkdir(other_dir,0700)==0);
    CHECK(g_file_set_contents(source,"int learner_value = 7;\n",-1,&error));
    if(strcmp(mode,"damaged")==0) CHECK(g_file_set_contents(metadata,"damaged history",-1,&error));
    CHECK(umi_studio_settings_create(&settings)==UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings,UMI_STUDIO_SETTING_AI_ALLOW_REMOTE,0)==UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings,UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS,0)==UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings,UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION,0)==UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings,UMI_STUDIO_SETTING_AUTO_SAVE,0)==UMI_STATUS_OK);
    CHECK(umi_studio_settings_save(settings,umi_studio_settings_default_path())==UMI_STATUS_OK);
    UmiStudioServicesOptions services_options={0};
    CHECK(umi_studio_bootstrap_create_with_options(&services_options,&bootstrap)==UMI_STATUS_OK);
    UmiStudioServices *services=umi_studio_bootstrap_services(bootstrap);
    UmiRecentItemRegistry *registry=umi_studio_services_recent_items(services);
    UmiRecentItemSnapshot item={0},found;
    (void)snprintf(item.id,sizeof(item.id),"%s","workspace-one");
    (void)snprintf(item.kind,sizeof(item.kind),"%s","workspace");
    (void)snprintf(item.label,sizeof(item.label),"%s","Learner project");
    if(strcmp(mode,"missing-folder")==0 || strcmp(mode,"refresh-availability")==0 || strcmp(mode,"pin-missing-folder")==0)
        (void)snprintf(item.uri,sizeof(item.uri),"%s/missing-project",directory);
    else (void)snprintf(item.uri,sizeof(item.uri),"%s",project);
    item.pinned=strcmp(mode,"pinned")==0 || strcmp(mode,"unpin")==0;item.open_count=4U;
    CHECK(umi_platform_recent_items_registry_upsert(registry,&item)==UMI_STATUS_OK);
    CHECK(umi_platform_recent_items_registry_find(registry,item.id,&item)==UMI_STATUS_OK);
    if(strcmp(mode,"damaged")!=0) CHECK(UmiRecentItemsSaveLocal(registry,metadata)==UMI_STATUS_OK);
    application=gtk_application_new("org.umicom.studio.recent-workspace-test",G_APPLICATION_NON_UNIQUE);
    CHECK(application && g_application_register(G_APPLICATION(application),NULL,&error));
    UmiStudioGtkWorkbenchOptions options={0};
    CHECK(umi_studio_gtk_workbench_create_with_options(application,umi_studio_bootstrap_ui(bootstrap),
        umi_studio_bootstrap_desktop_shell(bootstrap),&options,&workbench)==UMI_STATUS_OK);
    GtkWidget *root=GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    GtkWidget *remove_widget=Find(root,"studio.welcome.recent.remove.workspace-one");
    GtkWidget *refresh_widget=Find(root,"studio.welcome.recent.refresh");
    CHECK(GTK_IS_BUTTON(remove_widget) && GTK_IS_BUTTON(refresh_widget) && gtk_widget_get_sensitive(remove_widget));
    remove=g_object_ref(remove_widget);refresh=g_object_ref(refresh_widget);
    GtkWidget *pin_widget=Find(root,"studio.welcome.recent.pin.workspace-one");
    CHECK(GTK_IS_BUTTON(pin_widget) && gtk_widget_get_sensitive(pin_widget));
    pin=g_object_ref(pin_widget);
    if(strcmp(mode,"missing-folder")==0 || strcmp(mode,"refresh-availability")==0 || strcmp(mode,"pin-missing-folder")==0) {
        GtkWidget *open=Find(gtk_widget_get_parent(remove),"workspace.open-folder");
        CHECK(GTK_IS_BUTTON(open) && !gtk_widget_get_sensitive(open));
        CHECK(umi_studio_gtk_workbench_refresh(workbench)==UMI_STATUS_OK);
        CHECK(!gtk_widget_get_sensitive(open));
        g_signal_emit_by_name(open,"clicked");
        if(strcmp(mode,"refresh-availability")==0) {
            CHECK(g_mkdir(item.uri,0700)==0);
            g_signal_emit_by_name(refresh,"clicked");
            GtkWidget *new_remove=Find(root,"studio.welcome.recent.remove.workspace-one");
            CHECK(GTK_IS_BUTTON(new_remove));
            open=Find(gtk_widget_get_parent(new_remove),"workspace.open-folder");
            CHECK(GTK_IS_BUTTON(open) && gtk_widget_get_sensitive(open));
        }
    }
    UmiStudioWorkspaceSnapshot before,after;
    CHECK(umi_studio_workspace_snapshot(services,&before)==UMI_STATUS_OK && !before.graph.open);
    if(strcmp(mode,"retained")==0 || strcmp(mode,"pin-retained")==0) {
        umi_studio_gtk_workbench_destroy(workbench);workbench=NULL;
        umi_studio_bootstrap_destroy(bootstrap);bootstrap=NULL;
        guint windows=g_list_model_get_n_items(gtk_window_get_toplevels());
        g_signal_emit_by_name(remove,"clicked");
        g_signal_emit_by_name(refresh,"clicked");
        g_signal_emit_by_name(pin,"clicked");
        CHECK(g_list_model_get_n_items(gtk_window_get_toplevels())==windows);
        CHECK(UmiRecentItemsLoadLocal(metadata,&disk,&loaded)==UMI_STATUS_OK && loaded);
        CHECK(umi_platform_recent_items_registry_find(disk,item.id,&found)==UMI_STATUS_OK);
    } else if(strcmp(mode,"pin")==0 || strcmp(mode,"unpin")==0 || strncmp(mode,"pin-",4U)==0) {
        CHECK(strcmp(mode,"pin")==0 || strcmp(mode,"unpin")==0 ||
            strcmp(mode,"pin-stale")==0 || strcmp(mode,"pin-save-failure")==0 ||
            strcmp(mode,"pin-retry")==0 || strcmp(mode,"pin-cwd")==0 ||
            strcmp(mode,"pin-missing-folder")==0 || strcmp(mode,"pin-other-changed")==0);
        if(strcmp(mode,"pin-other-changed")==0) {
            UmiRecentItemSnapshot second=item;
            (void)snprintf(second.id,sizeof(second.id),"%s","workspace-two");
            second.last_opened=90U;
            CHECK(umi_platform_recent_items_registry_upsert(registry,&second)==UMI_STATUS_OK);
        }
        if(strcmp(mode,"pin-save-failure")==0 || strcmp(mode,"pin-retry")==0) {
            CHECK(g_rename(metadata,backup)==0 && g_mkdir(metadata,0700)==0);
            g_signal_emit_by_name(pin,"clicked");
            CHECK(umi_platform_recent_items_registry_find(registry,item.id,&found)==UMI_STATUS_OK);
            CHECK(found.pinned==item.pinned && found.revision==item.revision);
            CHECK(Find(root,"studio.welcome.recent.pin.workspace-one")==pin);
            CHECK(g_rmdir(metadata)==0 && g_rename(backup,metadata)==0);
            if(strcmp(mode,"pin-save-failure")==0) goto preserved;
        }
        if(strcmp(mode,"pin-cwd")==0) CHECK(g_chdir(other_dir)==0);
        if(strcmp(mode,"pin-stale")==0) {
            CHECK(umi_platform_recent_items_registry_touch(registry,item.id,99U)==UMI_STATUS_OK);
            g_signal_emit_by_name(pin,"clicked");
            CHECK(umi_platform_recent_items_registry_find(registry,item.id,&found)==UMI_STATUS_OK);
            CHECK(found.last_opened==99U && !found.pinned);
            GtkWidget *current=Find(root,"studio.welcome.recent.pin.workspace-one");
            CHECK(GTK_IS_BUTTON(current) && current!=pin);
            g_signal_emit_by_name(current,"clicked");
        } else g_signal_emit_by_name(pin,"clicked");
        CHECK(umi_platform_recent_items_registry_find(registry,item.id,&found)==UMI_STATUS_OK);
        int expectedPin=strcmp(mode,"unpin")!=0;
        CHECK(found.pinned==expectedPin && strcmp(found.uri,item.uri)==0);
        CHECK(found.open_count==(strcmp(mode,"pin-stale")==0?5U:4U));
        CHECK(UmiRecentItemsLoadLocal(metadata,&disk,&loaded)==UMI_STATUS_OK && loaded);
        UmiRecentItemSnapshot restored;
        CHECK(umi_platform_recent_items_registry_find(disk,item.id,&restored)==UMI_STATUS_OK);
        CHECK(restored.pinned==expectedPin && restored.last_opened==found.last_opened);
        GtkWidget *current=Find(root,"studio.welcome.recent.pin.workspace-one");
        CHECK(GTK_IS_BUTTON(current) && current!=pin);
        CHECK(strcmp(gtk_button_get_label(GTK_BUTTON(current)),expectedPin?"Unpin project":"Pin project")==0);
        /* A retained pre-change button must not undo the newer preference. */
        uint64_t revision=found.revision;
        g_signal_emit_by_name(pin,"clicked");
        CHECK(umi_platform_recent_items_registry_find(registry,item.id,&found)==UMI_STATUS_OK);
        CHECK(found.pinned==expectedPin && found.revision==revision);
        if(strcmp(mode,"pin-other-changed")==0) {
            CHECK(umi_platform_recent_items_registry_find(disk,"workspace-two",&restored)==UMI_STATUS_OK);
            CHECK(restored.last_opened==90U && !restored.pinned);
            UmiRecentItemSnapshot ranked[2];size_t count=0U;
            UmiRecentItemQuery query={.kind="workspace"};
            CHECK(umi_platform_recent_items_registry_query(registry,&query,ranked,2U,&count)==UMI_STATUS_OK);
            CHECK(count==2U && strcmp(ranked[0].id,item.id)==0);
        }
        if(strcmp(mode,"pin-cwd")==0) {
            char *wrong=g_build_filename(other_dir,".umicom","studio-recent-items",NULL);
            int exists=g_file_test(wrong,G_FILE_TEST_EXISTS);
            g_free(wrong);CHECK(!exists);
        }
    } else if(strcmp(mode,"refresh")==0 || strcmp(mode,"unrelated")==0) {
        UmiRecentItemSnapshot second=item;
        (void)snprintf(second.id,sizeof(second.id),"%s","workspace-two");
        CHECK(umi_platform_recent_items_registry_upsert(registry,&second)==UMI_STATUS_OK);
        if(strcmp(mode,"refresh")==0) {
            CHECK(Find(root,"studio.welcome.recent.remove.workspace-two")==NULL);
            g_signal_emit_by_name(refresh,"clicked");
            CHECK(GTK_IS_BUTTON(Find(root,"studio.welcome.recent.remove.workspace-two")));
            CHECK(umi_platform_recent_items_registry_count(registry)==2U);
        } else {
            g_signal_emit_by_name(remove,"clicked");
            CHECK(umi_platform_recent_items_registry_find(registry,item.id,&found)==UMI_STATUS_NOT_FOUND);
            CHECK(umi_platform_recent_items_registry_find(registry,second.id,&found)==UMI_STATUS_OK);
            CHECK(UmiRecentItemsLoadLocal(metadata,&disk,&loaded)==UMI_STATUS_OK && loaded);
            CHECK(umi_platform_recent_items_registry_find(disk,second.id,&found)==UMI_STATUS_OK);
        }
    } else if(strcmp(mode,"stale")==0) {
        CHECK(umi_platform_recent_items_registry_touch(registry,item.id,99U)==UMI_STATUS_OK);
        g_signal_emit_by_name(remove,"clicked");
        CHECK(umi_platform_recent_items_registry_find(registry,item.id,&found)==UMI_STATUS_OK && found.last_opened==99U);
        GtkWidget *current=Find(root,"studio.welcome.recent.remove.workspace-one");
        CHECK(GTK_IS_BUTTON(current) && current!=remove);
        g_signal_emit_by_name(current,"clicked");
        CHECK(umi_platform_recent_items_registry_find(registry,item.id,&found)==UMI_STATUS_NOT_FOUND);
    } else if(strcmp(mode,"damaged")==0) {
        g_signal_emit_by_name(remove,"clicked");
        CHECK(umi_platform_recent_items_registry_find(registry,item.id,&found)==UMI_STATUS_OK);
        CHECK(g_file_get_contents(metadata,&contents,&length,&error) && strcmp(contents,"damaged history")==0);
        g_clear_pointer(&contents,g_free);
    } else {
        CHECK(strcmp(mode,"remove")==0 || strcmp(mode,"pinned")==0 || strcmp(mode,"missing-folder")==0 || strcmp(mode,"refresh-availability")==0 || strcmp(mode,"pin-missing-folder")==0 ||
            strcmp(mode,"save-failure")==0 || strcmp(mode,"retry")==0 || strcmp(mode,"cwd")==0);
        if(strcmp(mode,"save-failure")==0 || strcmp(mode,"retry")==0) {
            CHECK(g_rename(metadata,backup)==0 && g_mkdir(metadata,0700)==0);
            g_signal_emit_by_name(remove,"clicked");
            CHECK(umi_platform_recent_items_registry_find(registry,item.id,&found)==UMI_STATUS_OK);
            CHECK(Find(root,"studio.welcome.recent.remove.workspace-one")==remove);
            CHECK(g_rmdir(metadata)==0 && g_rename(backup,metadata)==0);
            if(strcmp(mode,"save-failure")==0) goto preserved;
        }
        if(strcmp(mode,"cwd")==0) CHECK(g_chdir(other_dir)==0);
        g_signal_emit_by_name(remove,"clicked");
        CHECK(umi_platform_recent_items_registry_find(registry,item.id,&found)==UMI_STATUS_NOT_FOUND);
        CHECK(Find(root,"studio.welcome.recent.remove.workspace-one")==NULL);
        CHECK(UmiRecentItemsLoadLocal(metadata,&disk,&loaded)==UMI_STATUS_OK && loaded);
        CHECK(umi_platform_recent_items_registry_count(disk)==0U);
        /* Repeating a retained, now-stale button cannot delete another item. */
        g_signal_emit_by_name(remove,"clicked");
        CHECK(umi_platform_recent_items_registry_count(registry)==0U);
    }
preserved:
    if(bootstrap!=NULL) {
        CHECK(umi_studio_workspace_snapshot(services,&after)==UMI_STATUS_OK);
        CHECK(after.graph.open==before.graph.open && after.graph.revision==before.graph.revision);
    }
    CHECK(g_file_get_contents(source,&contents,&length,&error));
    CHECK(strcmp(contents,"int learner_value = 7;\n")==0);
cleanup:
    umi_studio_gtk_workbench_destroy(workbench);
    umi_studio_bootstrap_destroy(bootstrap);
    umi_settings_destroy(settings);
    umi_platform_recent_items_registry_destroy(disk);
    g_clear_object(&remove);g_clear_object(&refresh);g_clear_object(&pin);g_clear_object(&application);
    if(changed && g_chdir(cwd)!=0)failed=1;
    if(changed){if(data!=NULL)g_setenv("UMICOM_STUDIO_DATA_PATH",data,TRUE);else g_unsetenv("UMICOM_STUDIO_DATA_PATH");}
    if(error!=NULL)fprintf(stderr,"%s\n",error->message);
    if(directory!=NULL)printf("Retained isolated history fixture: %s\n",directory);
    g_clear_error(&error);
    g_free(contents);g_free(cwd);g_free(data);g_free(directory);g_free(state_dir);g_free(metadata);
    g_free(backup);g_free(other_dir);g_free(project);g_free(source);
    return failed;
}
