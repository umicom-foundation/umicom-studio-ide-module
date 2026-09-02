/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/learning_centre.h
 *
 * PURPOSE:
 *   Project the Framework-owned foundations curriculum into stable Studio view
 *   snapshots that GTK4, console and future web frontends can render.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_LEARNING_CENTRE_H
#define UMICOM_STUDIO_LEARNING_CENTRE_H

#include "umicom/umicom.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio learning lesson snapshot data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioLearningLessonSnapshot {
    uint32_t struct_size;
    uint32_t api_version;
    char lesson_id[UMI_TEACHER_ID_CAPACITY];
    char title[UMI_TEACHER_TEXT_CAPACITY];
    char summary[UMI_TEACHER_TEXT_CAPACITY];
    char exercise[UMI_TEACHER_TEXT_CAPACITY];
    char resource_path[UMI_TEACHER_PATH_CAPACITY];
    char stage[UMI_TEACHER_ID_CAPACITY];
    uint32_t sequence;
    uint32_t estimated_minutes;
    uint32_t required_score;
    int requires_compiler;
    int requires_repository;
    int requires_github_account;
} UmiStudioLearningLessonSnapshot;

/**
 * Represent the studio learning centre snapshot data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioLearningCentreSnapshot {
    uint32_t struct_size;
    uint32_t api_version;
    char centre_id[UMI_TEACHER_ID_CAPACITY];
    char title[UMI_TEACHER_TEXT_CAPACITY];
    char summary[UMI_TEACHER_TEXT_CAPACITY];
    size_t lesson_count;
    uint32_t estimated_minutes;
    uint64_t revision;
    int interactive;
} UmiStudioLearningCentreSnapshot;

/**
 * Provide the studio learning centre snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_learning_centre_snapshot(
    UmiStudioLearningCentreSnapshot *out_snapshot);
/**
 * Find studio learning centre lesson while leaving the underlying catalogue or model owned
 * by this module.
 */
UmiStatus umi_studio_learning_centre_lesson_at(
    size_t index,
    UmiStudioLearningLessonSnapshot *out_lesson);
/**
 * Provide the studio learning centre find lesson operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_learning_centre_find_lesson(
    const char *lesson_id,
    UmiStudioLearningLessonSnapshot *out_lesson);

#ifdef __cplusplus
}
#endif

#endif
