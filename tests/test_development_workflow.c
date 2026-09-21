/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_development_workflow.c
 * PURPOSE: Exercise production commands, compiler gating and the process-backed
 * debugger host. The child DAP peer is a protocol fixture, not an actual debugger.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/workspace.h"
#include "umicom/studio/commands.h"
#include "umicom/studio/debugger.h"
#include "umicom/developer_project/new_project.h"
#include "umicom/platform/threading.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define CHECK(c) do { if(!(c)){fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#c);result=1;goto done;} }while(0)
#define OK(c) do{UmiStatus st_=(c);if(st_!=UMI_STATUS_OK){fprintf(stderr,"%s: %s\n",#c,umi_status_text(st_));result=1;goto done;}}while(0)
int main(int argc,char **argv)
{
    if(argc!=3) return 2;
    const char *mode=argv[2]; int result=0; int created=0;
    UmiStudioBootstrap *bootstrap=NULL;UmiStudioServicesOptions options={0};
    UmiDeveloperProjectService *projects=NULL;UmiDeveloperProjectGenerationRequest request;
    UmiDeveloperProjectGeneratorReport report;UmiDeveloperProjectModel model;UmiBuildProfile profile;
    UmiBuildResult *buildResult=NULL; UmiStatus poll=UMI_STATUS_OK;
    char root[UMI_PATH_CAPACITY]={0},tmp[UMI_PATH_CAPACITY],name[160],source[UMI_PATH_CAPACITY],message[512];
    struct timespec now;CHECK(timespec_get(&now,TIME_UTC)==TIME_UTC);
    (void)snprintf(name,sizeof(name),"Umicom Studio Notes-%lld-%ld-%s",(long long)now.tv_sec,now.tv_nsec,mode);
    OK(umi_fs_temp_directory(tmp,sizeof(tmp)));OK(umi_fs_join(root,sizeof(root),tmp,name));CHECK(!umi_fs_exists(root));
    OK(umi_developer_project_service_create(&projects));umi_developer_project_generation_request_init(&request);
    strcpy(request.template_id,"developer.template.c23-console");strcpy(request.application_name,"Umicom Notes");
    strcpy(request.application_id,"org.umicom.notes");strcpy(request.repository_name,"umicom-notes");
    strcpy(request.target_name,"umicom_notes");strcpy(request.project_root,root);
    OK(UmiDeveloperProjectCreateNew(projects,&request,&report,&model,&profile));created=1;
    OK(umi_fs_join(source,sizeof(source),root,"src/main.c"));
    OK(umi_fs_write_text(source,"#include <stdio.h>\nint main(void){unsigned savedNotes=2;printf(\"Umicom Notes: %u\\n\",savedNotes);return 0;}\n"));
    OK(umi_studio_bootstrap_create_with_options(&options,&bootstrap));OK(umi_studio_bootstrap_start(bootstrap));
    UmiStudioServices *services=umi_studio_bootstrap_services(bootstrap);
    UmiStudioBuildService *build=umi_studio_services_build(services);
    UmiStudioDebuggerService *debugger=umi_studio_services_debugger(services);
    UmiCommandRegistry *commands=umi_ui_workbench_commands(umi_studio_ui_workbench(umi_studio_bootstrap_ui(bootstrap)));
    OK(umi_studio_workspace_open(services,root,0,0));OK(umi_studio_build_service_set_profile(build,&profile));
    CHECK(umi_command_registry_execute(commands,UMI_STUDIO_COMMAND_DEBUG_START,NULL,message,sizeof(message))==UMI_STATUS_PERMISSION_DENIED);
    CHECK(!UmiStudioBuildBusy(build) && !UmiStudioDebuggerNativeBusy(debugger));
    if(strcmp(mode,"untrusted")==0)goto done;
    OK(umi_studio_workspace_set_trusted(services,1));
    OK(UmiStudioDebuggerConfigureNative(debugger,"lldb",strcmp(mode,"adapter-missing")==0?"/definitely/missing/lldb-dap":argv[1]));
    if(strcmp(mode,"no-build-submitted")==0){
        OK(UmiStudioDebuggerQueueNative(debugger,build,&profile));
        CHECK(UmiStudioDebuggerPollNative(debugger,build)==UMI_STATUS_INVALID_STATE);
        CHECK(!UmiStudioDebuggerNativeBusy(debugger));goto done;
    }
    OK(umi_studio_debugger_service_add_breakpoint(debugger,source,1,0));
    OK(umi_command_registry_execute(commands,UMI_STUDIO_COMMAND_DEBUG_START,NULL,message,sizeof(message)));
    CHECK(UmiStudioDebuggerNativeBusy(debugger));
    CHECK(umi_command_registry_execute(commands,UMI_STUDIO_COMMAND_BUILD_PACKAGE,"background",message,sizeof(message))==UMI_STATUS_BUSY);
    if(strcmp(mode,"cancel-building")==0){
        OK(umi_command_registry_execute(commands,UMI_STUDIO_COMMAND_DEBUG_STOP,NULL,message,sizeof(message)));
    }
    OK(umi_build_result_create(&buildResult));
    for(unsigned attempt=0;attempt<15000U;++attempt){
        while(UmiStudioBuildCollect(build,buildResult)==UMI_STATUS_OK)
            printf("%s: %s\n%s\n",umi_build_phase_text(buildResult->phase),umi_status_text(buildResult->status),buildResult->output);
        if(!UmiStudioBuildBusy(build)) break;
        umi_thread_sleep_ms(10U);
    }
    CHECK(!UmiStudioBuildBusy(build));
    if(strcmp(mode,"profile-changed")==0){UmiBuildProfile changed=profile;strcpy(changed.configuration,"Release");OK(umi_studio_build_service_set_profile(build,&changed));}
    for(unsigned attempt=0;attempt<100U;++attempt){
        poll=UmiStudioDebuggerPollNative(debugger,build);
        if(poll!=UMI_STATUS_OK)break;
        UmiStudioDebuggerSnapshot snapshot;OK(umi_studio_debugger_service_snapshot(debugger,&snapshot));
        if(snapshot.variable_count!=0U || !UmiStudioDebuggerNativeBusy(debugger))break;
        umi_thread_sleep_ms(5U);
    }
    if(strcmp(mode,"adapter-missing")==0 || strcmp(mode,"profile-changed")==0){
        CHECK(poll!=UMI_STATUS_OK);CHECK(!UmiStudioDebuggerNativeBusy(debugger));goto done;
    }
    if(strcmp(mode,"cancel-building")==0){CHECK(!UmiStudioDebuggerNativeBusy(debugger));goto done;}
    CHECK(poll==UMI_STATUS_OK);
    UmiStudioDebuggerSnapshot snapshot;OK(umi_studio_debugger_service_snapshot(debugger,&snapshot));
    CHECK(snapshot.thread_count==1U && snapshot.stack_frame_count==1U && snapshot.variable_count==1U);
    CHECK(strcmp(snapshot.controller_state,"paused")==0);
    UmiDebugService *debugModel=umi_studio_debugger_service_model(debugger);
    UmiDebugVariableSnapshot variable;OK(umi_debug_variable_registry_at(umi_debug_service_variable(debugModel),0,&variable));
    CHECK(strcmp(variable.value,"2")==0);
    char watch[sizeof(((UmiDebugWatchSnapshot *)0)->id)];OK(umi_studio_debugger_service_add_watch(debugger,"savedNotes",watch,sizeof(watch)));
    UmiDebugWatchSnapshot value;OK(umi_debug_watch_registry_find(umi_debug_service_watch(debugModel),watch,&value));CHECK(strcmp(value.value,"2")==0);
    OK(umi_studio_debugger_service_next(debugger,0));
    for(unsigned attempt=0;attempt<30U;++attempt){OK(UmiStudioDebuggerPollNative(debugger,build));
        OK(umi_debug_variable_registry_at(umi_debug_service_variable(debugModel),0,&variable));
        if(strcmp(variable.value,"5")==0) break;
        umi_thread_sleep_ms(5U);}
    CHECK(strcmp(variable.value,"5")==0);
    OK(umi_command_registry_execute(commands,UMI_STUDIO_COMMAND_DEBUG_STOP,NULL,message,sizeof(message)));
    CHECK(!UmiStudioDebuggerNativeBusy(debugger));
    if(strcmp(mode,"compile-failure")==0){
        /* The older executable remains on disk. A failed new build must not launch it. */
        OK(umi_fs_write_text(source,"int main(void) { broken source }\n"));
        OK(umi_command_registry_execute(commands,UMI_STUDIO_COMMAND_DEBUG_START,NULL,message,sizeof(message)));
        for(unsigned attempt=0;attempt<10000U;++attempt){
            while(UmiStudioBuildCollect(build,buildResult)==UMI_STATUS_OK)printf("%s\n",buildResult->output);
            if(!UmiStudioBuildBusy(build)) break;
            umi_thread_sleep_ms(5U);
        }
        CHECK(UmiStudioDebuggerPollNative(debugger,build)!=UMI_STATUS_OK);
        CHECK(!UmiStudioDebuggerNativeBusy(debugger));
    } else {
        OK(umi_command_registry_execute(commands,UMI_STUDIO_COMMAND_BUILD_PACKAGE,"background",message,sizeof(message)));
        for(unsigned attempt=0;attempt<15000U;++attempt){
            while(UmiStudioBuildCollect(build,buildResult)==UMI_STATUS_OK)printf("%s\n",buildResult->output);
            if(!UmiStudioBuildBusy(build)) break;
            umi_thread_sleep_ms(5U);
        }
        CHECK(!UmiStudioBuildBusy(build));CHECK(buildResult->phase==UMI_BUILD_PHASE_PACKAGE && buildResult->status==UMI_STATUS_OK);
    }
done:
    umi_build_result_destroy(buildResult);umi_studio_bootstrap_destroy(bootstrap);umi_developer_project_service_destroy(projects);
    if(result==0 && created) (void)umi_fs_remove_tree(root);
    return result;
}
