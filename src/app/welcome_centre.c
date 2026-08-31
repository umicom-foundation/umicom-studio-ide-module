/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/welcome_centre.c
 *
 * PURPOSE:
 *   Implement Studio welcome/start centre for projects, recent workspaces and learning actions.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* BEGINNER NOTE:
 * Studio composes reusable Framework capabilities here. Generic models and
 * engines stay in Framework so the same mechanisms can serve future products.
 */
#include "umicom/studio/welcome_centre.h"
#include "umicom/studio/learning_centre.h"
#include "umicom/studio/productisation_contribution.h"

#include <stdint.h>
#include <string.h>

/* Copy display text into fixed snapshot storage and preserve a terminator. */
static void copy_text(char *destination,
                      size_t capacity,
                      const char *source)
{
    size_t length;

    /* An absent destination cannot receive safe display text. */
    if (destination == NULL || capacity == 0U) {
        return;
    }

    /* Treat optional missing source text as an empty user-facing value. */
    if (source == NULL) {
        source = "";
    }

    length = strlen(source);
    /* Trim display text when it cannot fit; stable identifiers use Framework's
     * stricter workspace-guide copier before reaching this presentation code. */
    if (length >= capacity) {
        length = capacity - 1U;
    }

    /* Avoid asking memcpy to copy from an empty optional string. */
    if (length > 0U) {
        (void)memcpy(destination, source, length);
    }
    destination[length] = '\0';
}

/* Compose project actions, lessons and canonical workspace choices. */
UmiStatus umi_studio_welcome_centre_snapshot(
    UmiStudioServices *services,
    UmiStudioWelcomeCentreSnapshot *out_snapshot)
{
    UmiStudioLearningCentreSnapshot learning;
    UmiProductWorkspaceGuide workspace_guide;
    UmiStatus status;
    (void)services;
    /* A caller must supply storage for the toolkit-neutral snapshot. */
    if (out_snapshot == NULL) return UMI_STATUS_INVALID_ARGUMENT;

    status = umi_studio_product_workspace_guide(&workspace_guide);
    /* Do not display stale copied workspace choices when Framework rejects them. */
    if (status != UMI_STATUS_OK) return status;

    (void)memset(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->struct_size = (uint32_t)sizeof(*out_snapshot);
    out_snapshot->api_version = 1U;
    copy_text(out_snapshot->area_id, sizeof(out_snapshot->area_id),
              "studio.welcome-centre");
    copy_text(out_snapshot->view_type, sizeof(out_snapshot->view_type),
              "studio.welcome-centre");
    copy_text(out_snapshot->title, sizeof(out_snapshot->title),
              "Welcome Centre");
    copy_text(
        out_snapshot->summary, sizeof(out_snapshot->summary),
        "Create, open, clone and resume projects, or begin the guided C and "
        "Umicom learning journey. Choose a ready-made workspace and customise "
        "its panels whenever your work changes.");
    copy_text(out_snapshot->recommended_layout_id,
              sizeof(out_snapshot->recommended_layout_id),
              workspace_guide.recommended_layout_id);
    out_snapshot->workspace_choice_count = workspace_guide.choice_count;
    out_snapshot->readiness_percent = workspace_guide.readiness_percent;
    out_snapshot->product_ready = workspace_guide.acceptance_ready;
    /* Learning lessons are optional, while three project commands and the
     * canonical workspace choices remain available on every start. */
    if (umi_studio_learning_centre_snapshot(&learning) == UMI_STATUS_OK) {
        out_snapshot->item_count =
            3U + workspace_guide.choice_count + learning.lesson_count;
    } else {
        /* A learning-service problem must not hide project and layout actions. */
        out_snapshot->item_count = 3U + workspace_guide.choice_count;
    }
    out_snapshot->revision = 3U;
    out_snapshot->available = 1;
    return UMI_STATUS_OK;
}
