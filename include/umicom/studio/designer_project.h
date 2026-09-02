/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/designer_project.h
 *
 * PURPOSE:
 *   Provide Studio project-level helpers for storing multiple reusable designer documents under one project identity.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This Studio-facing contract composes reusable Framework capabilities.
 * Generic designer and declarative logic remains owned by Umicom Framework.
 */

#ifndef UMICOM_STUDIO_DESIGNER_PROJECT_H
#define UMICOM_STUDIO_DESIGNER_PROJECT_H
#include "umicom/studio/designer.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Represent the studio designer project data shared with callers of this public contract.
 */
typedef struct UmiStudioDesignerProject { UmiDesignerProject *project; } UmiStudioDesignerProject;
/**
 * Initialise studio designer project from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_designer_project_create(const char *project_id,UmiStudioDesignerProject *out_project);
/**
 * Release or reset state held by studio designer project so the same storage can be reused
 * safely.
 */
void umi_studio_designer_project_dispose(UmiStudioDesignerProject *project);
#ifdef __cplusplus
}
#endif
#endif
