/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/compiler_views.h
 *
 * PURPOSE:
 *   Publish Studio's public compiler views contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_COMPILER_VIEWS_H
#define UMICOM_STUDIO_COMPILER_VIEWS_H
#include <stddef.h>
#include <stdbool.h>
/**
 * List the named studio compiler view region values accepted by this public contract.
 */
typedef enum UmiStudioCompilerViewRegion { UMI_STUDIO_COMPILER_VIEW_PRIMARY = 1, UMI_STUDIO_COMPILER_VIEW_SECONDARY, UMI_STUDIO_COMPILER_VIEW_BOTTOM } UmiStudioCompilerViewRegion;
/**
 * Represent the studio compiler view data shared with callers of this public contract.
 */
typedef struct UmiStudioCompilerView { const char *view_id; const char *title; const char *icon; const char *description; UmiStudioCompilerViewRegion default_region; bool closable; bool movable; bool supports_multiple_instances; } UmiStudioCompilerView;
/**
 * Return the number of records represented by studio compiler views without changing their
 * state.
 */
size_t umi_studio_compiler_views_count(void);
/**
 * Find studio compiler views while leaving the underlying catalogue or model owned by this
 * module.
 */
const UmiStudioCompilerView *umi_studio_compiler_views_at(size_t index);
/**
 * Find studio compiler views while leaving the underlying catalogue or model owned by this
 * module.
 */
const UmiStudioCompilerView *umi_studio_compiler_views_find(const char *view_id);
#endif
