/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_commands.c
 *
 * PURPOSE:
 *   Verify that canonical Studio commands are registered once and execute
 *   through the Framework command registry without a graphical frontend.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/commands.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/platform_contract.h"
#include "umicom/studio/trading.h"
#include "umicom/studio/workbench_commands.h"
#include "umicom/studio/tests.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "umicom/platform/clock.h"
#include "umicom/platform/path.h"
#include "umicom/platform/filesystem.h"

/* Emit command identities only, never user arguments or private model data.
 * Flushing before dispatch localises a blocking command without changing its
 * assertions or treating diagnostic instrumentation as a functional repair. */
static void ReportCommandPhase(const char *phase)
{
    /* Preserve the identity-only diagnostic. Monotonic durations now reveal
     * which preceding command is slow without logging any model/user data. */
#if 0
    fprintf(stderr, "Studio command phase: %s\n", phase);
#endif
    static uint64_t previous = 0U;
    UmiClock clock = umi_clock_system();
    const uint64_t now = clock.monotonic_nanoseconds(&clock);
    const uint64_t elapsed = previous != 0U && now >= previous ? now - previous : 0U;
    fprintf(stderr, "Studio command phase: %s; previous phase duration: %" PRIu64 " ms\n",
        phase, elapsed / UINT64_C(1000000));
    previous = now;
    fflush(stderr);
}


/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioBootstrap *bootstrap = NULL;
    UmiCommandRegistry *registry;
    UmiCommandSnapshot snapshot;
    UmiStudioPlatformContractSnapshot contract;
    UmiTradingWorkspaceSnapshot trading_snapshot;
    char message[512];

    /* CTest supplies an owned fixture. Refuse accidental direct execution from
     * a developer workspace before the established relative-state operations. */
    char fixture_directory[UMI_PATH_CAPACITY];
    const char *fixture_root = getenv("UMICOM_STUDIO_COMMAND_FIXTURE_ROOT");
    if (fixture_root == NULL ||
        umi_fs_current_directory(fixture_directory, sizeof fixture_directory) != UMI_STATUS_OK ||
        !umi_path_equal(fixture_directory, fixture_root) ||
        !umi_fs_is_file("applications/studio/src/app/ai_platform.c")) {
        fputs("Run studio.commands through CTest in its configured fixture directory.\n", stderr);
        return 2;
    }

    (void)umi_fs_remove_tree(".umicom");
    ReportCommandPhase("bootstrap-create");
    assert(umi_studio_bootstrap_create(&bootstrap) == UMI_STATUS_OK);
    ReportCommandPhase("bootstrap-start");
    assert(umi_studio_bootstrap_start(bootstrap) == UMI_STATUS_OK);

    registry = umi_studio_bootstrap_command_registry(bootstrap);
    /* The explicit restart command is registered even before a session exists.
     * It must report missing execution state rather than creating a process. */
    assert(umi_command_registry_snapshot(registry, UMI_STUDIO_COMMAND_DEBUG_RESTART, &snapshot) == UMI_STATUS_OK);
    ReportCommandPhase("UMI_STUDIO_COMMAND_DEBUG_RESTART");
    assert(umi_command_registry_execute(registry, UMI_STUDIO_COMMAND_DEBUG_RESTART, NULL,
        message, sizeof message) == UMI_STATUS_INVALID_STATE);
    assert(registry != NULL);
    /*
     * The old assertion is retained for builds that intentionally enforce an
     * exclusive Studio-only command registry.  Normal Studio composition also
     * receives reusable designer commands, so the default contract validates
     * required identities while allowing additive registry contributions.
     */
#if defined(UMI_STUDIO_ENFORCE_EXCLUSIVE_COMMAND_REGISTRY)
    assert(umi_command_registry_count(registry) ==
           UMI_STUDIO_CORE_COMMAND_COUNT + UMI_STUDIO_WORKBENCH_COMMAND_COUNT);
