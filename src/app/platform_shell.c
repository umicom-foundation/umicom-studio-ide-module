/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/platform_shell.c
 *
 * PURPOSE:
 *   Implement the Studio platform shell as a product-composition owner over reusable Framework platform aggregates.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * Studio composes reusable Framework capabilities here. Generic models and
 * engines stay in Framework so the same mechanisms can serve future products.
 */
#include "umicom/studio/platform_shell.h"
#include <stdlib.h>
#include <string.h>
struct UmiStudioPlatformShell {UmiStudioServices*services;UmiUiWorkbenchPlatform*workbench;UmiResourceCentre*resources;UmiProductCentre*products;UmiChartWorkspace*charts;UmiDesignerAuthoringSession*designer;UmiFrontendApplication*frontend;UmiStudioDeveloperWorkbench*developer;uint64_t revision;};
/* Provide the copy text operation used by this module and its client applications. */
static void copy_text(char*d,size_t c,const char*s){size_t n;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(d==NULL||c==0U)return;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==NULL)s="";n=strlen(s);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(n>=c)n=c-1U;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(n>0U)memcpy(d,s,n);d[n]='\0';}
/*
 * Initialise studio platform shell from caller-provided values so later operations receive
 * a known state.
 */
UmiStatus umi_studio_platform_shell_create(UmiStudioServices*services,UmiStudioPlatformShell**out){UmiStudioPlatformShell*p;UmiStatus s=UMI_STATUS_OK;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(out==NULL)return UMI_STATUS_INVALID_ARGUMENT;*out=NULL;p=calloc(1U,sizeof(*p));/* Protect caller-owned memory by checking that required state is available before it is used. */ if(p==NULL)return UMI_STATUS_OUT_OF_MEMORY;p->services=services;p->revision=1U;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_ui_workbench_platform_create(&p->workbench);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_platform_resource_centre_create(&p->resources);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_product_centre_create(&p->products);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_chart_workspace_create(&p->charts);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_designer_authoring_session_create(&p->designer);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_frontend_application_create(&p->frontend);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_studio_developer_workbench_create(&p->developer);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s!=UMI_STATUS_OK){umi_studio_platform_shell_destroy(p);return s;}*out=p;return UMI_STATUS_OK;}
/*
 * Release or reset state held by studio platform shell so the same storage can be reused
 * safely.
 */
void umi_studio_platform_shell_destroy(UmiStudioPlatformShell*p){/* Protect caller-owned memory by checking that required state is available before it is used. */ if(p==NULL)return;umi_studio_developer_workbench_destroy(p->developer);umi_frontend_application_destroy(p->frontend);umi_designer_authoring_session_destroy(p->designer);umi_chart_workspace_destroy(p->charts);umi_product_centre_destroy(p->products);umi_platform_resource_centre_destroy(p->resources);umi_ui_workbench_platform_destroy(p->workbench);free(p);}
/*
 * Provide the studio platform shell snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_platform_shell_snapshot(UmiStudioPlatformShell*p,UmiStudioPlatformShellSnapshot*o){/* Protect caller-owned memory by checking that required state is available before it is used. */ if(p==NULL||o==NULL)return UMI_STATUS_INVALID_ARGUMENT;memset(o,0,sizeof(*o));o->struct_size=(uint32_t)sizeof(*o);o->api_version=1U;copy_text(o->area_id,sizeof(o->area_id),"studio.platform-shell");copy_text(o->view_type,sizeof(o->view_type),"studio.platform-shell");copy_text(o->title,sizeof(o->title),"Platform Shell");copy_text(o->summary,sizeof(o->summary),"Unified Studio shell, developer services, resources, products, charts, designer and multi-frontend platform.");o->revision=p->revision;o->available=1;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(umi_ui_workbench_platform_snapshot(p->workbench,&o->workbench)!=UMI_STATUS_OK)return UMI_STATUS_INTERNAL_ERROR;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(umi_platform_resource_centre_snapshot(p->resources,&o->resources)!=UMI_STATUS_OK)return UMI_STATUS_INTERNAL_ERROR;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(umi_product_centre_snapshot(p->products,&o->products)!=UMI_STATUS_OK)return UMI_STATUS_INTERNAL_ERROR;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(umi_chart_workspace_snapshot(p->charts,&o->charts)!=UMI_STATUS_OK)return UMI_STATUS_INTERNAL_ERROR;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(umi_designer_authoring_session_snapshot(p->designer,&o->designer)!=UMI_STATUS_OK)return UMI_STATUS_INTERNAL_ERROR;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(umi_frontend_application_snapshot(p->frontend,&o->frontend)!=UMI_STATUS_OK)return UMI_STATUS_INTERNAL_ERROR;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(umi_studio_developer_workbench_snapshot(p->developer,&o->developer)!=UMI_STATUS_OK)return UMI_STATUS_INTERNAL_ERROR;o->item_count=o->workbench.list_items+o->resources.recent_count+o->products.marketplace_count+o->charts.panes_count+o->designer.templates_count+o->frontend.widgets_count+o->developer.item_count;return UMI_STATUS_OK;}
