/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/compiler_project_binding.c
 *
 * PURPOSE:
 *   Implement the compiler project binding behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE | Compiler project bindings v2 | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/studio/compiler_project_binding.h"
#include <stdio.h>
#include <string.h>
/*
 * Initialise studio compiler project binding from caller-provided values so later
 * operations receive a known state.
 */
UmiStatus umi_studio_compiler_project_binding_init(UmiStudioCompilerProjectBinding *binding,const char *workspace_id,const char *project_root,const char *profile_id,const UmiCompilerProjectManifest *manifest)
{ /* Protect caller-owned memory by checking that required state is available before it is used. */ if (binding == NULL || workspace_id == NULL || workspace_id[0] == '\0' || project_root == NULL || project_root[0] == '\0' || profile_id == NULL || profile_id[0] == '\0' || manifest == NULL) return UMI_STATUS_INVALID_ARGUMENT; (void)memset(binding,0,sizeof(*binding)); (void)snprintf(binding->workspace_id,sizeof(binding->workspace_id),"%s",workspace_id); (void)snprintf(binding->project_root,sizeof(binding->project_root),"%s",project_root); (void)snprintf(binding->profile_id,sizeof(binding->profile_id),"%s",profile_id); binding->manifest = *manifest; binding->trusted = false; binding->auto_configure = true; binding->revision = 1U; return UMI_STATUS_OK; }
/*
 * Check that studio compiler project binding satisfies its contract before another service
 * relies on it.
 */
UmiStatus umi_studio_compiler_project_binding_validate(const UmiStudioCompilerProjectBinding *binding,char *out_reason,size_t capacity)
{ UmiStatus status; /* Protect caller-owned memory by checking that required state is available before it is used. */ if (binding == NULL || out_reason == NULL || capacity == 0U) return UMI_STATUS_INVALID_ARGUMENT; /* Protect caller-owned memory by checking that required state is available before it is used. */ if (binding->workspace_id[0] == '\0' || binding->project_root[0] == '\0' || binding->profile_id[0] == '\0') { (void)snprintf(out_reason,capacity,"Workspace, root and profile are required"); return UMI_STATUS_INVALID_STATE; } /* Protect caller-owned memory by checking that required state is available before it is used. */ if (!binding->trusted) { (void)snprintf(out_reason,capacity,"Workspace trust is required before compiler execution"); return UMI_STATUS_PERMISSION_DENIED; } status = umi_compiler_manifest_validate(&binding->manifest,out_reason,capacity); return status; }
