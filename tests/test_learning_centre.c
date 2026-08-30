/*-----------------------------------------------------------------------------
 * Umicom Studio IDE Tests
 * File: applications/studio/tests/test_learning_centre.c
 *
 * PURPOSE:
 *   Verify Studio exposes the Framework foundations journey as an interactive,
 *   ordered set of beginner-friendly lesson snapshots.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include <assert.h>
#include <string.h>

#include "umicom/studio/learning_centre.h"

int main(void)
{
    UmiStudioLearningCentreSnapshot centre;
    UmiStudioLearningLessonSnapshot lesson;

    assert(umi_studio_learning_centre_snapshot(&centre) == UMI_STATUS_OK);
    assert(centre.interactive == 1);
    assert(centre.lesson_count == 16U);
    assert(centre.estimated_minutes >= 800U);
    assert(umi_studio_learning_centre_lesson_at(0U, &lesson) ==
           UMI_STATUS_OK);
    assert(strcmp(lesson.lesson_id, "foundations.welcome") == 0);
    assert(umi_studio_learning_centre_find_lesson(
        "foundations.pull-request", &lesson) == UMI_STATUS_OK);
    assert(lesson.requires_github_account == 1);
    assert(umi_studio_learning_centre_lesson_at(
        centre.lesson_count, &lesson) == UMI_STATUS_NOT_FOUND);
    return 0;
}
