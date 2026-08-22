/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/context_link_centre.h
 *
 * PURPOSE:
 *   Compose Studio-specific group and endpoint definitions over the reusable Framework context-host runtime.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_CONTEXT_LINK_CENTRE_H
#define UMICOM_STUDIO_CONTEXT_LINK_CENTRE_H

#include <stdint.h>

#include "umicom/studio/ui.h"
#include "umicom/platform/session_store.h"
#include "umicom/workbench_context_host/workbench_context_host.h"

typedef struct UmiStudioContextLinkCentre UmiStudioContextLinkCentre;

UmiStatus umi_studio_context_link_centre_create(
    UmiUiWorkbench *workbench,
    UmiSessionStore *session,
    UmiStudioContextLinkCentre **out_centre);
void umi_studio_context_link_centre_destroy(
    UmiStudioContextLinkCentre *centre);
UmiStatus umi_studio_context_link_centre_refresh(
    UmiStudioContextLinkCentre *centre,
    uint64_t now_ms);
UmiWorkbenchContextHost *umi_studio_context_link_centre_host(
    UmiStudioContextLinkCentre *centre);

UmiStatus umi_studio_context_link_centre_publish_project(
    UmiStudioContextLinkCentre *centre,
    const char *project_id,
    const char *repository_id,
    const char *root_path,
    const char *target_id,
    const char *configuration_id,
    const char *language_id,
    uint64_t now_ms);
UmiStatus umi_studio_context_link_centre_publish_source_location(
    UmiStudioContextLinkCentre *centre,
    const char *file_path,
    const char *symbol,
    uint32_t line,
    uint32_t column,
    uint32_t selection_length,
    uint64_t now_ms);
UmiStatus umi_studio_context_link_centre_publish_diagnostic(
    UmiStudioContextLinkCentre *centre,
    const char *file_path,
    uint32_t line,
    uint32_t column,
    const char *severity,
    const char *diagnostic_code,
    const char *message,
    uint64_t now_ms);
UmiStatus umi_studio_context_link_centre_publish_source_control(
    UmiStudioContextLinkCentre *centre,
    const char *project_id,
    const char *root_path,
    const char *repository_id,
    const char *branch,
    const char *path,
    const char *change_kind,
    uint64_t now_ms);
UmiStatus umi_studio_context_link_centre_publish_debug_location(
    UmiStudioContextLinkCentre *centre,
    const char *file_path,
    const char *symbol,
    uint32_t line,
    uint32_t column,
    uint64_t now_ms);
UmiStatus umi_studio_context_link_centre_publish_test(
    UmiStudioContextLinkCentre *centre,
    const char *test_id,
    const char *suite_id,
    const char *outcome,
    const char *source_uri,
    uint64_t source_line,
    uint64_t duration_ms,
    uint64_t now_ms);
UmiStatus umi_studio_context_link_centre_publish_ai(
    UmiStudioContextLinkCentre *centre,
    const char *conversation_id,
    const char *message_id,
    const char *provider_id,
    const char *model_id,
    const char *evidence_id,
    uint64_t now_ms);

#endif
