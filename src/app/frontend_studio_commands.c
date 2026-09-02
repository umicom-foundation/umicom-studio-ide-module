/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/frontend_studio_commands.c
 *
 * PURPOSE:
 *   Implement the frontend studio commands behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/studio/frontend_studio_commands.h"
#include "umicom/studio/frontend_studio_inspector.h"
#include "umicom/studio/frontend_studio_preview.h"
#include <stdio.h>
#include <string.h>
static const UmiStudioFrontendCommand commands[UMI_STUDIO_FRONTEND_COMMAND_COUNT]={{"frontend.preview.start","Start Frontend Preview",1},{"frontend.preview.stop","Stop Frontend Preview",1},{"frontend.preview.reload","Reload Frontend Preview",1},{"frontend.preview.navigate","Navigate Frontend Preview",1},{"frontend.device.select","Select Responsive Device",1},{"frontend.changes.publish","Publish Frontend Changes",1},{"frontend.inspector.clear","Clear Browser Inspector",1},{"frontend.pane.activate","Activate Frontend Pane",1},{"frontend.snapshot","Show Frontend Snapshot",0}};
/*
 * Return the number of records represented by studio frontend commands without changing
 * their state.
 */
size_t umi_studio_frontend_commands_count(void){return UMI_STUDIO_FRONTEND_COMMAND_COUNT;}
/*
 * Find studio frontend command while leaving the underlying catalogue or model owned by
 * this module.
 */
const UmiStudioFrontendCommand *umi_studio_frontend_command_at(size_t i){return i<UMI_STUDIO_FRONTEND_COMMAND_COUNT?&commands[i]:NULL;}
/*
 * Perform studio frontend command through the module contract so client applications do
 * not duplicate its policy.
 */
UmiStatus umi_studio_frontend_command_execute(UmiStudioFrontendCentre *c,const char *id,const char *arg,char *m,size_t cap){UmiStatus st=UMI_STATUS_NOT_FOUND;uint64_t g=0U;UmiStudioFrontendCentreSnapshot snap;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(c==NULL||id==NULL)return UMI_STATUS_INVALID_ARGUMENT;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(strcmp(id,"frontend.preview.start")==0)st=umi_studio_frontend_preview_start(c->platform);else /* Protect caller-owned memory by checking that required state is available before it is used. */ if(strcmp(id,"frontend.preview.stop")==0)st=umi_studio_frontend_preview_stop(c->platform);else /* Protect caller-owned memory by checking that required state is available before it is used. */ if(strcmp(id,"frontend.preview.reload")==0)st=umi_studio_frontend_preview_reload(c->platform);else /* Protect caller-owned memory by checking that required state is available before it is used. */ if(strcmp(id,"frontend.preview.navigate")==0)st=arg!=NULL?umi_studio_frontend_preview_navigate(c->platform,arg):UMI_STATUS_INVALID_ARGUMENT;else /* Protect caller-owned memory by checking that required state is available before it is used. */ if(strcmp(id,"frontend.device.select")==0)st=arg!=NULL?umi_studio_frontend_preview_select_device(c->platform,arg):UMI_STATUS_INVALID_ARGUMENT;else /* Protect caller-owned memory by checking that required state is available before it is used. */ if(strcmp(id,"frontend.changes.publish")==0)st=umi_studio_frontend_centre_publish(c,&g);else /* Protect caller-owned memory by checking that required state is available before it is used. */ if(strcmp(id,"frontend.inspector.clear")==0){umi_studio_frontend_inspector_clear(c->platform);st=UMI_STATUS_OK;}else /* Protect caller-owned memory by checking that required state is available before it is used. */ if(strcmp(id,"frontend.pane.activate")==0)st=arg!=NULL?umi_studio_frontend_centre_activate(c,arg):UMI_STATUS_INVALID_ARGUMENT;else /* Protect caller-owned memory by checking that required state is available before it is used. */ if(strcmp(id,"frontend.snapshot")==0)st=umi_studio_frontend_centre_snapshot(c,&snap);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(m!=NULL&&cap>0U){/* Protect caller-owned memory by checking that required state is available before it is used. */ if(g>0U)(void)snprintf(m,cap,"Frontend command %s: %s; generation %llu",id,umi_status_text(st),(unsigned long long)g);/* Use this fallback path when the earlier condition does not apply. */ else (void)snprintf(m,cap,"Frontend command %s: %s",id,umi_status_text(st));}return st;}
