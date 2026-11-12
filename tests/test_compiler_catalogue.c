/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_compiler_catalogue.c
 *
 * PURPOSE:
 *   Verify the test compiler catalogue behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/compiler_catalogue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* The original assertion-only success case is retained for comparison. Active checks cover malformed later rows, source/destination overlap, borrowed selection, failed publication and NDEBUG builds. The previous implementation is retained for review. */
#if 0
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void) { UmiCompilerRegistry registry; UmiCompilerProvider gcc = umi_compiler_gcc_provider("gcc",""); UmiStudioCompilerCatalogue catalogue; umi_compiler_registry_init(&registry); assert(umi_compiler_registry_register(&registry,&gcc) == UMI_STATUS_OK); assert(umi_studio_compiler_catalogue_project(&registry,"gcc",&catalogue) == UMI_STATUS_OK); assert(catalogue.count == 1U); assert(catalogue.rows[0].selected); return 0; }

#endif
/* Exercise complete source/view admission without side-effecting assert calls,
 * keeping these checks active in configurations that define NDEBUG. */
#define CHECK(c) do { if (!(c)) { (void)fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#c); goto cleanup; } } while (0)
int main(void)
{
    int result = 1;
    UmiCompilerRegistry *registry = malloc(sizeof(*registry)), *saved_registry = malloc(sizeof(*saved_registry));
    UmiStudioCompilerCatalogue *catalogue = malloc(sizeof(*catalogue)), *saved = malloc(sizeof(*saved));
    UmiCompilerProvider gcc = umi_compiler_gcc_provider("gcc",""), clang = umi_compiler_clang_provider("clang","");
    char long_id[UMI_COMPILER_ID_CAPACITY+1U];
    CHECK(registry != NULL && saved_registry != NULL && catalogue != NULL && saved != NULL);
    umi_compiler_registry_init(registry);
    CHECK(umi_compiler_registry_register(registry,&gcc) == UMI_STATUS_OK);
    CHECK(umi_compiler_registry_register(registry,&clang) == UMI_STATUS_OK);
    CHECK(umi_studio_compiler_catalogue_project(registry,"gcc",catalogue) == UMI_STATUS_OK);
    CHECK(catalogue->count == 2U && catalogue->rows[0].selected && !catalogue->rows[1].selected);
    CHECK(umi_studio_compiler_catalogue_valid(catalogue));
    CHECK(umi_studio_compiler_catalogue_find(catalogue,"clang") == &catalogue->rows[1]);
    CHECK(umi_studio_compiler_catalogue_project(registry,catalogue->rows[1].provider_id,catalogue) == UMI_STATUS_OK);
    CHECK(!catalogue->rows[0].selected && catalogue->rows[1].selected);
    (void)memcpy(saved,catalogue,sizeof(*saved));
    registry->count = UMI_COMPILER_MAX_PROVIDERS+1U;
    CHECK(umi_studio_compiler_catalogue_project(registry,"gcc",catalogue) == UMI_STATUS_INVALID_STATE);
    CHECK(memcmp(saved,catalogue,sizeof(*saved)) == 0);
    registry->count = 2U;
    (void)memset(registry->items[1].descriptor.executable,'x',sizeof(registry->items[1].descriptor.executable));
    CHECK(umi_studio_compiler_catalogue_project(registry,"gcc",catalogue) == UMI_STATUS_INVALID_STATE);
    CHECK(memcmp(saved,catalogue,sizeof(*saved)) == 0);
    registry->items[1] = clang;
    (void)memset(long_id,'x',sizeof(long_id)-1U);long_id[sizeof(long_id)-1U] = '\0';
    CHECK(umi_studio_compiler_catalogue_project(registry,long_id,catalogue) == UMI_STATUS_CAPACITY_EXCEEDED);
    CHECK(memcmp(saved,catalogue,sizeof(*saved)) == 0);
    CHECK(umi_studio_compiler_catalogue_find(catalogue,long_id) == NULL);
    (void)memcpy(saved_registry,registry,sizeof(*saved_registry));
    CHECK(umi_studio_compiler_catalogue_project(registry,NULL,(UmiStudioCompilerCatalogue *)(void *)registry) == UMI_STATUS_INVALID_ARGUMENT);
    CHECK(memcmp(saved_registry,registry,sizeof(*saved_registry)) == 0);
    catalogue->count = UMI_STUDIO_COMPILER_CATALOGUE_MAX+1U;
    CHECK(!umi_studio_compiler_catalogue_valid(catalogue));
    CHECK(umi_studio_compiler_catalogue_find(catalogue,"gcc") == NULL);
    (void)memcpy(catalogue,saved,sizeof(*saved));
    (void)memset(catalogue->rows[1].name,'x',sizeof(catalogue->rows[1].name));
    CHECK(umi_studio_compiler_catalogue_find(catalogue,"gcc") == NULL);
    CHECK(umi_studio_compiler_catalogue_project(registry,NULL,catalogue) == UMI_STATUS_OK);
    CHECK(!catalogue->rows[0].selected && !catalogue->rows[1].selected);
    registry->revision = UINT64_MAX;
    CHECK(umi_studio_compiler_catalogue_project(registry,"gcc",catalogue) == UMI_STATUS_OK);
    CHECK(catalogue->revision == UINT64_MAX);
    result = 0;
cleanup:
    free(saved);free(catalogue);free(saved_registry);free(registry);
    return result;
}
