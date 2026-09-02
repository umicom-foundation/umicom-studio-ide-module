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
/**
 * Represent the studio polyglot workspace data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioPolyglotWorkspace { char workspace_id[UMI_COMPILER_ID_CAPACITY]; UmiCompilerProjectManifest manifest; UmiInteropGraph interop; UmiCompilerBuildGraph build; char active_unit[UMI_COMPILER_ID_CAPACITY]; uint32_t language_mask; bool dirty; uint64_t revision; } UmiStudioPolyglotWorkspace;
/**
 * Initialise studio polyglot workspace from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_polyglot_workspace_init(UmiStudioPolyglotWorkspace *workspace,const char *workspace_id,const UmiCompilerProjectManifest *manifest);
/**
 * Provide the studio polyglot workspace select unit operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_polyglot_workspace_select_unit(UmiStudioPolyglotWorkspace *workspace,const char *unit_id);
/**
 * Return the number of records represented by studio polyglot workspace language without
 * changing their state.
 */
size_t umi_studio_polyglot_workspace_language_count(const UmiStudioPolyglotWorkspace *workspace);
#endif