#define A(fn,type,field) type *fn(UmiStudioPlatformShell*p){return p!=NULL?p->field:NULL;}
A(umi_studio_platform_shell_workbench,UmiUiWorkbenchPlatform,workbench) A(umi_studio_platform_shell_resources,UmiResourceCentre,resources) A(umi_studio_platform_shell_products,UmiProductCentre,products) A(umi_studio_platform_shell_charts,UmiChartWorkspace,charts) A(umi_studio_platform_shell_designer,UmiDesignerAuthoringSession,designer) A(umi_studio_platform_shell_frontend,UmiFrontendApplication,frontend) A(umi_studio_platform_shell_developer,UmiStudioDeveloperWorkbench,developer)
#undef A

/*
 * Provide the studio platform shell activate project operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_platform_shell_activate_project(
    UmiStudioPlatformShell *shell,
    const UmiProjectWorkspaceSelectionRequest *request,
    UmiProjectWorkspaceSelectionSnapshot *out_selection)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if(shell==NULL||request==NULL)return UMI_STATUS_INVALID_ARGUMENT;
    status=umi_studio_developer_workbench_activate_project(
        shell->developer,request,out_selection);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if(status==UMI_STATUS_OK)shell->revision+=1U;
    return status;
}

/*
 * Provide the studio platform shell validate project operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_platform_shell_validate_project(
    UmiStudioPlatformShell *shell,
    UmiProjectWorkspaceValidationReport *out_report)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if(shell==NULL||out_report==NULL)return UMI_STATUS_INVALID_ARGUMENT;
    return umi_studio_developer_workbench_validate_project(
        shell->developer,out_report);
}

/*
 * Provide the studio platform shell prepare project workflow operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_platform_shell_prepare_project_workflow(
    UmiStudioPlatformShell *shell,
    const UmiDeveloperProjectWorkflowRequest *request,
    UmiDeveloperProjectWorkflowSnapshot *out_workflow)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if(shell==NULL||request==NULL)return UMI_STATUS_INVALID_ARGUMENT;
    status=umi_studio_developer_workbench_prepare_project_workflow(
        shell->developer,request,out_workflow);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if(status==UMI_STATUS_OK)shell->revision+=1U;
    return status;
}

/*
 * Provide the studio platform shell import project operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_platform_shell_import_project(
    UmiStudioPlatformShell *shell,
    const UmiDeveloperProjectBootstrapRequest *request,
    UmiDeveloperProjectBootstrapSnapshot *out_snapshot)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (shell == NULL || request == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_studio_developer_workbench_import_project(
        shell->developer, request, out_snapshot);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) shell->revision += 1U;
    return status;
}
