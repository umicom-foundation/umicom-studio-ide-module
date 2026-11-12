/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_umicc_polyglot.c
 *
 * PURPOSE:
 *   Verify the test umicc polyglot behavior for
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
#include "umicom/studio/umicc_integration.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* The original test used side-effecting assertions and a large automatic manifest. Keep it for comparison; the active test runs checks with NDEBUG and owns large values explicitly. The previous implementation is retained for review. */
#if 0
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void) { UmiStudioUmiccIntegration umicc; UmiCompilerProjectManifest manifest; UmiCompilerUnitManifest unit = {0}; UmiStudioPolyglotWorkspace *workspace = calloc(1U,sizeof(*workspace)); char reason[256]; assert(workspace != NULL); assert(umi_studio_umicc_integration_configure(&umicc,"umicc","0.1","x86_64-unknown-linux-gnu",true) == UMI_STATUS_OK); assert(umi_studio_umicc_integration_validate(&umicc,reason,sizeof(reason)) == UMI_STATUS_OK); assert(umi_compiler_manifest_init(&manifest,"p","Poly","x86_64-unknown-linux-gnu") == UMI_STATUS_OK); (void)strcpy(unit.unit_id,"main"); (void)strcpy(unit.source,"main.uai"); (void)strcpy(unit.output,"main.o"); unit.language = UMI_COMPILER_LANGUAGE_UAI; assert(umi_compiler_manifest_add_unit(&manifest,&unit) == UMI_STATUS_OK); assert(umi_studio_polyglot_workspace_init(workspace,"w",&manifest) == UMI_STATUS_OK); assert(umi_studio_polyglot_workspace_language_count(workspace) == 1U); free(workspace); return 0; }

#endif
/* Execute setup checks in every build mode. Heap-own the large inline manifest
 * and workspace, and release them even when a setup assertion fails. */
#define CHECK(c) do { if (!(c)) { (void)fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#c); goto cleanup; } } while (0)
int main(void)
{
    int result = 1;
    UmiStudioUmiccIntegration umicc;
    UmiCompilerProjectManifest *manifest = malloc(sizeof(*manifest));
    UmiStudioPolyglotWorkspace *workspace = calloc(1U,sizeof(*workspace));
    UmiCompilerUnitManifest unit = {0};
    char reason[256];
    CHECK(manifest != NULL && workspace != NULL);
    CHECK(umi_studio_umicc_integration_configure(&umicc,"umicc","0.1","x86_64-unknown-linux-gnu",true) == UMI_STATUS_OK);
    CHECK(umi_studio_umicc_integration_validate(&umicc,reason,sizeof(reason)) == UMI_STATUS_OK);
    CHECK(umi_compiler_manifest_init(manifest,"p","Poly","x86_64-unknown-linux-gnu") == UMI_STATUS_OK);
    (void)strcpy(unit.unit_id,"main");
    (void)strcpy(unit.source,"main.uai");
    (void)strcpy(unit.output,"main.o");
    unit.language = UMI_COMPILER_LANGUAGE_UAI;
    CHECK(umi_compiler_manifest_add_unit(manifest,&unit) == UMI_STATUS_OK);
    CHECK(umi_studio_polyglot_workspace_init(workspace,"w",manifest) == UMI_STATUS_OK);
    CHECK(umi_studio_polyglot_workspace_language_count(workspace) == 1U);
    result = 0;
cleanup:
    free(workspace);
    free(manifest);
    return result;
}
