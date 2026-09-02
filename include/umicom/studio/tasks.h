/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/tasks.h
 *
 * PURPOSE:
 *   Define Studio task submission and completion helpers over the Framework
 *   task queue so build, search, indexing and language work share one worker
 *   infrastructure.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_TASKS_H
#define UMICOM_STUDIO_TASKS_H

#include "umicom/studio/services.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Provide the studio tasks submit operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_tasks_submit(UmiStudioServices *services,
                                  const UmiTaskConfig *config,
                                  UmiTask **out_task);
/**
 * Release or reset state held by studio tasks wait and so the same storage can be reused
 * safely.
 */
UmiStatus umi_studio_tasks_wait_and_destroy(UmiTask *task,
                                            uint32_t timeout_ms);
/**
 * Provide the studio tasks stats operation used by this module and its client
 * applications.
 */
UmiTaskQueueStats umi_studio_tasks_stats(UmiStudioServices *services);

#ifdef __cplusplus
}
#endif

#endif
