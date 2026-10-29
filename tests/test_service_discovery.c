/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_service_discovery.c
 *
 * PURPOSE:
 *   Verify Studio exposes stable copied Framework service metadata.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Setup and checks must run in release configurations too. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "umicom/studio/state.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static UmiStudioBootstrap *Create(void)
{
    UmiStudioBootstrap *bootstrap = NULL;
    UmiStudioServicesOptions options = {0};
    assert(umi_studio_bootstrap_create_with_options(&options, &bootstrap) == UMI_STATUS_OK);
    return bootstrap;
}
static void CheckCreated(void)
{
    UmiStudioBootstrap *bootstrap = Create();
    size_t total = 99U;
    assert(UmiStudioStateCopyServices(bootstrap, NULL, 0U, &total) == UMI_STATUS_OK);
    assert(total == umi_service_registry_count(umi_studio_bootstrap_service_registry(bootstrap)));
    assert(UmiStudioBootstrapRelease(&bootstrap) == UMI_STATUS_OK);
    assert(bootstrap == NULL);
}
static void CheckInvalid(void)
{
    size_t total = 99U;
    assert(UmiStudioStateCopyServices(NULL, NULL, 0U, &total) == UMI_STATUS_INVALID_ARGUMENT);
    assert(total == 0U);
    UmiStudioBootstrap *bootstrap = Create();
    total = 99U;
    assert(UmiStudioStateCopyServices(bootstrap, NULL, 1U, &total) == UMI_STATUS_INVALID_ARGUMENT);
    assert(total == 0U);
    assert(UmiStudioStateCopyServices(bootstrap, NULL, 0U, NULL) == UMI_STATUS_INVALID_ARGUMENT);
    assert(UmiStudioBootstrapRelease(&bootstrap) == UMI_STATUS_OK);
}
static void CheckStarted(const char *scenario)
{
    UmiStudioBootstrap *bootstrap = Create();
    assert(umi_studio_bootstrap_start(bootstrap) == UMI_STATUS_OK);
    UmiServiceSnapshot *items = calloc(UMI_SERVICE_REGISTRY_MAX, sizeof(*items));
    assert(items != NULL);
    size_t total = 0U;
    size_t capacity = strcmp(scenario, "short-list") == 0 ? 1U : UMI_SERVICE_REGISTRY_MAX;
    UmiStatus expected = capacity == 1U ? UMI_STATUS_CAPACITY_EXCEEDED : UMI_STATUS_OK;
    assert(UmiStudioStateCopyServices(bootstrap, items, capacity, &total) == expected);
    assert(total > 1U && total <= UMI_SERVICE_REGISTRY_MAX);
    assert(items[0].service_id[0] != '\0' && items[0].provider_module_id[0] != '\0');
    UmiServiceSnapshot saved;
    (void)memcpy(&saved, &items[0], sizeof(saved));
    if (strcmp(scenario, "started-report") == 0) {
        UmiStudioStateReport report;
        assert(umi_studio_state_capture(bootstrap, &report) == UMI_STATUS_OK);
        assert(report.service_count == total);
        for (size_t index = 0U; index < total; ++index) {
            assert(items[index].service_id[0] != '\0');
            assert(items[index].provider_module_id[0] != '\0');
            for (size_t previous = 0U; previous < index; ++previous) {
                assert(strcmp(items[index].service_id, items[previous].service_id) != 0 ||
                    strcmp(items[index].provider_module_id, items[previous].provider_module_id) != 0);
            }
        }
    }
    /* Checked release stops modules and workers. Copies retain no owner pointers. */
    assert(UmiStudioBootstrapRelease(&bootstrap) == UMI_STATUS_OK);
    assert(bootstrap == NULL);
    assert(memcmp(&saved, &items[0], sizeof(saved)) == 0);
    assert(strlen(items[0].service_id) < UMI_SERVICE_ID_CAPACITY);
    assert(strlen(items[0].provider_module_id) < UMI_SERVICE_PROVIDER_CAPACITY);
    free(items);
}
int main(int argc, char **argv)
{
    assert(argc == 2);
    if (strcmp(argv[1], "created") == 0) CheckCreated();
    else if (strcmp(argv[1], "invalid-arguments") == 0) CheckInvalid();
    else if (strcmp(argv[1], "started-report") == 0 ||
             strcmp(argv[1], "short-list") == 0 ||
             strcmp(argv[1], "after-release") == 0) CheckStarted(argv[1]);
    else assert(!"Unknown Studio service discovery case");
    return 0;
}
