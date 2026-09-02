/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/compiler_project_binding.h
 *
 * PURPOSE:
 *   Publish Studio's public compiler project binding contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_COMPILER_PROJECT_BINDING_H
#define UMICOM_STUDIO_COMPILER_PROJECT_BINDING_H
#include "umicom/compiler/compiler.h"
/**
 * Represent the studio compiler project binding data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioCompilerProjectBinding { char workspace_id[UMI_COMPILER_ID_CAPACITY]; char project_root[UMI_COMPILER_PATH_CAPACITY]; char profile_id[UMI_COMPILER_ID_CAPACITY]; UmiCompilerProjectManifest manifest; bool trusted; bool auto_configure; uint64_t revision; } UmiStudioCompilerProjectBinding;
/**
 * Initialise studio compiler project binding from caller-provided values so later
 * operations receive a known state.
 */
UmiStatus umi_studio_compiler_project_binding_init(UmiStudioCompilerProjectBinding *binding,const char *workspace_id,const char *project_root,const char *profile_id,const UmiCompilerProjectManifest *manifest);
/**
 * Check that studio compiler project binding satisfies its contract before another service
 * relies on it.
 */
UmiStatus umi_studio_compiler_project_binding_validate(const UmiStudioCompilerProjectBinding *binding,char *out_reason,size_t capacity);
#endif
