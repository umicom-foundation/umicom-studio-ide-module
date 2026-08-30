/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_production_panels.c
 *
 * PURPOSE:
 *   Verify the test production panels behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE production control-plane test | Sammy Hegab | Umicom Foundation | MIT */
#include <assert.h>
#include <stdlib.h>
#include "umicom/studio/production_panels.h"
static int available(const char *id, void *context) {
    (void)context; return id != NULL && id[0] != '\0';
}
int main(void) {
    UmiApplicationProductionRuntime *runtime = calloc(1U, sizeof(*runtime));
    UmiStudioProductionPanelCatalogue panels;
    assert(runtime != NULL);
    assert(umi_application_production_runtime_init(umi_studio_productisation_contribution(), available, NULL, runtime) == UMI_STATUS_OK);
    assert(umi_studio_production_panels_build(runtime, &panels) == UMI_STATUS_OK);
    assert(panels.count == runtime->panels.count);
    free(runtime); return 0;
}

