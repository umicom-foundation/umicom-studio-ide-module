/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_fabric.c
 *
 * PURPOSE:
 *   Verify Studio Integration Fabric default schema and topic registration.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/fabric.h"

#include <assert.h>

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioServices *services;
    assert(umi_studio_bootstrap_create(&bootstrap) == UMI_STATUS_OK);
    services = umi_studio_bootstrap_services(bootstrap);
    assert(umi_studio_fabric_validate(services) == UMI_STATUS_OK);
    assert(umi_schema_registry_count(umi_studio_services_schema_registry(services)) == 4U);
    assert(umi_topic_registry_count(umi_studio_services_topics(services)) == 4U);
    umi_studio_bootstrap_destroy(bootstrap);
    return 0;
}
