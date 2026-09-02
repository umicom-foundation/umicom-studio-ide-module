/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/polyglot_workspace.c
 *
 * PURPOSE:
 *   Implement the polyglot workspace behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE | Polyglot workspace v2 | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/studio/polyglot_workspace.h"
#include <stdio.h>
#include <string.h>
/*
 * Initialise studio polyglot workspace from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_polyglot_workspace_init(UmiStudioPolyglotWorkspace *workspace,const char *workspace_id,const UmiCompilerProjectManifest *manifest)
{ size_t index; /* Protect caller-owned memory by checking that required state is available before it is used. */ if (workspace == NULL || workspace_id == NULL || workspace_id[0] == '\0' || manifest == NULL) return UMI_STATUS_INVALID_ARGUMENT; (void)memset(workspace,0,sizeof(*workspace)); (void)snprintf(workspace->workspace_id,sizeof(workspace->workspace_id),"%s",workspace_id); workspace->manifest = *manifest; /* Visit each bounded item once so every record receives the same rule. */ for (index = 0U; index < manifest->unit_count; ++index) workspace->language_mask |= UMI_COMPILER_LANGUAGE_BIT(manifest->units[index].language); /* Protect caller-owned memory by checking that required state is available before it is used. */ if (manifest->unit_count > 0U) (void)snprintf(workspace->active_unit,sizeof(workspace->active_unit),"%s",manifest->units[0].unit_id); workspace->revision = 1U; return UMI_STATUS_OK; }
/*
 * Provide the studio polyglot workspace select unit operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_polyglot_workspace_select_unit(UmiStudioPolyglotWorkspace *workspace,const char *unit_id)
{ /* Protect caller-owned memory by checking that required state is available before it is used. */ if (workspace == NULL || unit_id == NULL) return UMI_STATUS_INVALID_ARGUMENT; /* Protect caller-owned memory by checking that required state is available before it is used. */ if (umi_compiler_manifest_find_unit(&workspace->manifest,unit_id) == NULL) return UMI_STATUS_NOT_FOUND; (void)snprintf(workspace->active_unit,sizeof(workspace->active_unit),"%s",unit_id); workspace->revision += 1U; return UMI_STATUS_OK; }
/*
 * Return the number of records represented by studio polyglot workspace language without
 * changing their state.
 */
size_t umi_studio_polyglot_workspace_language_count(const UmiStudioPolyglotWorkspace *workspace) { size_t count = 0U; UmiCompilerLanguage language; /* Protect caller-owned memory by checking that required state is available before it is used. */ if (workspace == NULL) return 0U; /* Visit each bounded item once so every record receives the same rule. */ for (language = UMI_COMPILER_LANGUAGE_C; language <= UMI_COMPILER_LANGUAGE_UAI; language = (UmiCompilerLanguage)(language + 1)) /* Protect caller-owned memory by checking that required state is available before it is used. */ if ((workspace->language_mask & UMI_COMPILER_LANGUAGE_BIT(language)) != 0U) count += 1U; return count; }
