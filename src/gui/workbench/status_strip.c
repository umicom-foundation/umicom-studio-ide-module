/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/status_strip.c
 *
 * PURPOSE:
 *   Render visible Framework status items as a GTK4 horizontal status strip.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "status_strip.h"

/*
 * Provide the studio status strip new operation used by this module and its client
 * applications.
 */
GtkWidget *umi_studio_status_strip_new(UmiStudioUi *ui)
{
    GtkWidget *box;
    UmiUiStatusSnapshot item;
    UmiUiStatusModel *model;
    size_t index;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (ui == NULL) return NULL;
    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    model = umi_ui_workbench_status(umi_studio_ui_workbench(ui));
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < umi_ui_status_model_count(model); ++index) {
        GtkWidget *label;
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (umi_ui_status_model_at(model, index, &item) != UMI_STATUS_OK || !item.visible) continue;
        label = gtk_label_new(item.text);
        gtk_widget_set_tooltip_text(label, item.tooltip);
        gtk_box_append(GTK_BOX(box), label);
    }
    return box;
}
