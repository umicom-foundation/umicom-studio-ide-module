/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/designer_preview.c
 *
 * PURPOSE:
 *   Bridge Studio designer state to Framework preview and starter-code generation without embedding generator logic in the GUI.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This Studio adapter keeps product composition thin: it delegates reusable
 * behaviour to Framework services and adds only Studio-specific coordination.
 */

#include "umicom/studio/designer_preview.h"
/*
 * Provide the studio designer generate preview operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_designer_generate_preview(UmiStudioDesigner *d,char *out,size_t cap,UmiDeclDiagnosticList *diags){return umi_studio_designer_preview(d,out,cap,diags);}
/*
 * Provide the studio designer generate c operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_designer_generate_c(UmiStudioDesigner *d,UmiStudioDeclarative *decl,char *out,size_t cap,UmiDeclDiagnosticList *diags){UmiDeclApplicationPlan plan;UmiStatus s;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(d==NULL||decl==NULL)return UMI_STATUS_INVALID_ARGUMENT;s=umi_studio_declarative_compile(decl,umi_designer_document_declarative(umi_studio_designer_document(d)),&plan,diags);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_decl_generate_c_main(&plan,out,cap);umi_decl_plan_dispose(&plan);return s;}
