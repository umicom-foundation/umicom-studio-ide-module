/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_designer_workspace_model.c
 *
 * PURPOSE:
 *   Verify Studio's thin designer facade projects synchronized Framework panel
 *   models and inserts palette components through undoable operations.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "umicom/studio/declarative.h"
#include "umicom/studio/designer.h"

/* Exercise the complete Studio-to-Framework designer path without a GUI. */
int main(void)
{
    UmiStudioDeclarative *declarative = NULL;
    UmiStudioDesigner *designer = NULL;
    UmiDesignerWorkspaceModel model;
    UmiDesignerLiveSourceWorkspace live_source;
    UmiDeclDiagnosticList diagnostics;
    char node_id[UMI_DECL_ID_CAPACITY];

    assert(umi_studio_declarative_create(&declarative) == UMI_STATUS_OK);
    assert(umi_studio_designer_create(
        declarative,
        &designer) == UMI_STATUS_OK);
    assert(umi_studio_designer_add_palette_component(
        designer,
        "button",
        node_id,
        sizeof(node_id)) == UMI_STATUS_OK);
    assert(strcmp(node_id, "button-1") == 0);
    assert(umi_studio_designer_set_selected_property(
        designer,
        "title",
        "Run") == UMI_STATUS_OK);
    assert(umi_studio_designer_set_selected_property(
        designer,
        "visible",
        "false") == UMI_STATUS_OK);
    assert(umi_studio_designer_set_selected_property(
        designer,
        "visible",
        "not-a-boolean") != UMI_STATUS_OK);
    assert(umi_studio_designer_workspace_model(
        designer,
        "button",
        &model) == UMI_STATUS_OK);
    assert(model.document.component_count == 2U);
    assert(model.has_inspector);
    assert(model.has_inspector_schema);
    assert(strcmp(model.selected_node_id, "button-1") == 0);
    assert(model.inspector.attribute_count == 2U);
    assert(model.inspector.attributes[1].value.kind == UMI_DECL_VALUE_BOOLEAN);
    assert(model.inspector_schema.property_count > 0U);
    assert(model.palette_count > 0U);

    /* Code mode consumes the same canonical source that produced the design. */
    assert(umi_studio_designer_live_source_snapshot(
        designer,
        &live_source) == UMI_STATUS_OK);
    assert(live_source.health == UMI_DESIGNER_PREVIEW_CURRENT);
    assert(live_source.source_length > 0U);
    umi_decl_diagnostics_clear(&diagnostics);
    assert(umi_studio_designer_apply_live_source(
        designer,
        live_source.source,
        1000U,
        &diagnostics) == UMI_STATUS_OK);

    /* Invalid text reports diagnostics but leaves the last preview available. */
    umi_decl_diagnostics_clear(&diagnostics);
    assert(umi_studio_designer_apply_live_source(
        designer,
        "this is not a valid design definition",
        2000U,
        &diagnostics) != UMI_STATUS_OK);
    assert(umi_studio_designer_live_source_snapshot(
        designer,
        &live_source) == UMI_STATUS_OK);
    assert(live_source.health == UMI_DESIGNER_PREVIEW_INVALID);
    assert(live_source.preview_length > 0U);

    umi_studio_designer_destroy(designer);
    umi_studio_declarative_destroy(declarative);
    return EXIT_SUCCESS;
}
