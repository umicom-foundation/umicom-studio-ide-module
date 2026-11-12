/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_compiler_synchronization.c
 *
 * PURPOSE:
 *   Verify Studio follows selected toolchain replacement through shared Framework publication.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/


#include "umicom/studio/compiler_centre.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define REQUIRE(c) do { if (!(c)) { fprintf(stderr,"compiler sync %s:%d\n",__FILE__,__LINE__); return 1; } } while (0)
static UmiToolchainProfile profile;
static int Select(UmiStudioToolchainCentre *tools,const char *driver)
{
    umi_toolchain_profile_init(&profile);
    (void)strcpy(profile.profile_id,"fixture-gcc");
    (void)strcpy(profile.display_name,"Fixture GCC");
    (void)strcpy(profile.root,"/usr");
    /* Selection coverage should not depend on the host's inherited PATH length.
     * Retain the earlier bin-directory fixture for review; empty keeps this case
     * focused on the explicitly supplied compiler and registry metadata. */
#if 0
    (void)strcpy(profile.bin_directory,"/usr/bin");
#endif
    (void)strcpy(profile.prefix_directory,"/usr");
    (void)strcpy(profile.target_triple,"x86_64-unknown-linux-gnu");
    profile.family = UMI_TOOLCHAIN_POSIX_GCC;
    profile.selected_c_compiler = UMI_TOOL_GCC;
    profile.selected_cpp_compiler = UMI_TOOL_GXX;
    profile.c23_capable = 1;
    profile.complete = 1;
    const UmiToolKind kinds[] = {UMI_TOOL_GCC,UMI_TOOL_GXX,UMI_TOOL_CMAKE,UMI_TOOL_CTEST,
                                UMI_TOOL_NINJA,UMI_TOOL_PKG_CONFIG};
    for (size_t i = 0U; i < sizeof(kinds)/sizeof(kinds[0]); ++i) {
        UmiToolInfo *tool = umi_toolchain_profile_tool_mutable(&profile,kinds[i]);
        REQUIRE(tool != NULL);
        tool->state = UMI_TOOL_VALIDATED;
        (void)strcpy(tool->path,kinds[i] == UMI_TOOL_GCC ? driver : "unexecuted-fixture-tool");
        (void)strcpy(tool->version,kinds[i] == UMI_TOOL_GCC ? "gcc 16.1.0" : "1.0");
    }
    REQUIRE(umi_toolchain_catalogue_upsert_profile(umi_studio_toolchain_centre_catalogue(tools),&profile) == UMI_STATUS_OK);
    REQUIRE(umi_studio_toolchain_centre_select_profile(tools,"fixture-gcc") == UMI_STATUS_OK);
    return 0;
}
int main(void)
{
    UmiStudioToolchainCentre *tools = NULL;
    UmiStudioCompilerCentre *centre = NULL;
    UmiStudioCompilerCentreSnapshot snapshot;
    REQUIRE(umi_studio_toolchain_centre_create(&tools) == UMI_STATUS_OK);
    REQUIRE(umi_studio_compiler_centre_create(tools,&centre) == UMI_STATUS_OK);
    REQUIRE(Select(tools,"first-fixture-gcc") == 0);
    REQUIRE(umi_studio_compiler_centre_sync_toolchain(centre) == UMI_STATUS_OK);
    UmiCompilerCentre *engine = umi_studio_compiler_centre_engine(centre);
    REQUIRE(engine != NULL);
    REQUIRE(Select(tools,"second-fixture-gcc") == 0);
    REQUIRE(umi_studio_compiler_centre_sync_toolchain(centre) == UMI_STATUS_OK);
    REQUIRE(umi_studio_compiler_centre_engine(centre) == engine);
    REQUIRE(engine->providers.count == 1U && engine->profile_count == 1U);
    REQUIRE(strcmp(engine->providers.items[0].descriptor.executable,"second-fixture-gcc") == 0);
    REQUIRE(strcmp(umi_compiler_centre_active_profile(engine)->linker.driver,"second-fixture-gcc") == 0);
    REQUIRE(umi_studio_compiler_centre_snapshot(centre,&snapshot) == UMI_STATUS_OK);
    REQUIRE(snapshot.toolchain_synchronised && strcmp(snapshot.compiler.active_profile_id,"toolchain-selected") == 0);
    umi_studio_compiler_centre_destroy(centre);
    umi_studio_toolchain_centre_destroy(tools);
    return 0;
}
