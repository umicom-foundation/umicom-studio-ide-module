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
/* Keep this contract executable in Release as well as Debug. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <string.h>

#include "umicom/studio/learning_centre.h"
#include "umicom/teacher/foundations_curriculum.h"

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioLearningCentreSnapshot centre;
    UmiStudioLearningLessonSnapshot lesson;

    assert(umi_studio_learning_centre_snapshot(&centre) == UMI_STATUS_OK);
    assert(centre.interactive == 1);
    assert(centre.lesson_count == umi_teacher_foundations_curriculum_count());
    assert(centre.estimated_minutes >= 800U);
    assert(umi_studio_learning_centre_lesson_at(0U, &lesson) ==
           UMI_STATUS_OK);
    assert(strcmp(lesson.lesson_id, "foundations.welcome") == 0);
    assert(umi_studio_learning_centre_find_lesson(
        "foundations.pull-request", &lesson) == UMI_STATUS_OK);
    assert(lesson.requires_github_account == 1);
    assert(umi_studio_learning_centre_lesson_at(
        centre.lesson_count, &lesson) == UMI_STATUS_NOT_FOUND);
    /* Compare every projection with its owner, not a second application list. */
    for (size_t index = 0U; index < centre.lesson_count; ++index) {
        const UmiTeacherFoundationsLesson *source =
            umi_teacher_foundations_curriculum_at(index);
        assert(source != NULL);
        assert(umi_studio_learning_centre_lesson_at(index, &lesson) == UMI_STATUS_OK);
        assert(strcmp(lesson.lesson_id, source->id) == 0);
        assert(strcmp(lesson.resource_path, source->resource_path) == 0);
        assert(lesson.sequence == source->sequence);
    }
    assert(umi_studio_learning_centre_find_lesson("foundations.cmake", &lesson) == UMI_STATUS_OK);
    assert(umi_studio_learning_centre_find_lesson("foundations.assembly", &lesson) == UMI_STATUS_OK);
    return 0;
}
