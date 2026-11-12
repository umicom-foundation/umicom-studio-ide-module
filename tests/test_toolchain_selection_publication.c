/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_toolchain_selection_publication.c
 *
 * PURPOSE:
 *   Verify refused manual toolchain selection preserves Studio and catalogue state.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/


#include "umicom/studio/toolchain_centre.h"
#include <stdio.h>
#include <string.h>
#define REQUIRE(c) do { if (!(c)) { fprintf(stderr,"selection %s:%d\n",__FILE__,__LINE__); return 1; } } while (0)
static UmiToolchainProfile profile, previous_profile;
static UmiEnvironmentPlan previous_environment;
int main(void)
{
    UmiStudioToolchainCentre *centre = NULL;
    UmiToolchainCatalogueSnapshot before, after;
    REQUIRE(umi_studio_toolchain_centre_create(&centre) == UMI_STATUS_OK);
    UmiToolchainCatalogue *catalogue = umi_studio_toolchain_centre_catalogue(centre);
    umi_toolchain_profile_init(&profile);
    (void)strcpy(profile.profile_id,"good");
    profile.selected_c_compiler = UMI_TOOL_GCC;
    profile.tools[UMI_TOOL_GCC].state = UMI_TOOL_VALIDATED;
    (void)strcpy(profile.tools[UMI_TOOL_GCC].path,"gcc-fixture");
    REQUIRE(umi_toolchain_catalogue_upsert_profile(catalogue,&profile) == UMI_STATUS_OK);
    (void)strcpy(profile.profile_id,"bad");
    (void)strcpy(profile.tools[UMI_TOOL_GCC].path,"gcc\nrefused");
    REQUIRE(umi_toolchain_catalogue_upsert_profile(catalogue,&profile) == UMI_STATUS_OK);
    REQUIRE(umi_studio_toolchain_centre_select_profile(centre,"good") == UMI_STATUS_OK);
    const UmiToolchainProfile *selected = umi_studio_toolchain_centre_selected_profile(centre);
    const UmiEnvironmentPlan *environment = umi_studio_toolchain_centre_environment(centre);
    REQUIRE(selected != NULL && environment != NULL);
    (void)memcpy(&previous_profile,selected,sizeof(previous_profile));
    (void)memcpy(&previous_environment,environment,sizeof(previous_environment));
    REQUIRE(umi_toolchain_catalogue_snapshot(catalogue,&before) == UMI_STATUS_OK);
    REQUIRE(umi_studio_toolchain_centre_select_profile(centre,"bad") == UMI_STATUS_INVALID_ARGUMENT);
    REQUIRE(memcmp(&previous_profile,umi_studio_toolchain_centre_selected_profile(centre),sizeof(previous_profile)) == 0);
    REQUIRE(memcmp(&previous_environment,umi_studio_toolchain_centre_environment(centre),sizeof(previous_environment)) == 0);
    REQUIRE(umi_toolchain_catalogue_snapshot(catalogue,&after) == UMI_STATUS_OK);
    REQUIRE(before.revision == after.revision && strcmp(after.selected_profile_id,"good") == 0);
    REQUIRE(umi_studio_toolchain_centre_select_profile(centre,"missing") == UMI_STATUS_NOT_FOUND);
    REQUIRE(strcmp(umi_environment_plan_find(environment,"CC"),"gcc-fixture") == 0);
    for (size_t i = 0U; i < environment->count; ++i) {
        REQUIRE(environment->process_entries[i].name == environment->entries[i].name);
        REQUIRE(environment->process_entries[i].value == environment->entries[i].value);
    }
    umi_studio_toolchain_centre_destroy(centre);
    return 0;
}
