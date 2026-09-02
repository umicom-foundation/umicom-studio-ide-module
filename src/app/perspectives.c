/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/perspectives.c
 *
 * PURPOSE:
 *   Register the canonical Studio perspectives in deterministic order.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/perspectives.h"

#include <string.h>

static const UmiUiPerspectiveSnapshot STUDIO_PERSPECTIVES[] = {
    { UMI_STUDIO_PERSPECTIVE_DEVELOP, "Develop", "Edit and navigate source", "applications-development-symbolic", 10, 1 },
    { UMI_STUDIO_PERSPECTIVE_BUILD, "Build", "Configure, compile and inspect build output", "system-run-symbolic", 20, 0 },
    { UMI_STUDIO_PERSPECTIVE_TEST, "Test", "Discover and execute automated tests", "emblem-ok-symbolic", 30, 0 },
    { UMI_STUDIO_PERSPECTIVE_DEBUG, "Debug", "Control execution and inspect threads, stack frames, variables, watches, breakpoints and console output", "applications-engineering-symbolic", 40, 0 },
    { UMI_STUDIO_PERSPECTIVE_SOURCE_CONTROL, "Source Control", "Review changes, compose commits, inspect history, branches, remotes, conflicts, diffs and operations", "vcs-branch-symbolic", 50, 0 },
    { UMI_STUDIO_PERSPECTIVE_AI, "AI", "Use governed AI and AuthorEngine services", "system-search-symbolic", 60, 0 },
    { UMI_STUDIO_PERSPECTIVE_DESIGNER, "Designer", "Compose declarative Umicom applications", "applications-graphics-symbolic", 70, 0 },
    { UMI_STUDIO_PERSPECTIVE_RELEASE, "Release", "Build evidence, packages and delivery", "package-x-generic-symbolic", 80, 0 }
};

/* Add studio perspectives only after its inputs and available capacity have been checked. */
UmiStatus umi_studio_perspectives_register(UmiUiWorkbench *workbench)
{
    UmiUiPerspectiveModel *model;
    size_t index;
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (workbench == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    model = umi_ui_workbench_perspectives(workbench);
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U;
         index < sizeof(STUDIO_PERSPECTIVES) / sizeof(STUDIO_PERSPECTIVES[0]);
         ++index) {
        status = umi_ui_perspective_model_upsert(model, &STUDIO_PERSPECTIVES[index]);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) {
            return status;
        }
    }
    return umi_ui_workbench_activate_perspective(workbench,
                                                  UMI_STUDIO_PERSPECTIVE_DEVELOP);
}

/*
 * Return the number of records represented by studio perspective definition without
 * changing their state.
 */
size_t umi_studio_perspective_definition_count(void)
{
    return sizeof(STUDIO_PERSPECTIVES) / sizeof(STUDIO_PERSPECTIVES[0]);
}
