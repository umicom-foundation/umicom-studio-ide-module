/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/polyglot_workspace.h
 *
 * PURPOSE:
 *   Publish Studio's public polyglot workspace contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_POLYGLOT_WORKSPACE_H
#define UMICOM_STUDIO_POLYGLOT_WORKSPACE_H
#include "umicom/compiler/compiler.h"
typedef struct UmiStudioPolyglotWorkspace { char workspace_id[UMI_COMPILER_ID_CAPACITY]; UmiCompilerProjectManifest manifest; UmiInteropGraph interop; UmiCompilerBuildGraph build; char active_unit[UMI_COMPILER_ID_CAPACITY]; uint32_t language_mask; bool dirty; uint64_t revision; } UmiStudioPolyglotWorkspace;
UmiStatus umi_studio_polyglot_workspace_init(UmiStudioPolyglotWorkspace *workspace,const char *workspace_id,const UmiCompilerProjectManifest *manifest);
UmiStatus umi_studio_polyglot_workspace_select_unit(UmiStudioPolyglotWorkspace *workspace,const char *unit_id);
size_t umi_studio_polyglot_workspace_language_count(const UmiStudioPolyglotWorkspace *workspace);
#endif
