/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/designer/tree_view.c
 *
 * PURPOSE:
 *   Render the semantic component hierarchy as a simple GTK4 list for the
 *   reference visual designer.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This file is a presentation adapter. GTK widgets are created here, while
 * authoritative application/designer state remains in the shared Framework.
 */
#include "tree_view.h"

GtkWidget *umi_studio_designer_tree_view_new(
    UmiStudioDesigner *designer)
{
    GtkWidget *scroll = gtk_scrolled_window_new();
    GtkWidget *list = gtk_list_box_new();
    UmiDeclDocument *document;
    size_t index;
    size_t count;

    if (designer == NULL) {
        return scroll;
    }

    document = umi_designer_document_declarative(
        umi_studio_designer_document(designer));
    count = umi_decl_document_node_count(document);

    for (index = 0U; index < count; ++index) {
        UmiDeclNode node;
        gchar *label_text;
        GtkWidget *row;

        if (umi_decl_document_node_at(
                document, index, &node) != UMI_STATUS_OK) {
            continue;
        }

        /*
         * Node identifiers are Framework-owned bounded strings. Dynamic GTK
         * formatting preserves their complete value and avoids an arbitrary
         * presentation buffer that could truncate a valid component name.
         */
        label_text = g_strdup_printf(
            "%s  [%s]",
            node.node_id,
            node.component_type);
        if (label_text == NULL) {
            continue;
        }

        row = gtk_label_new(label_text);
        g_free(label_text);

        gtk_label_set_xalign(GTK_LABEL(row), 0.0F);
        gtk_list_box_append(GTK_LIST_BOX(list), row);
    }

    gtk_scrolled_window_set_child(
        GTK_SCROLLED_WINDOW(scroll),
        list);
    return scroll;
}
