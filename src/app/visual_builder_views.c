/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/visual_builder_views.c
 *
 * PURPOSE:
 *   Implement the visual builder views behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
 * Umicom Studio IDE - Visual Builder v2 toolkit-neutral workbench panes
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/visual_builder_views.h"
#include <stdint.h>
/* Provide the set text operation used by this module and its client applications. */
static UmiStatus set_text(UmiUiViewModel *view,const char *key,const char *text){UmiUiValue value;UmiStatus status=umi_ui_value_set_string(&value,text);return status==UMI_STATUS_OK?umi_ui_view_model_set_property(view,key,&value):status;}
/* Return the number of records represented by set without changing their state. */
static UmiStatus set_count(UmiUiViewModel *view,const char *key,size_t count){UmiUiValue value;UmiStatus status=umi_ui_value_set_integer(&value,(int64_t)count);return status==UMI_STATUS_OK?umi_ui_view_model_set_property(view,key,&value):status;}
/*
 * Provide the studio visual builder pane id operation used by this module and its client
 * applications.
 */
const char *umi_studio_visual_builder_pane_id(UmiStudioVisualBuilderPane pane){/* Select the behaviour associated with the requested command or state value. */ switch(pane){case UMI_STUDIO_VISUAL_BUILDER_PANE_CANVAS:return "canvas";case UMI_STUDIO_VISUAL_BUILDER_PANE_HIERARCHY:return "hierarchy";case UMI_STUDIO_VISUAL_BUILDER_PANE_PROPERTIES:return "properties";case UMI_STUDIO_VISUAL_BUILDER_PANE_BINDINGS:return "bindings";case UMI_STUDIO_VISUAL_BUILDER_PANE_INTERACTIONS:return "interactions";case UMI_STUDIO_VISUAL_BUILDER_PANE_TEMPLATES:return "templates";case UMI_STUDIO_VISUAL_BUILDER_PANE_PREVIEW:return "preview";case UMI_STUDIO_VISUAL_BUILDER_PANE_SOURCE:return "generated-source";default:return "unknown";}}
/*
 * Initialise studio visual builder view from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_visual_builder_view_create(UmiStudioVisualBuilderCentre *centre,UmiStudioVisualBuilderPane pane,UmiUiViewModel **out_view)
{
    UmiStudioVisualBuilderSnapshot snapshot; const char *pane_id; UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if(centre==NULL||out_view==NULL)return UMI_STATUS_INVALID_ARGUMENT;
    pane_id=umi_studio_visual_builder_pane_id(pane);
    /* Create this optional product surface only when its build option is enabled. */
    if(pane==0||pane>UMI_STUDIO_VISUAL_BUILDER_PANE_SOURCE)return UMI_STATUS_INVALID_ARGUMENT;
    *out_view=NULL; status=umi_studio_visual_builder_centre_snapshot(centre,&snapshot);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if(status==UMI_STATUS_OK)status=umi_ui_view_model_create(pane_id,umi_studio_visual_builder_capability_id(),UMI_UI_ROLE_PANE,out_view);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if(status==UMI_STATUS_OK)status=set_text(*out_view,"builder.pane",pane_id);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if(status==UMI_STATUS_OK)status=set_count(*out_view,"builder.components",snapshot.builder.document.component_count);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if(status==UMI_STATUS_OK)status=set_count(*out_view,"builder.selection",snapshot.builder.selected_count);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if(status==UMI_STATUS_OK)status=set_count(*out_view,"builder.bindings",snapshot.builder.binding_count);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if(status==UMI_STATUS_OK)status=set_count(*out_view,"builder.interactions",snapshot.builder.interaction_count);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if(status!=UMI_STATUS_OK&&*out_view!=NULL){umi_ui_view_model_destroy(*out_view);*out_view=NULL;} return status;
}
