/* Umicom Studio IDE production control-plane test | Sammy Hegab | Umicom Foundation | MIT */
#include <assert.h>
#include "umicom/studio/production_profile.h"
int main(void) {
    UmiStudioProductionProfile profile;
    assert(umi_studio_production_profile_build(&profile) == UMI_STATUS_OK);
    assert(profile.binding.experience != NULL);
    assert(profile.binding.adoption_snapshot.acceptance_ready);
    return 0;
}