#endif
    assert(umi_studio_platform_contract_capture_bootstrap(
        bootstrap, &contract) == UMI_STATUS_OK);
    assert(umi_studio_platform_contract_validate(&contract) == UMI_STATUS_OK);
    assert(umi_command_registry_count(registry) == contract.runtime.command_count);
    assert(umi_command_registry_snapshot(
        registry,
        UMI_STUDIO_COMMAND_SESSION_SAVE,
        &snapshot
    ) == UMI_STATUS_OK);
    assert(strcmp(snapshot.category, "Session") == 0);

    ReportCommandPhase("UMI_STUDIO_COMMAND_TASKS_WAIT_IDLE");
    assert(umi_command_registry_execute(
        registry,
        UMI_STUDIO_COMMAND_TASKS_WAIT_IDLE,
        "1000",
        message,
        sizeof(message)
    ) == UMI_STATUS_OK);
    assert(strstr(message, "idle") != NULL);

    ReportCommandPhase("UMI_STUDIO_COMMAND_WORKSPACE_REFRESH");
    assert(umi_command_registry_execute(
        registry,
        UMI_STUDIO_COMMAND_WORKSPACE_REFRESH,
        "",
        message,
        sizeof(message)
    ) == UMI_STATUS_OK);
    assert(strstr(message, "refreshed") != NULL);

    ReportCommandPhase("UMI_STUDIO_COMMAND_WATCHER_SCAN");
    assert(umi_command_registry_execute(
        registry,
        UMI_STUDIO_COMMAND_WATCHER_SCAN,
        "",
        message,
        sizeof(message)
    ) == UMI_STATUS_OK);
    assert(strstr(message, "scanned") != NULL);

    ReportCommandPhase("UMI_STUDIO_COMMAND_DATA_INTEGRITY");
    assert(umi_command_registry_execute(
        registry,
        UMI_STUDIO_COMMAND_DATA_INTEGRITY,
        "",
        message,
        sizeof(message)
    ) == UMI_STATUS_OK);
    assert(strstr(message, "integrity OK") != NULL);

    ReportCommandPhase("UMI_STUDIO_COMMAND_MESSAGES_FLUSH");
    assert(umi_command_registry_execute(
        registry,
        UMI_STUDIO_COMMAND_MESSAGES_FLUSH,
        "",
        message,
        sizeof(message)
    ) == UMI_STATUS_OK);
    assert(strstr(message, "Outbox delivered") != NULL);

    ReportCommandPhase("UMI_STUDIO_COMMAND_MESSAGES_REPLAY");
    assert(umi_command_registry_execute(
        registry,
        UMI_STUDIO_COMMAND_MESSAGES_REPLAY,
        "",
        message,
        sizeof(message)
    ) == UMI_STATUS_OK);
    assert(strstr(message, "Replayed") != NULL);


    ReportCommandPhase("UMI_STUDIO_COMMAND_NOTIFICATION_INFO");
    assert(umi_command_registry_execute(
        registry,
        UMI_STUDIO_COMMAND_NOTIFICATION_INFO,
        "Command test notification",
        message,
        sizeof(message)
    ) == UMI_STATUS_OK);
    assert(strstr(message, "published") != NULL);

    ReportCommandPhase("UMI_STUDIO_COMMAND_DEVELOPER_REPORT");
    assert(umi_command_registry_execute(
        registry,
        UMI_STUDIO_COMMAND_DEVELOPER_REPORT,
        "",
        message,
        sizeof(message)
    ) == UMI_STATUS_OK);
    assert(strstr(message, "Build profile:") != NULL);

    /*
     * Filtering, stable selection and refresh remain entirely inside the
     * Framework coordinator, so this exercises the professional workspace
     * commands without launching a compiler process during the unit test.
     */
    assert(umi_command_registry_snapshot(
               registry, UMI_STUDIO_COMMAND_BUILD_FILTER, &snapshot) ==
           UMI_STATUS_OK);
    assert(strcmp(snapshot.category, "Build") == 0);
    ReportCommandPhase("UMI_STUDIO_COMMAND_BUILD_FILTER");
    assert(umi_command_registry_execute(
               registry, UMI_STUDIO_COMMAND_BUILD_FILTER, "all",
               message, sizeof(message)) == UMI_STATUS_OK);
    assert(strstr(message, "build node") != NULL);
    ReportCommandPhase("UMI_STUDIO_COMMAND_BUILD_SELECT_NODE");
    assert(umi_command_registry_execute(
               registry, UMI_STUDIO_COMMAND_BUILD_SELECT_NODE, "configure",
               message, sizeof(message)) == UMI_STATUS_OK);
    assert(strstr(message, "configure") != NULL);
    ReportCommandPhase("UMI_STUDIO_COMMAND_BUILD_REFRESH");
    assert(umi_command_registry_execute(
               registry, UMI_STUDIO_COMMAND_BUILD_REFRESH, "",
               message, sizeof(message)) == UMI_STATUS_OK);
    assert(strstr(message, "refreshed") != NULL);

    ReportCommandPhase("UMI_STUDIO_COMMAND_AI_EXPLAIN_CODE");
    assert(umi_command_registry_execute(
        registry,
        UMI_STUDIO_COMMAND_AI_EXPLAIN_CODE,
        "applications/studio/src/app/ai_platform.c|Explain ownership.",
        message,
        sizeof(message)
    ) == UMI_STATUS_OK);
    assert(strstr(message, "explanation plan") != NULL);
    assert(strstr(message, "repository file(s)") != NULL);

    ReportCommandPhase("UMI_STUDIO_COMMAND_LANGUAGE_INITIALIZE");
    assert(umi_command_registry_execute(
        registry,
        UMI_STUDIO_COMMAND_LANGUAGE_INITIALIZE,
        "0",
        message,
        sizeof(message)
    ) == UMI_STATUS_OK);
    assert(strstr(message, "LSP initialise request") != NULL);

    ReportCommandPhase("UMI_STUDIO_COMMAND_DEBUG_INITIALIZE");
    assert(umi_command_registry_execute(
        registry,
        UMI_STUDIO_COMMAND_DEBUG_INITIALIZE,
        "umicom.test.debug",
        message,
        sizeof(message)
    ) == UMI_STATUS_OK);
    assert(strstr(message, "DAP initialise request") != NULL);

    ReportCommandPhase("UMI_STUDIO_COMMAND_DEBUG_ADD_BREAKPOINT");
    assert(umi_command_registry_execute(
        registry,
        UMI_STUDIO_COMMAND_DEBUG_ADD_BREAKPOINT,
        "main.c:42",
        message,
        sizeof(message)
    ) == UMI_STATUS_OK);
    ReportCommandPhase("UMI_STUDIO_COMMAND_DEBUG_SET_BREAKPOINT_ENABLED");
    assert(umi_command_registry_execute(
        registry,
        UMI_STUDIO_COMMAND_DEBUG_SET_BREAKPOINT_ENABLED,
        "main.c:42:1=0",
        message,
        sizeof(message)
    ) == UMI_STATUS_OK);
    ReportCommandPhase("UMI_STUDIO_COMMAND_DEBUG_ADD_WATCH");
    assert(umi_command_registry_execute(
        registry,
        UMI_STUDIO_COMMAND_DEBUG_ADD_WATCH,
        "counter + 1",
        message,
        sizeof(message)
    ) == UMI_STATUS_OK);
    assert(strstr(message, "watch-1") != NULL);
    ReportCommandPhase("UMI_STUDIO_COMMAND_DEBUG_REMOVE_WATCH");
    assert(umi_command_registry_execute(
        registry,
        UMI_STUDIO_COMMAND_DEBUG_REMOVE_WATCH,
        "watch-1",
        message,
        sizeof(message)
    ) == UMI_STATUS_OK);
    ReportCommandPhase("UMI_STUDIO_COMMAND_DEBUG_CLEAR_CONSOLE");
    assert(umi_command_registry_execute(
        registry,
        UMI_STUDIO_COMMAND_DEBUG_CLEAR_CONSOLE,
        "",
        message,
        sizeof(message)
    ) == UMI_STATUS_OK);

    assert(umi_command_registry_snapshot(
               registry, UMI_STUDIO_COMMAND_VCS_FILTER, &snapshot) ==
           UMI_STATUS_OK);
    assert(strcmp(snapshot.category, "Source Control") == 0);
    ReportCommandPhase("UMI_STUDIO_COMMAND_VCS_FILTER");
    assert(umi_command_registry_execute(
               registry, UMI_STUDIO_COMMAND_VCS_FILTER, "conflicts",
               message, sizeof(message)) == UMI_STATUS_OK);
    assert(strstr(message, "conflicts") != NULL);
    ReportCommandPhase("UMI_STUDIO_COMMAND_VCS_SET_COMMIT_MESSAGE");
    assert(umi_command_registry_execute(
               registry, UMI_STUDIO_COMMAND_VCS_SET_COMMIT_MESSAGE,
               "feat: professional source control", message,
               sizeof(message)) == UMI_STATUS_OK);
    assert(umi_command_registry_snapshot(
               registry, UMI_STUDIO_COMMAND_VCS_COMMIT_COMPOSED, &snapshot) ==
           UMI_STATUS_OK);

    assert(umi_command_registry_snapshot(
               registry, UMI_STUDIO_COMMAND_TRADING_REFRESH, &snapshot) ==
           UMI_STATUS_OK);
    assert(strcmp(snapshot.category, "Trading") == 0);
    ReportCommandPhase("UMI_STUDIO_COMMAND_TRADING_FILTER_INSTRUMENTS");
    assert(umi_command_registry_execute(
               registry, UMI_STUDIO_COMMAND_TRADING_FILTER_INSTRUMENTS,
               "CME", message, sizeof(message)) == UMI_STATUS_OK);
    ReportCommandPhase("UMI_STUDIO_COMMAND_TRADING_SELECT_INSTRUMENT");
    assert(umi_command_registry_execute(
               registry, UMI_STUDIO_COMMAND_TRADING_SELECT_INSTRUMENT,
               "CME.ES.REFERENCE", message, sizeof(message)) == UMI_STATUS_OK);
    ReportCommandPhase("UMI_STUDIO_COMMAND_TRADING_PREVIEW_ORDER");
    assert(umi_command_registry_execute(
               registry, UMI_STUDIO_COMMAND_TRADING_PREVIEW_ORDER,
               "", message, sizeof(message)) == UMI_STATUS_OK);
    assert(strstr(message, "allowed") != NULL);

    /* Command-palette tape controls must reach the same Framework workspace
     * used by Studio's dockable Time and Sales view. */
    ReportCommandPhase("UMI_STUDIO_COMMAND_TRADING_FILTER_TRADE_TAPE");
    assert(umi_command_registry_execute(
               registry, UMI_STUDIO_COMMAND_TRADING_FILTER_TRADE_TAPE,
               "buyer:10", message, sizeof(message)) == UMI_STATUS_OK);
    assert(umi_studio_trading_service_snapshot(
               umi_studio_services_trading(
                   umi_studio_bootstrap_services(bootstrap)),
               &trading_snapshot) == UMI_STATUS_OK);
    assert(trading_snapshot.trade_tape.filter ==
           UMI_TRADING_TRADE_TAPE_BUYER_INITIATED);
    assert(trading_snapshot.trade_tape.minimum_size == 10.0);
    ReportCommandPhase("UMI_STUDIO_COMMAND_TRADING_PAUSE_TRADE_TAPE");
    assert(umi_command_registry_execute(
               registry, UMI_STUDIO_COMMAND_TRADING_PAUSE_TRADE_TAPE,
               "", message, sizeof(message)) == UMI_STATUS_OK);
    assert(umi_studio_trading_service_snapshot(
               umi_studio_services_trading(
                   umi_studio_bootstrap_services(bootstrap)),
               &trading_snapshot) == UMI_STATUS_OK);
    assert(trading_snapshot.trade_tape.paused);
    ReportCommandPhase("UMI_STUDIO_COMMAND_TRADING_RESUME_TRADE_TAPE");
    assert(umi_command_registry_execute(
               registry, UMI_STUDIO_COMMAND_TRADING_RESUME_TRADE_TAPE,
               "", message, sizeof(message)) == UMI_STATUS_OK);

    {
        UmiTestPlatformItemSnapshot item = {0};
        UmiStudioTestService *tests = umi_studio_services_tests(
            umi_studio_bootstrap_services(bootstrap));
        (void)strcpy(item.id, "studio.command.alpha");
        (void)strcpy(item.name, "Studio Command Alpha");
        (void)strcpy(item.kind, "test");
        item.enabled = 1;
        assert(umi_test_platform_item_registry_upsert(
                   umi_test_platform_service_item(
                       umi_studio_test_service_platform(tests)), &item) ==
               UMI_STATUS_OK);
        ReportCommandPhase("UMI_STUDIO_COMMAND_TESTS_FILTER");
        assert(umi_command_registry_execute(
                   registry, UMI_STUDIO_COMMAND_TESTS_FILTER, "all",
                   message, sizeof(message)) == UMI_STATUS_OK);
        assert(strstr(message, "1 test(s)") != NULL);
        ReportCommandPhase("UMI_STUDIO_COMMAND_TESTS_SELECT");
        assert(umi_command_registry_execute(
                   registry, UMI_STUDIO_COMMAND_TESTS_SELECT,
                   "studio.command.alpha", message, sizeof(message)) ==
               UMI_STATUS_OK);
        assert(umi_command_registry_snapshot(
                   registry, UMI_STUDIO_COMMAND_TESTS_RUN_SELECTED,
                   &snapshot) == UMI_STATUS_OK);
        assert(strcmp(snapshot.category, "Testing") == 0);
    }

    ReportCommandPhase("UMI_STUDIO_COMMAND_SECURITY_REPORT");
    assert(umi_command_registry_execute(
        registry,
        UMI_STUDIO_COMMAND_SECURITY_REPORT,
        "",
        message,
        sizeof(message)
    ) == UMI_STATUS_OK);
    assert(strstr(message, "Security:") != NULL);

    ReportCommandPhase("UMI_STUDIO_COMMAND_SESSION_SAVE");
    assert(umi_command_registry_execute(
        registry,
        UMI_STUDIO_COMMAND_SESSION_SAVE,
        "",
        message,
        sizeof(message)
    ) == UMI_STATUS_OK);
    assert(umi_fs_is_file(".umicom/studio.session"));

    ReportCommandPhase("bootstrap-stop");
    assert(umi_studio_bootstrap_stop(bootstrap) == UMI_STATUS_OK);
    ReportCommandPhase("bootstrap-destroy");
    umi_studio_bootstrap_destroy(bootstrap);
    assert(umi_fs_remove_tree(".umicom") == UMI_STATUS_OK);
    ReportCommandPhase("complete");
    return 0;
}
