/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_compiler_centre.c
 *
 * PURPOSE:
 *   Verify the test compiler centre behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/compiler_centre.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
/* Retain the original acceptance checks while executing them independently of NDEBUG and extending replacement, directory and snapshot coverage. The previous implementation is retained for review. */
#if 0
int main(void) { UmiStudioCompilerCentre *centre = NULL; UmiStudioCompilerCentreSnapshot snapshot; assert(umi_studio_compiler_centre_create(NULL,&centre) == UMI_STATUS_OK); assert(umi_studio_compiler_centre_register_defaults(centre,"gcc","clang","rustc","zig","",NULL) == UMI_STATUS_OK); assert(umi_studio_compiler_centre_activate_view(centre,"compiler.build-graph") == UMI_STATUS_OK); assert(umi_studio_compiler_centre_snapshot(centre,&snapshot) == UMI_STATUS_OK); assert(snapshot.compiler.providers >= 4U); assert(snapshot.compiler.profiles == 1U); umi_studio_compiler_centre_destroy(centre); return 0; }
#endif
#define REQUIRE(c) do { if (!(c)) { fprintf(stderr,"compiler centre %s:%d\n",__FILE__,__LINE__); return 1; } } while (0)
int main(void)
{
    UmiStudioCompilerCentre *centre = NULL;
    UmiStudioCompilerCentreSnapshot snapshot, previous;
    static UmiCompilerProjectManifest manifest;
    static UmiCompilerPlan plan;
    UmiCompilerUnitManifest unit = {0};
    REQUIRE(umi_studio_compiler_centre_create(NULL,&centre) == UMI_STATUS_OK);
    REQUIRE(umi_studio_compiler_centre_register_defaults(centre,"gcc","clang","rustc","zig","",NULL) == UMI_STATUS_OK);
    REQUIRE(umi_studio_compiler_centre_activate_view(centre,"compiler.build-graph") == UMI_STATUS_OK);
    REQUIRE(umi_studio_compiler_centre_snapshot(centre,&snapshot) == UMI_STATUS_OK);
    REQUIRE(snapshot.compiler.providers >= 4U && snapshot.compiler.profiles == 1U);
    UmiCompilerCentre *engine = umi_studio_compiler_centre_engine(centre);
    REQUIRE(umi_studio_compiler_centre_register_defaults(centre,"new-gcc","","","","",NULL) == UMI_STATUS_OK);
    REQUIRE(umi_studio_compiler_centre_engine(centre) == engine);
    REQUIRE(strcmp(umi_compiler_registry_find(&engine->providers,"gcc")->descriptor.executable,"new-gcc") == 0);
    REQUIRE(engine->providers.count == 4U && engine->profile_count == 1U);
    REQUIRE(umi_compiler_manifest_init(&manifest,"project","Project","x86_64-unknown-linux-gnu") == UMI_STATUS_OK);
    (void)strcpy(unit.unit_id,"main"); (void)strcpy(unit.source,"source.c"); (void)strcpy(unit.output,"object.o");
    unit.language = UMI_COMPILER_LANGUAGE_C;
    REQUIRE(umi_compiler_manifest_add_unit(&manifest,&unit) == UMI_STATUS_OK);
    REQUIRE(umi_studio_compiler_centre_open_project(centre,&manifest) == UMI_STATUS_OK);
    REQUIRE(umi_studio_compiler_centre_plan_unit_in_directory(centre,"project","main","explicit-directory",&plan) == UMI_STATUS_OK);
    REQUIRE(strcmp(plan.working_directory,"explicit-directory") == 0);
    REQUIRE(strcmp(engine->compilation_database.items[0].directory,"explicit-directory") == 0);
    REQUIRE(umi_studio_compiler_centre_plan_unit(centre,"project","main",&plan) == UMI_STATUS_OK);
    REQUIRE(strcmp(plan.working_directory,".") == 0);
    REQUIRE(umi_studio_compiler_centre_snapshot(centre,&snapshot) == UMI_STATUS_OK);
    (void)memcpy(&previous,&snapshot,sizeof(previous));
    engine->profile_count = UMI_COMPILER_MAX_PROFILES+1U;
    REQUIRE(umi_studio_compiler_centre_snapshot(centre,&snapshot) == UMI_STATUS_INVALID_STATE);
    REQUIRE(memcmp(&snapshot,&previous,sizeof(snapshot)) == 0);
    engine->profile_count = 1U;
    engine->revision = UINT64_MAX;
    REQUIRE(umi_studio_compiler_centre_snapshot(centre,&snapshot) == UMI_STATUS_OK);
    REQUIRE(snapshot.revision == UINT64_MAX && snapshot.compiler.revision == UINT64_MAX);
    REQUIRE(umi_studio_compiler_centre_snapshot(centre,(UmiStudioCompilerCentreSnapshot *)(void *)engine) == UMI_STATUS_INVALID_ARGUMENT);
    umi_studio_compiler_centre_destroy(centre);
    return 0;
}

