/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/designer.h
 *
 * PURPOSE:
 *   Compose the reusable Framework designer engine into Studio authoring operations, undo/redo, selection and preview.
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

#ifndef UMICOM_STUDIO_DESIGNER_H
#define UMICOM_STUDIO_DESIGNER_H
#include "umicom/studio/declarative.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Represent the studio designer data shared with callers of this public contract.
 */
typedef struct UmiStudioDesigner UmiStudioDesigner;
/**
 * Represent the studio designer snapshot data shared with callers of this public contract.
 */
typedef struct UmiStudioDesignerSnapshot { UmiDesignerDocumentSnapshot document; size_t palette_items; size_t selected_items; size_t undo_count; size_t redo_count; } UmiStudioDesignerSnapshot;
/**
 * Initialise studio designer from caller-provided values so later operations receive a
 * known state.
 */
UmiStatus umi_studio_designer_create(UmiStudioDeclarative *declarative,UmiStudioDesigner **out_designer);
/**
 * Release or reset state held by studio designer so the same storage can be reused safely.
 */
void umi_studio_designer_destroy(UmiStudioDesigner *designer);
/**
 * Provide the studio designer bind commands operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_designer_bind_commands(UmiStudioDesigner *designer,UmiCommandRegistry *commands);
/**
 * Provide the studio designer new operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_designer_new(UmiStudioDesigner *designer,const char *application_id);
/**
 * Provide the studio designer load text operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_designer_load_text(UmiStudioDesigner *designer,const char *source,UmiDeclDiagnosticList *diagnostics);
/**
 * Provide the studio designer add component operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_designer_add_component(UmiStudioDesigner *designer,const char *node_id,const char *component_type,const char *parent_id);
/**
 * Provide the studio designer set property operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_designer_set_property(UmiStudioDesigner *designer,const char *node_id,const char *property_name,const char *value_text);
/**
 * Provide the studio designer select operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_designer_select(UmiStudioDesigner *designer,const char *node_id);
/**
 * Provide the studio designer undo operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_designer_undo(UmiStudioDesigner *designer);
/**
 * Provide the studio designer redo operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_designer_redo(UmiStudioDesigner *designer);
/**
 * Provide the studio designer preview operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_designer_preview(UmiStudioDesigner *designer,char *out_text,size_t capacity,UmiDeclDiagnosticList *diagnostics);
/**
 * Provide the studio designer snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_designer_snapshot(const UmiStudioDesigner *designer,UmiStudioDesignerSnapshot *out_snapshot);
/**
 * Provide the studio designer document operation used by this module and its client
 * applications.
 */
UmiDesignerDocument *umi_studio_designer_document(UmiStudioDesigner *designer);
#ifdef __cplusplus
}
#endif
#endif
