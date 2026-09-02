/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/learning_centre.c
 *
 * PURPOSE:
 *   Adapt reusable Framework lessons to Studio-owned presentation snapshots
 *   without copying curriculum, progression or assessment logic.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/learning_centre.h"

#include <stdio.h>
#include <string.h>

/* Provide the copy text operation used by this module and its client applications. */
static UmiStatus copy_text(char *destination,
                           size_t capacity,
                           const char *source)
{
    int written;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (destination == NULL || capacity == 0U || source == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    written = snprintf(destination, capacity, "%s", source);
    return written >= 0 && (size_t)written < capacity
        ? UMI_STATUS_OK : UMI_STATUS_CAPACITY_EXCEEDED;
}

/* Provide the project lesson operation used by this module and its client applications. */
static UmiStatus project_lesson(
    const UmiTeacherFoundationsLesson *lesson,
    UmiStudioLearningLessonSnapshot *out_lesson)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (lesson == NULL || out_lesson == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    (void)memset(out_lesson, 0, sizeof(*out_lesson));
    out_lesson->struct_size = (uint32_t)sizeof(*out_lesson);
    out_lesson->api_version = 1U;

    status = copy_text(out_lesson->lesson_id,
                       sizeof(out_lesson->lesson_id), lesson->id);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = copy_text(out_lesson->title,
                           sizeof(out_lesson->title), lesson->title);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = copy_text(out_lesson->summary,
                           sizeof(out_lesson->summary), lesson->summary);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = copy_text(out_lesson->exercise,
                           sizeof(out_lesson->exercise), lesson->exercise);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = copy_text(out_lesson->resource_path,
                           sizeof(out_lesson->resource_path),
                           lesson->resource_path);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = copy_text(
            out_lesson->stage, sizeof(out_lesson->stage),
            umi_teacher_foundations_stage_text(lesson->stage));
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    out_lesson->sequence = lesson->sequence;
    out_lesson->estimated_minutes = lesson->estimated_minutes;
    out_lesson->required_score = lesson->required_score;
    out_lesson->requires_compiler = lesson->requires_compiler;
    out_lesson->requires_repository = lesson->requires_repository;
    out_lesson->requires_github_account = lesson->requires_github_account;
    return UMI_STATUS_OK;
}

/*
 * Provide the studio learning centre snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_learning_centre_snapshot(
    UmiStudioLearningCentreSnapshot *out_snapshot)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (out_snapshot == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    (void)memset(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->struct_size = (uint32_t)sizeof(*out_snapshot);
    out_snapshot->api_version = 1U;
    (void)copy_text(out_snapshot->centre_id,
                    sizeof(out_snapshot->centre_id),
                    "studio.learning.foundations");
    (void)copy_text(out_snapshot->title,
                    sizeof(out_snapshot->title),
                    "Learn C and Umicom from the beginning");
    (void)copy_text(
        out_snapshot->summary, sizeof(out_snapshot->summary),
        "Short lessons and practical exercises lead from computer basics to "
        "your first reviewed Umicom contribution.");
    out_snapshot->lesson_count = umi_teacher_foundations_curriculum_count();
    out_snapshot->estimated_minutes =
        umi_teacher_foundations_curriculum_minutes();
    out_snapshot->revision = 1U;
    out_snapshot->interactive = 1;
    return UMI_STATUS_OK;
}

/*
 * Find studio learning centre lesson while leaving the underlying catalogue or model owned
 * by this module.
 */
UmiStatus umi_studio_learning_centre_lesson_at(
    size_t index,
    UmiStudioLearningLessonSnapshot *out_lesson)
{
    const UmiTeacherFoundationsLesson *lesson =
        umi_teacher_foundations_curriculum_at(index);
    return lesson != NULL ? project_lesson(lesson, out_lesson)
                          : UMI_STATUS_NOT_FOUND;
}

/*
 * Provide the studio learning centre find lesson operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_learning_centre_find_lesson(
    const char *lesson_id,
    UmiStudioLearningLessonSnapshot *out_lesson)
{
    const UmiTeacherFoundationsLesson *lesson =
        umi_teacher_foundations_curriculum_find(lesson_id);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (lesson_id == NULL || out_lesson == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    return lesson != NULL ? project_lesson(lesson, out_lesson)
                          : UMI_STATUS_NOT_FOUND;
}
