/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_polyglot_workspace_publication.c
 *
 * PURPOSE:
 *   Verify polyglot workspace admission, borrowed-input initialization and selection failure preservation.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/


#include "umicom/studio/polyglot_workspace.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(c) do { if (!(c)) { (void)fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#c); goto cleanup; } } while (0)
int main(void)
{
    int result = 1;
    UmiStudioPolyglotWorkspace *workspace = malloc(sizeof(*workspace));
    UmiStudioPolyglotWorkspace *saved = malloc(sizeof(*saved));
    UmiCompilerProjectManifest *manifest = malloc(sizeof(*manifest));
    UmiCompilerUnitManifest unit = {0};
    char overlong[UMI_COMPILER_ID_CAPACITY + 1U];
    CHECK(workspace != NULL && saved != NULL && manifest != NULL);
    CHECK(umi_compiler_manifest_init(manifest,"project","Project","x86_64-unknown-linux-gnu") == UMI_STATUS_OK);
    /* Empty drafts retain the previous workspace contract without pretending
     * to satisfy a runnable project's semantic requirements. */
    CHECK(umi_studio_polyglot_workspace_init(workspace,"workspace",manifest) == UMI_STATUS_OK);
    CHECK(umi_studio_polyglot_workspace_language_count(workspace) == 0U);
    (void)strcpy(unit.unit_id,"main");
    (void)strcpy(unit.source,"main.c");
    (void)strcpy(unit.output,"main.o");
    unit.language = UMI_COMPILER_LANGUAGE_C;
    CHECK(umi_compiler_manifest_add_unit(manifest,&unit) == UMI_STATUS_OK);
    (void)strcpy(unit.unit_id,"support");
    (void)strcpy(unit.source,"support.rs");
    unit.language = UMI_COMPILER_LANGUAGE_RUST;
    CHECK(umi_compiler_manifest_add_unit(manifest,&unit) == UMI_STATUS_OK);
    CHECK(umi_studio_polyglot_workspace_init(workspace,"workspace",manifest) == UMI_STATUS_OK);
    CHECK(umi_studio_polyglot_workspace_language_count(workspace) == 2U);
    CHECK(strcmp(workspace->active_unit,"main") == 0);
    CHECK(umi_studio_polyglot_workspace_select_unit(workspace,workspace->manifest.units[1].unit_id) == UMI_STATUS_OK);
    CHECK(strcmp(workspace->active_unit,"support") == 0 && workspace->revision == 2U);
    CHECK(umi_studio_polyglot_workspace_select_unit(workspace,workspace->active_unit) == UMI_STATUS_OK);
    CHECK(workspace->revision == 3U);
    CHECK(umi_studio_polyglot_workspace_init(workspace,workspace->workspace_id,&workspace->manifest) == UMI_STATUS_OK);
    CHECK(workspace->manifest.unit_count == 2U && workspace->revision == 1U);
    CHECK(strcmp(workspace->workspace_id,"workspace") == 0 && strcmp(workspace->active_unit,"main") == 0);
    (void)memcpy(saved,workspace,sizeof(*saved));
    (void)memset(overlong,'x',sizeof(overlong)-1U);overlong[sizeof(overlong)-1U] = '\0';
    CHECK(umi_studio_polyglot_workspace_init(workspace,overlong,manifest) == UMI_STATUS_CAPACITY_EXCEEDED);
    CHECK(memcmp(saved,workspace,sizeof(*saved)) == 0);
    CHECK(umi_studio_polyglot_workspace_select_unit(workspace,overlong) == UMI_STATUS_CAPACITY_EXCEEDED);
    CHECK(memcmp(saved,workspace,sizeof(*saved)) == 0);
    CHECK(umi_studio_polyglot_workspace_select_unit(workspace,"absent") == UMI_STATUS_NOT_FOUND);
    CHECK(memcmp(saved,workspace,sizeof(*saved)) == 0);
    manifest->unit_count = UMI_COMPILER_MAX_UNITS + 1U;
    CHECK(umi_studio_polyglot_workspace_init(workspace,"new",manifest) == UMI_STATUS_INVALID_STATE);
    CHECK(memcmp(saved,workspace,sizeof(*saved)) == 0);
    manifest->unit_count = 2U;
    manifest->units[1].language = (UmiCompilerLanguage)99;
    CHECK(umi_studio_polyglot_workspace_init(workspace,"new",manifest) == UMI_STATUS_INVALID_STATE);
    CHECK(memcmp(saved,workspace,sizeof(*saved)) == 0);
    workspace->revision = UINT64_MAX;
    (void)memcpy(saved,workspace,sizeof(*saved));
    CHECK(umi_studio_polyglot_workspace_select_unit(workspace,"support") == UMI_STATUS_CAPACITY_EXCEEDED);
    CHECK(memcmp(saved,workspace,sizeof(*saved)) == 0);
    workspace->language_mask = 0U;
    (void)memcpy(saved,workspace,sizeof(*saved));
    CHECK(umi_studio_polyglot_workspace_select_unit(workspace,"support") == UMI_STATUS_INVALID_STATE);
    CHECK(umi_studio_polyglot_workspace_language_count(workspace) == 0U);
    CHECK(memcmp(saved,workspace,sizeof(*saved)) == 0);
    CHECK(umi_studio_polyglot_workspace_init(NULL,"id",manifest) == UMI_STATUS_INVALID_ARGUMENT);
    result = 0;
cleanup:
    free(manifest);
    free(saved);
    free(workspace);
    return result;
}
