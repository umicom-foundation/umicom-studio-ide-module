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

/* Assertions are the executable test contract, including in Release builds. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "umicom/studio/declarative.h"
#include "umicom/studio/designer.h"

/* Resolve semantic properties by identity; geometry precedes edited properties
 * and future schemas may add more attributes without changing their meaning. */
static const UmiDeclAttribute *FindAttribute(
    const UmiDesignerInspectorSnapshot *inspector, const char *name)
{
    const UmiDeclAttribute *found = NULL;
    assert(inspector != NULL && name != NULL);
    assert(inspector->attribute_count <=
        sizeof(inspector->attributes) / sizeof(inspector->attributes[0]));
    for (size_t index = 0U; index < inspector->attribute_count; ++index) {
        if (strcmp(inspector->attributes[index].name, name) == 0) {
            assert(found == NULL); /* A property may appear only once. */
            found = &inspector->attributes[index];
        }
    }
    assert(found != NULL);
    return found;
}

/* Check values as well as kinds so rejecting an invalid edit cannot silently
 * reset a valid boolean, lose a title, or overwrite palette placement. */
static void CheckEditedProperties(const UmiDesignerWorkspaceModel *model,
                                  const int64_t *geometry)
{
    static const char *const geometryNames[] = {"x", "y", "width", "height"};
    const UmiDeclAttribute *attribute = FindAttribute(&model->inspector, "title");
    assert(attribute->value.kind == UMI_DECL_VALUE_STRING);
    assert(strcmp(attribute->value.text, "Run") == 0);
    attribute = FindAttribute(&model->inspector, "visible");
    assert(attribute->value.kind == UMI_DECL_VALUE_BOOLEAN);
    assert(attribute->value.boolean_value == 0);
    for (size_t index = 0U; index < 4U; ++index) {
        attribute = FindAttribute(&model->inspector, geometryNames[index]);
        assert(attribute->value.kind == UMI_DECL_VALUE_INTEGER);
        assert(attribute->value.integer_value == geometry[index]);
    }
}

/* Exercise the complete Studio-to-Framework designer path without a GUI. */
int main(void)
{
    UmiStudioDeclarative *declarative = NULL;
    UmiStudioDesigner *designer = NULL;
    UmiDesignerWorkspaceModel *model;
    UmiDesignerLiveSourceWorkspace live_source;
    UmiDeclDiagnosticList diagnostics;
    char node_id[UMI_DECL_ID_CAPACITY];
    static const char *const geometryNames[] = {"x", "y", "width", "height"};
    int64_t initialGeometry[4];

    /* The synchronized model includes bounded palette, hierarchy and
     * inspector arrays. Heap storage keeps the test independent of stack size. */
    model = (UmiDesignerWorkspaceModel *)calloc(1U, sizeof(*model));
    assert(model != NULL);

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
    assert(umi_studio_designer_workspace_model(designer, "button", model) == UMI_STATUS_OK);
    assert(model->has_inspector);
    for (size_t index = 0U; index < 4U; ++index) {
        const UmiDeclAttribute *attribute = FindAttribute(&model->inspector, geometryNames[index]);
        assert(attribute->value.kind == UMI_DECL_VALUE_INTEGER);
        initialGeometry[index] = attribute->value.integer_value;
    }
    assert(initialGeometry[0] >= 0 && initialGeometry[1] >= 0);
    assert(initialGeometry[2] > 0 && initialGeometry[3] > 0);
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
        model) == UMI_STATUS_OK);
    assert(model->document.component_count == 2U);
    assert(model->has_inspector);
    assert(model->has_inspector_schema);
    assert(strcmp(model->selected_node_id, "button-1") == 0);
    CheckEditedProperties(model, initialGeometry);
    assert(model->inspector_schema.property_count > 0U);
    assert(model->palette_count > 0U);

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
    free(model);
    return EXIT_SUCCESS;
}
