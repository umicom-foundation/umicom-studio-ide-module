/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/editor_workspace.c
 *
 * PURPOSE:
 *   Implement Studio editor-workspace composition over the reusable Framework editor session.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This is a product-neutral C23 model. The registry owns snapshot copies by
 * value; callers own external resources and coordinate cross-thread mutation.
 */
#include "umicom/studio/editor_workspace.h"
#include <stdlib.h>
#include <string.h>
struct UmiStudioEditorWorkspace{UmiStudioServices*services;UmiEditorSession*editor;uint64_t revision;};
/*
 * Initialise studio editor workspace from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_editor_workspace_create(UmiStudioServices*services,UmiStudioEditorWorkspace**out){UmiStudioEditorWorkspace*w;UmiStatus s;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(out==NULL)return UMI_STATUS_INVALID_ARGUMENT;*out=NULL;w=calloc(1U,sizeof(*w));/* Protect caller-owned memory by checking that required state is available before it is used. */ if(w==NULL)return UMI_STATUS_OUT_OF_MEMORY;w->services=services;w->revision=1U;s=umi_editor_session_create(&w->editor);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s!=UMI_STATUS_OK){free(w);return s;}*out=w;return UMI_STATUS_OK;}
/*
 * Release or reset state held by studio editor workspace so the same storage can be reused
 * safely.
 */
void umi_studio_editor_workspace_destroy(UmiStudioEditorWorkspace*w){/* Protect caller-owned memory by checking that required state is available before it is used. */ if(w==NULL)return;umi_editor_session_destroy(w->editor);free(w);}
/*
 * Provide the studio editor workspace snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_editor_workspace_snapshot(UmiStudioEditorWorkspace*w,UmiStudioEditorWorkspaceSnapshot*o){/* Protect caller-owned memory by checking that required state is available before it is used. */ if(w==NULL||o==NULL)return UMI_STATUS_INVALID_ARGUMENT;memset(o,0,sizeof(*o));o->struct_size=(uint32_t)sizeof(*o);o->api_version=1U;o->revision=w->revision;o->available=1;return umi_editor_session_snapshot(w->editor,&o->editor);}
/*
 * Provide the studio editor workspace session operation used by this module and its client
 * applications.
 */
UmiEditorSession *umi_studio_editor_workspace_session(UmiStudioEditorWorkspace*w){return w!=NULL?w->editor:NULL;}
