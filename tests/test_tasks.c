/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_tasks.c
 *
 * PURPOSE:
 *   Verify that Studio submits background work through the shared Framework
 *   task queue and retains clear task ownership and completion semantics.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "studio_test_state.h"
#include "umicom/studio/tasks.h"

/* This executable performs its setup in assertions; keep them active in Release. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdatomic.h>

/*
 * Exercise work and return a clear result when the behaviour no longer matches its
 * contract.
 */
static UmiStatus work(UmiTaskContext *context, void *user_data)
{
    atomic_int *counter = (atomic_int *)user_data;
    (void)atomic_fetch_add(counter, 1);
    return umi_task_context_report(context, 100U, "done");
}

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioServices *services = NULL;
    UmiTask *task = NULL;
    UmiTaskConfig config;
    atomic_int counter;
    UmiTaskQueueStats stats;

    /* Keep the former caller-relative cleanup for review. Each invocation now
     * owns a unique state directory, so direct execution cannot erase user state. */
#if 0
    (void)umi_fs_remove_tree(".umicom");
#endif
    StudioTestState test_state = {0};
    if (!StudioTestStateBegin(&test_state)) return 1;
    atomic_init(&counter, 0);
    assert(umi_studio_services_create(NULL, NULL, &services) == UMI_STATUS_OK);

    config.label = "Studio task";
    config.function = work;
    config.user_data = &counter;
    config.progress_sink = NULL;
    config.progress_user_data = NULL;

    assert(umi_studio_tasks_submit(services,
                                   &config,
                                   &task) == UMI_STATUS_OK);
    assert(umi_studio_tasks_wait_and_destroy(task,
                                             5000U) == UMI_STATUS_OK);
    assert(atomic_load(&counter) == 1);

    /* Task completion precedes queue bookkeeping. Wait for queue idleness
     * before checking aggregate counters, rather than depending on timing. */
    assert(umi_task_queue_wait_idle(umi_studio_services_task_queue(services),
                                    5000U) == UMI_STATUS_OK);
    stats = umi_studio_tasks_stats(services);
    assert(stats.submitted == 1U);
    assert(stats.completed == 1U);
    assert(stats.failed == 0U);

    umi_studio_services_destroy(services);
    /* Retain generated evidence instead of deleting a launch-directory tree. */
#if 0
    assert(umi_fs_remove_tree(".umicom") == UMI_STATUS_OK);
#endif
    if (!StudioTestStateEnd(&test_state)) return 1;
    return 0;
}
