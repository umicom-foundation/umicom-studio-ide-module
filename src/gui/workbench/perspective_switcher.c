/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/perspective_switcher.c
 *
 * PURPOSE:
 *   Render the Framework perspective model as GTK4 buttons that activate the
 *   shared Studio workbench state.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "perspective_switcher.h"

#include <stdlib.h>

typedef struct PerspectiveButtonContext {
    UmiStudioUi *ui;
    char perspective_id[UMI_UI_ID_CAPACITY];
} PerspectiveButtonContext;

/*
 * Provide the activate perspective operation used by this module and its client
 * applications.
 */
static void activate_perspective(GtkButton *button, gpointer user_data)
{
    PerspectiveButtonContext *context = (PerspectiveButtonContext *)user_data;
    (void)button;
    (void)umi_ui_workbench_activate_perspective(
        umi_studio_ui_workbench(context->ui), context->perspective_id);
}

/* Provide the free context operation used by this module and its client applications. */
static void free_context(gpointer data, GClosure *closure)
{
    (void)closure;
    free(data);
}

/*
 * Provide the studio perspective switcher new operation used by this module and its client
 * applications.
 */
GtkWidget *umi_studio_perspective_switcher_new(UmiStudioUi *ui)
{
    GtkWidget *box;
    UmiUiPerspectiveSnapshot item;
    size_t index;
    UmiUiPerspectiveModel *model;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (ui == NULL) return NULL;
    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    model = umi_ui_workbench_perspectives(umi_studio_ui_workbench(ui));
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < umi_ui_perspective_model_count(model); ++index) {
        GtkWidget *button;
        PerspectiveButtonContext *context;
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (umi_ui_perspective_model_at(model, index, &item) != UMI_STATUS_OK) continue;
        button = gtk_button_new_with_label(item.title);
        context = (PerspectiveButtonContext *)calloc(1U, sizeof(*context));
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (context == NULL) continue;
        context->ui = ui;
        (void)g_strlcpy(context->perspective_id, item.perspective_id, sizeof(context->perspective_id));
        g_signal_connect_data(button, "clicked", G_CALLBACK(activate_perspective), context,
                              free_context, 0);
        gtk_box_append(GTK_BOX(box), button);
    }
    return box;
}
