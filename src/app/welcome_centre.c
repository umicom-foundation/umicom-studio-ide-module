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

#include <stdint.h>
#include <string.h>

static void copy_text(char *destination,
                      size_t capacity,
                      const char *source)
{
    size_t length;
    if (destination == NULL || capacity == 0U) return;
    if (source == NULL) source = "";
    length = strlen(source);
    if (length >= capacity) length = capacity - 1U;
    if (length > 0U) (void)memcpy(destination, source, length);
    destination[length] = '\0';
}

UmiStatus umi_studio_welcome_centre_snapshot(
    UmiStudioServices *services,
    UmiStudioWelcomeCentreSnapshot *out_snapshot)
{
    UmiStudioLearningCentreSnapshot learning;
    (void)services;
    if (out_snapshot == NULL) return UMI_STATUS_INVALID_ARGUMENT;

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
        "Umicom learning journey.");
    if (umi_studio_learning_centre_snapshot(&learning) == UMI_STATUS_OK) {
        /* Create, open and clone are followed by the Framework lessons. */
        out_snapshot->item_count = 3U + learning.lesson_count;
    }
    out_snapshot->revision = 2U;
    out_snapshot->available = 1;
    return UMI_STATUS_OK;
}
