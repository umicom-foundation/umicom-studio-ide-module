/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_recent_workspace_service.c
 *
 * PURPOSE:
 *   Check recent-history persistence without requiring a graphical display.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/services.h"
#include "umicom/studio/workspace.h"
#include "studio_test_state.h"
#include <string.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#x); failed=1; goto cleanup; } } while (0)
int main(int argc,char **argv)
{
    if(argc!=2)return 2;
    int pinning=strncmp(argv[1],"pin-",4U)==0;
    const char *mode=pinning?argv[1]+4:argv[1];
    int failed=0,started=0,loaded=0;
    StudioTestState state={0};
    UmiStudioServices *services=NULL;
    UmiSettings *settings=NULL;
    UmiRecentItemRegistry *disk=NULL;
    char *metadata=NULL,*other=NULL,*project=NULL,*source=NULL,*text=NULL;
    gsize size=0U;
    CHECK(StudioTestStateBegin(&state));started=1;
    metadata=g_build_filename(state.directory,".umicom","studio-recent-items",NULL);
    other=g_build_filename(state.directory,"another-directory",NULL);
    project=g_build_filename(state.directory,"project",NULL);
    source=g_build_filename(project,"notes.txt",NULL);
    CHECK(g_mkdir(other,0700)==0 && g_mkdir(project,0700)==0);
    CHECK(g_file_set_contents(source,"Project content is independent of history.\n",-1,NULL));
    if(strcmp(mode,"damaged")==0)CHECK(g_file_set_contents(metadata,"invalid history",-1,NULL));
    CHECK(umi_studio_settings_create(&settings)==UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings,UMI_STUDIO_SETTING_AI_ALLOW_REMOTE,0)==UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings,UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS,0)==UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings,UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION,0)==UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings,UMI_STUDIO_SETTING_AUTO_SAVE,0)==UMI_STATUS_OK);
    CHECK(umi_studio_settings_save(settings,umi_studio_settings_default_path())==UMI_STATUS_OK);
    UmiStudioServicesOptions options={0};
    CHECK(umi_studio_services_create_with_options(NULL,NULL,&options,&services)==UMI_STATUS_OK);
    UmiRecentItemRegistry *registry=umi_studio_services_recent_items(services);
    UmiRecentItemSnapshot item={0},found;
    (void)snprintf(item.id,sizeof(item.id),"%s","workspace");
    (void)snprintf(item.kind,sizeof(item.kind),"%s",strcmp(mode,"wrong-kind")==0?"lesson":"workspace");
    (void)snprintf(item.uri,sizeof(item.uri),"%s",project);
    (void)snprintf(item.label,sizeof(item.label),"%s","Owned fixture");
    item.pinned=pinning && strcmp(mode,"unset")==0;
    CHECK(umi_platform_recent_items_registry_upsert(registry,&item)==UMI_STATUS_OK);
    CHECK(umi_platform_recent_items_registry_find(registry,item.id,&item)==UMI_STATUS_OK);
    if(strcmp(mode,"damaged")!=0)CHECK(UmiRecentItemsSaveLocal(registry,metadata)==UMI_STATUS_OK);
    if(strcmp(mode,"open-workspace")==0)
        CHECK(umi_studio_services_open_workspace(services,project,0)==UMI_STATUS_OK);
    UmiStudioWorkspaceSnapshot before,after;
    CHECK(umi_studio_workspace_snapshot(services,&before)==UMI_STATUS_OK);
    UmiStatus expected=UMI_STATUS_OK;
    const char *id=item.id;
    if(strcmp(mode,"cwd")==0)CHECK(g_chdir(other)==0);
    else if(strcmp(mode,"damaged")==0)expected=UMI_STATUS_UNAVAILABLE;
    else if(strcmp(mode,"wrong-kind")==0)expected=UMI_STATUS_INVALID_ARGUMENT;
    else if(strcmp(mode,"missing")==0){id="not-recorded";expected=UMI_STATUS_NOT_FOUND;}
    else if(strcmp(mode,"null")==0) {
        if(pinning) CHECK(UmiStudioServicesPinRecentWorkspace(NULL,id,item.revision,1)==UMI_STATUS_INVALID_ARGUMENT);
        else CHECK(UmiStudioServicesForgetRecentWorkspace(NULL,id,item.revision)==UMI_STATUS_INVALID_ARGUMENT);
        id=NULL;expected=UMI_STATUS_INVALID_ARGUMENT;
    } else if(pinning && strcmp(mode,"stale")==0) {
        CHECK(umi_platform_recent_items_registry_touch(registry,item.id,88U)==UMI_STATUS_OK);
        expected=UMI_STATUS_INVALID_STATE;
    } else CHECK(strcmp(mode,"remove")==0 || strcmp(mode,"open-workspace")==0 ||
        (pinning && (strcmp(mode,"set")==0 || strcmp(mode,"unset")==0)));
    if(pinning) CHECK(UmiStudioServicesPinRecentWorkspace(services,id,item.revision,strcmp(mode,"unset")!=0)==expected);
    else CHECK(UmiStudioServicesForgetRecentWorkspace(services,id,item.revision)==expected);
    CHECK(umi_studio_workspace_snapshot(services,&after)==UMI_STATUS_OK);
    CHECK(before.graph.open==after.graph.open && before.graph.revision==after.graph.revision &&
        strcmp(before.graph.root,after.graph.root)==0);
    if(expected==UMI_STATUS_OK) {
        if(pinning) {
            CHECK(umi_platform_recent_items_registry_find(registry,item.id,&found)==UMI_STATUS_OK);
            CHECK(found.pinned==(strcmp(mode,"unset")!=0) && found.revision!=item.revision);
            CHECK(found.open_count==item.open_count && strcmp(found.uri,item.uri)==0);
            CHECK(UmiRecentItemsLoadLocal(metadata,&disk,&loaded)==UMI_STATUS_OK && loaded);
            CHECK(umi_platform_recent_items_registry_find(disk,item.id,&found)==UMI_STATUS_OK);
            CHECK(found.pinned==(strcmp(mode,"unset")!=0));
            CHECK(UmiStudioServicesPinRecentWorkspace(services,item.id,item.revision,0)==UMI_STATUS_INVALID_STATE);
        } else {
        CHECK(umi_platform_recent_items_registry_find(registry,item.id,&found)==UMI_STATUS_NOT_FOUND);
        CHECK(UmiRecentItemsLoadLocal(metadata,&disk,&loaded)==UMI_STATUS_OK && loaded);
        CHECK(umi_platform_recent_items_registry_find(disk,item.id,&found)==UMI_STATUS_NOT_FOUND);
        }
    } else {
        CHECK(umi_platform_recent_items_registry_find(registry,item.id,&found)==UMI_STATUS_OK);
        if(pinning && strcmp(mode,"stale")==0) CHECK(found.last_opened==88U && !found.pinned);
        else CHECK(found.revision==item.revision && found.pinned==item.pinned);
        if(strcmp(mode,"damaged")==0) {
            CHECK(g_file_get_contents(metadata,&text,&size,NULL) && strcmp(text,"invalid history")==0);
            g_clear_pointer(&text,g_free);
        }
    }
    CHECK(g_file_get_contents(source,&text,&size,NULL));
    CHECK(strcmp(text,"Project content is independent of history.\n")==0);
    if(strcmp(mode,"cwd")==0) {
        char *wrong=g_build_filename(other,".umicom","studio-recent-items",NULL);
        int exists=g_file_test(wrong,G_FILE_TEST_EXISTS);
        g_free(wrong);CHECK(!exists);
    }
cleanup:
    if(services!=NULL && UmiStudioServicesRelease(&services)!=UMI_STATUS_OK)failed=1;
    umi_settings_destroy(settings);
    umi_platform_recent_items_registry_destroy(disk);
    if(started && !StudioTestStateEnd(&state))failed=1;
    g_free(metadata);g_free(other);g_free(project);g_free(source);g_free(text);
    return failed;
}
