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
#include "umicom/studio/polyglot_workspace.h"
#include "umicom/base/text.h"
#include <stdlib.h>
#include <string.h>
/* Unchecked manifest traversal, truncating identities and revision wrap are superseded by Framework admission and staged workspace publication. Retain the prior workspace behavior for comparison. The previous implementation is retained for review. */
#if 0
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

#endif
/* Studio owns workspace selection; Framework owns manifest admission and language
 * projection. Stage the large inline value before touching the receiving owner,
 * including when the inputs borrow that owner's current fields. */
UmiStatus umi_studio_polyglot_workspace_init(UmiStudioPolyglotWorkspace *workspace,
    const char *workspace_id,const UmiCompilerProjectManifest *manifest)
{
    if (workspace == NULL || workspace_id == NULL || workspace_id[0] == '\0' || manifest == NULL)
        return UMI_STATUS_INVALID_ARGUMENT;
    char identity[UMI_COMPILER_ID_CAPACITY] = {0};
    UmiStatus status = umi_text_replace(identity,sizeof(identity),workspace_id);
    if (status != UMI_STATUS_OK) return status;
    uint32_t mask = 0U;
    status = umi_compiler_manifest_languages(manifest,&mask);
    if (status != UMI_STATUS_OK) return status;
    UmiStudioPolyglotWorkspace *candidate = calloc(1U,sizeof(*candidate));
    if (candidate == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    (void)memcpy(candidate->workspace_id,identity,sizeof(identity));
    (void)memcpy(&candidate->manifest,manifest,sizeof(*manifest));
    candidate->language_mask = mask;
    if (manifest->unit_count != 0U)
        (void)memcpy(candidate->active_unit,manifest->units[0].unit_id,
                     strlen(manifest->units[0].unit_id)+1U);
    candidate->revision = 1U;
    (void)memcpy(workspace,candidate,sizeof(*candidate));
    free(candidate);
    return UMI_STATUS_OK;
}

/* Check only the fields consumed by this workspace projection. Interop/build
 * graphs keep their own Framework contracts and are not executed by selection. */
static bool WorkspaceSelectionValid(const UmiStudioPolyglotWorkspace *workspace)
{
    if (workspace == NULL || workspace->revision == 0U ||
        workspace->workspace_id[0] == '\0' ||
        memchr(workspace->workspace_id,'\0',sizeof(workspace->workspace_id)) == NULL ||
        memchr(workspace->active_unit,'\0',sizeof(workspace->active_unit)) == NULL)
        return false;
    uint32_t mask = 0U;
    if (umi_compiler_manifest_languages(&workspace->manifest,&mask) != UMI_STATUS_OK ||
        mask != workspace->language_mask) return false;
    return workspace->active_unit[0] == '\0' ||
        umi_compiler_manifest_find_unit(&workspace->manifest,workspace->active_unit) != NULL;
}

UmiStatus umi_studio_polyglot_workspace_select_unit(UmiStudioPolyglotWorkspace *workspace,
    const char *unit_id)
{
    if (workspace == NULL || unit_id == NULL || unit_id[0] == '\0')
        return UMI_STATUS_INVALID_ARGUMENT;
    char identity[UMI_COMPILER_ID_CAPACITY] = {0};
    UmiStatus status = umi_text_replace(identity,sizeof(identity),unit_id);
    if (status != UMI_STATUS_OK) return status;
    if (!WorkspaceSelectionValid(workspace)) return UMI_STATUS_INVALID_STATE;
    if (umi_compiler_manifest_find_unit(&workspace->manifest,identity) == NULL)
        return UMI_STATUS_NOT_FOUND;
    /* Preserve the established successful-edit revision behavior, including
     * reselecting the current unit. The shared update refuses wrap before write. */
    return umi_text_update(workspace->active_unit,sizeof(workspace->active_unit),
                           identity,&workspace->revision);
}

size_t umi_studio_polyglot_workspace_language_count(const UmiStudioPolyglotWorkspace *workspace)
{
    if (!WorkspaceSelectionValid(workspace)) return 0U;
    size_t count = 0U;
    uint32_t mask = workspace->language_mask;
    while (mask != 0U) {
        count += (size_t)(mask & UINT32_C(1));
        mask >>= 1U;
    }
    return count;
}
