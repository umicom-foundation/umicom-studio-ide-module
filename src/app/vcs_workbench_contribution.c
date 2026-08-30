/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/vcs_workbench_contribution.c
 *
 * PURPOSE:
 *   Define Studio-only placement for the Framework VCS workbench. Runtime,
 *   comparison, merge, staging and review behaviour stays in Framework.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/vcs_workbench_contribution.h"

#include <string.h>

#define COMMAND(identifier, menu, group, position)                            \
    {                                                                          \
        (uint32_t)sizeof(UmiStudioVcsWorkbenchCommandContribution),            \
        UMI_STUDIO_VCS_WORKBENCH_CONTRIBUTION_API_VERSION,                     \
        identifier, menu, group, position, 1                                   \
    }

static const UmiStudioVcsWorkbenchCommandContribution COMMANDS[] = {
    COMMAND("vcs.workbench.compare.files", "Tools", "compare", 100),
    COMMAND("vcs.workbench.compare.folders", "Tools", "compare", 110),
    COMMAND("vcs.workbench.compare.revisions", "Source Control", "compare", 120),
    COMMAND("vcs.workbench.view.side-by-side", "View", "compare-layout", 200),
    COMMAND("vcs.workbench.view.inline", "View", "compare-layout", 210),
    COMMAND("vcs.workbench.view.linked-scroll", "View", "compare-layout", 220),
    COMMAND("vcs.workbench.change.next", "Navigate", "differences", 300),
    COMMAND("vcs.workbench.change.previous", "Navigate", "differences", 310),
    COMMAND("vcs.workbench.whitespace.exact", "View", "compare-policy", 400),
    COMMAND("vcs.workbench.whitespace.ignore", "View", "compare-policy", 410),
    COMMAND("vcs.workbench.merge.use-base", "Source Control", "merge", 500),
    COMMAND("vcs.workbench.merge.use-ours", "Source Control", "merge", 510),
    COMMAND("vcs.workbench.merge.use-theirs", "Source Control", "merge", 520),
    COMMAND("vcs.workbench.merge.use-both", "Source Control", "merge", 530),
    COMMAND("vcs.workbench.merge.finish", "Source Control", "merge", 540),
    COMMAND("vcs.workbench.merge.abort", "Source Control", "merge", 550),
    COMMAND("vcs.workbench.stage.hunk", "Source Control", "staging", 600),
    COMMAND("vcs.workbench.unstage.hunk", "Source Control", "staging", 610),
    COMMAND("vcs.workbench.stage.selected", "Source Control", "staging", 620),
    COMMAND("vcs.workbench.review.comment", "Source Control", "review", 700),
    COMMAND("vcs.workbench.review.resolve", "Source Control", "review", 710),
    COMMAND("vcs.workbench.review.approve", "Source Control", "review", 720),
    COMMAND("vcs.workbench.review.request-changes", "Source Control", "review", 730),
    COMMAND("vcs.workbench.export.patch", "File", "export", 800),
    COMMAND("vcs.workbench.export.report", "File", "export", 810),
    COMMAND("vcs.workbench.refresh", "Source Control", "compare", 900)
};

#undef COMMAND

#define VIEW(identifier, label, contract, region, position)                   \
    {                                                                          \
        (uint32_t)sizeof(UmiStudioVcsWorkbenchViewContribution),               \
        UMI_STUDIO_VCS_WORKBENCH_CONTRIBUTION_API_VERSION,                     \
        identifier, label, contract, region, position, 1, 1                    \
    }

static const UmiStudioVcsWorkbenchViewContribution VIEWS[] = {
    VIEW("studio.vcs-workbench.compare", "Compare",
         "umicom.vcs.workbench.compare-model", "editor-area", 200),
    VIEW("studio.vcs-workbench.difference-map", "Difference Map",
         "umicom.vcs.workbench.difference-map", "secondary-sidebar", 210),
    VIEW("studio.vcs-workbench.directory-compare", "Folder Compare",
         "umicom.vcs.workbench.directory-model", "editor-area", 220),
    VIEW("studio.vcs-workbench.merge", "Merge",
         "umicom.vcs.workbench.merge-model", "editor-area", 230),
    VIEW("studio.vcs-workbench.partial-staging", "Partial Staging",
         "umicom.vcs.workbench.partial-stage-model", "left-sidebar", 240),
    VIEW("studio.vcs-workbench.review", "Review",
         "umicom.vcs.workbench.review-session", "secondary-sidebar", 250)
};

#undef VIEW

size_t umi_studio_vcs_workbench_command_count(void)
{
    return sizeof(COMMANDS) / sizeof(COMMANDS[0]);
}

const UmiStudioVcsWorkbenchCommandContribution *
umi_studio_vcs_workbench_command_at(size_t index)
{
    return index < umi_studio_vcs_workbench_command_count()
        ? &COMMANDS[index] : NULL;
}

const UmiStudioVcsWorkbenchCommandContribution *
umi_studio_vcs_workbench_command_find(const char *framework_command_id)
{
    size_t index;
    if (framework_command_id == NULL) return NULL;
    for (index = 0U; index < umi_studio_vcs_workbench_command_count(); ++index) {
        if (strcmp(COMMANDS[index].framework_command_id,
                   framework_command_id) == 0) {
            return &COMMANDS[index];
        }
    }
    return NULL;
}

size_t umi_studio_vcs_workbench_view_count(void)
{
    return sizeof(VIEWS) / sizeof(VIEWS[0]);
}

const UmiStudioVcsWorkbenchViewContribution *
umi_studio_vcs_workbench_view_at(size_t index)
{
    return index < umi_studio_vcs_workbench_view_count()
        ? &VIEWS[index] : NULL;
}

const UmiStudioVcsWorkbenchViewContribution *
umi_studio_vcs_workbench_view_find(const char *view_id)
{
    size_t index;
    if (view_id == NULL) return NULL;
    for (index = 0U; index < umi_studio_vcs_workbench_view_count(); ++index) {
        if (strcmp(VIEWS[index].view_id, view_id) == 0) return &VIEWS[index];
    }
    return NULL;
}

UmiStatus umi_studio_vcs_workbench_create(
    UmiVcsWorkbenchRuntime **out_runtime)
{
    return umi_vcs_workbench_runtime_create(out_runtime);
}
