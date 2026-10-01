/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_debug_selection.c
 * PURPOSE: Verify Studio rejects inactive native inspection before changing Framework selections.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/debugger.h"
#include "umicom/debug/selection.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do {if(!(x)){fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#x);exit(1);}}while(0)
#define OK(x) CHECK((x)==UMI_STATUS_OK)
int main(void)
{
    UmiStudioDebuggerService *service=NULL;OK(umi_studio_debugger_service_create(&service));
    UmiDebugService *model=umi_studio_debugger_service_model(service);
    UmiDebugWorkspace *workspace=umi_studio_debugger_service_workspace(service);
    for(size_t i=0;i<2;++i){
        UmiDebugThreadSnapshot thread={0};(void)snprintf(thread.id,sizeof(thread.id),"t%zu",i);thread.stopped=1;
        OK(umi_debug_thread_registry_upsert(umi_debug_service_thread(model),&thread));
        UmiDebugStackFrameSnapshot frame={0};(void)snprintf(frame.id,sizeof(frame.id),"f%zu",i);strcpy(frame.thread_id,thread.id);
        OK(umi_debug_stack_frame_registry_upsert(umi_debug_service_stack_frame(model),&frame));
    }
    UmiDebugViewStamp before,after;OK(UmiDebugWorkspaceViewStamp(workspace,&before));
    CHECK(umi_studio_debugger_service_select_thread(service,"t1")==UMI_STATUS_INVALID_STATE);
    CHECK(umi_studio_debugger_service_select_frame(service,"f0")==UMI_STATUS_INVALID_STATE);
    CHECK(umi_studio_debugger_service_select_scope(service,"missing")==UMI_STATUS_INVALID_STATE);
    OK(UmiDebugWorkspaceViewStamp(workspace,&after));CHECK(UmiDebugViewStampEqual(&before,&after));
    /* Explicit memory-mode initialization preserves toolkit-neutral selection;
     * this test does not start an adapter or run any target program. */
    OK(umi_studio_debugger_service_initialize(service,"memory",NULL));
    OK(umi_studio_debugger_service_select_thread(service,"t1"));
    OK(umi_studio_debugger_service_select_frame(service,"f1"));
    OK(UmiDebugWorkspaceViewStamp(workspace,&after));CHECK(strcmp(after.selectedThread,"t1")==0&&strcmp(after.selectedFrame,"f1")==0);
    umi_studio_debugger_service_destroy(service);return 0;
}
