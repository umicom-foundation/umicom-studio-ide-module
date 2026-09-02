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
#include "umicom/designer/surface_interaction.h"
#include "umicom/studio/declarative.h"
#include "umicom/designer/workspace_model.h"
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
 * Replace the current Studio selection with one existing semantic component.
 * Lower-level Framework selection functions remain available for explicit
 * multi-selection workflows.
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
/**
 * Project the synchronized component palette, hierarchy and property inspector
 * used by Studio's dockable visual-design panels.
 */
UmiStatus umi_studio_designer_workspace_model(
    const UmiStudioDesigner *designer,
    const char *palette_query,
    UmiDesignerWorkspaceModel *out_model);
/**
 * Add a registered component beneath the selected container, or beneath the
 * design root when the selection cannot contain children. The generated node
 * identifier is returned so the frontend can focus the new component.
 */
UmiStatus umi_studio_designer_add_palette_component(
    UmiStudioDesigner *designer,
    const char *component_type,
    char *out_node_id,
    size_t node_id_capacity);
/**
 * Change a property on the currently selected component through the existing
 * undoable designer operation path.
 */
UmiStatus umi_studio_designer_set_selected_property(
    UmiStudioDesigner *designer,
    const char *property_name,
    const char *value_text);
/**
 * Copy the revision-aware live source state used by Code, Mixed and Preview
 * panels. The returned record is independent from the designer's ownership.
 */
UmiStatus umi_studio_designer_live_source_snapshot(
    const UmiStudioDesigner *designer,
    UmiDesignerLiveSourceWorkspace *out_workspace);
/**
 * Parse one debounced source edit and replace the semantic design only when
 * the complete text is valid. Invalid text keeps the previous visual model.
 */
UmiStatus umi_studio_designer_apply_live_source(
    UmiStudioDesigner *designer,
    const char *source,
    uint64_t now_millis,
    UmiDeclDiagnosticList *diagnostics);
/**
 * Select the topmost component below a canvas point and begin either moving
 * its body or resizing the handle under the pointer.
 */
UmiStatus umi_studio_designer_begin_surface_interaction(
    UmiStudioDesigner *designer,
    int32_t pointer_x,
    int32_t pointer_y,
    int32_t canvas_width,
    int32_t canvas_height,
    const UmiDesignerSurfaceOptions *options,
    UmiDesignerSurfaceInteraction *out_interaction);
/** Update a direct-manipulation preview without changing the saved design. */
UmiStatus umi_studio_designer_update_surface_interaction(
    UmiDesignerSurfaceInteraction *interaction,
    int32_t pointer_x,
    int32_t pointer_y);
/**
 * Commit a completed gesture through Studio's shared undo history and refresh
 * the canonical source and preview projections.
 */
UmiStatus umi_studio_designer_commit_surface_interaction(
    UmiStudioDesigner *designer,
    UmiDesignerSurfaceInteraction *interaction);
/** Cancel a gesture without changing document or history state. */
void umi_studio_designer_cancel_surface_interaction(
    UmiDesignerSurfaceInteraction *interaction);
#ifdef __cplusplus
}
#endif
#endif
