/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/visual_builder_templates.c
 *
 * PURPOSE:
 *   Coordinate Studio visual-builder templates through the Framework API.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/visual_builder_templates.h"

#include <stdlib.h>

/*
 * Find studio visual builder template while leaving the underlying catalogue or model
 * owned by this module.
 */
UmiStatus umi_studio_visual_builder_template_find(
    UmiStudioVisualBuilderCentre *centre,
    const char *template_id,
    UmiDesignerTemplate *out_template)
{
    UmiDesignerBuilderSession *session =
        umi_studio_visual_builder_centre_session(centre);
    return session != NULL
        ? umi_designer_template_registry_find(
              umi_designer_builder_session_templates(session),
              template_id,
              out_template)
        : UMI_STATUS_INVALID_ARGUMENT;
}

/*
 * Perform studio visual builder template through the module contract so client
 * applications do not duplicate its policy.
 */
UmiStatus umi_studio_visual_builder_template_apply(
    UmiStudioVisualBuilderCentre *centre,
    const char *template_id,
    const char *parent_id,
    const char *id_prefix)
{
    UmiDesignerBuilderSession *session =
        umi_studio_visual_builder_centre_session(centre);
    UmiDesignerTemplate *item;
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (session == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    item = (UmiDesignerTemplate *)malloc(sizeof(*item));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (item == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    status = umi_designer_template_registry_find(
        umi_designer_builder_session_templates(session),
        template_id,
        item);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_designer_template_instantiate(
            item,
            umi_designer_builder_session_transactions(session),
            parent_id,
            id_prefix);
    }
    free(item);
    return status;
}

/*
 * Return the number of records represented by studio visual builder template without
 * changing their state.
 */
size_t umi_studio_visual_builder_template_count(
    UmiStudioVisualBuilderCentre *centre)
{
    UmiDesignerBuilderSession *session =
        umi_studio_visual_builder_centre_session(centre);
    return session != NULL
        ? umi_designer_template_registry_count(
              umi_designer_builder_session_templates(session))
        : 0U;
}
