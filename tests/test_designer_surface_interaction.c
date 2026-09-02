/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_designer_surface_interaction.c
 *
 * PURPOSE:
 *   Verify that Studio composes Framework hit-testing, direct manipulation,
 *   source synchronization and undo without placing geometry policy in GTK.
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

/* Exercise the thin Studio facade with no desktop window or event loop. */
int main(void)
{
    UmiStudioDeclarative *declarative = NULL;
    UmiStudioDesigner *designer = NULL;
    UmiDesignerSurfaceOptions options;
    UmiDesignerSurfaceInteraction interaction;
    UmiDesignerLiveSourceWorkspace before_source;
    UmiDesignerLiveSourceWorkspace after_source;
    UmiDesignerRect start = {16, 24, 96, 40};
    UmiDesignerRect moved;
    char node_id[UMI_DECL_ID_CAPACITY];

    assert(umi_studio_declarative_create(&declarative) == UMI_STATUS_OK);
    assert(umi_studio_designer_create(declarative, &designer) ==
        UMI_STATUS_OK);
    assert(umi_studio_designer_add_palette_component(
        designer,
        "button",
        node_id,
        sizeof(node_id)) == UMI_STATUS_OK);
    assert(umi_designer_surface_set_rect(
        umi_studio_designer_document(designer),
        node_id,
        start) == UMI_STATUS_OK);
    umi_designer_surface_options_init(&options);
    assert(umi_studio_designer_live_source_snapshot(
        designer,
        &before_source) == UMI_STATUS_OK);

    /* An interior press chooses Move, and the release synchronizes generated
     * source only after Framework accepts the history operation. */
    assert(umi_studio_designer_begin_surface_interaction(
        designer,
        48,
        44,
        400,
        300,
        &options,
        &interaction) == UMI_STATUS_OK);
    assert(interaction.kind == UMI_DESIGNER_SURFACE_INTERACTION_MOVE);
    assert(strcmp(interaction.node_id, node_id) == 0);
    assert(umi_studio_designer_update_surface_interaction(
        &interaction,
        72,
        60) == UMI_STATUS_OK);
    assert(umi_studio_designer_commit_surface_interaction(
        designer,
        &interaction) == UMI_STATUS_OK);
    assert(umi_designer_surface_get_rect(
        umi_studio_designer_document(designer),
        node_id,
        &moved) == UMI_STATUS_OK);
    assert(moved.x == 40 && moved.y == 40);
    assert(umi_studio_designer_live_source_snapshot(
        designer,
        &after_source) == UMI_STATUS_OK);
    assert(after_source.source_revision > before_source.source_revision);
    assert(strstr(after_source.source, "property button-1 x 40") != NULL);

    /* Studio's normal Undo command reverses the same gesture because the GTK
     * adapter did not mutate the semantic node directly. */
    assert(umi_studio_designer_undo(designer) == UMI_STATUS_OK);
    assert(umi_designer_surface_get_rect(
        umi_studio_designer_document(designer),
        node_id,
        &moved) == UMI_STATUS_OK);
    assert(moved.x == start.x && moved.y == start.y);

    umi_studio_designer_destroy(designer);
    umi_studio_declarative_destroy(declarative);
    return EXIT_SUCCESS;
}
