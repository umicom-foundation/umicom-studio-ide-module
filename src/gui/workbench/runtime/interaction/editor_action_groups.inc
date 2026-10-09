/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/editor_action_groups.inc
 * PURPOSE: Group existing editor buttons without replacing their commands, lifetime bindings or automation identities.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/ui/gtk4/action_menu.h"
typedef struct RuntimeEditorActionGroup
{
    const char *id, *keywords;
    int navigation;
} RuntimeEditorActionGroup;
static const RuntimeEditorActionGroup runtime_editor_action_groups[] = {
    {"studio.editor.completion", "completion suggestions", 0},
    {"studio.editor.formatting", "format document selection indentation", 0},
    {"studio.editor.source-information", "hover documentation help", 0},
    {"studio.editor.symbols", "symbols outline project workspace", 0},
    {"studio.editor.parameters", "signature parameter help", 0},
    {"studio.editor.rename", "rename symbol workspace", 0},
    {"studio.editor.actions", "code actions quick fix refactor", 0},
    {"studio.editor.diagnostics", "problems diagnostics errors warnings", 0},
    {"studio.editor.live-diagnostics", "live diagnostics continuous language server source errors warnings", 0},
    {"studio.editor.definition", "source definition", 0},
    {"studio.editor.references", "find references usages", 0},
    {"studio.editor.type-definition", "type definition", 0},
    {"studio.editor.implementation", "implementation overrides", 0},
    {"studio.editor.syntax-selection", "syntax selection enclosing expression", 0},
    {"studio.editor.syntax-folding", "syntax folding regions language server collapse", 0},
    {"studio.editor.callers", "call hierarchy incoming callers", 0},
    {"studio.editor.callees", "call hierarchy outgoing callees", 0},
    {"studio.editor.snippet", "snippet template insertion library", 0},
    {"studio.editor.line-delete", "line editing delete current line", 0},
    {"studio.editor.line-duplicate", "line editing duplicate current line", 0},
    {"studio.editor.line-move-up", "line editing move current line up", 0},
    {"studio.editor.line-move-down", "line editing move current line down", 0},
    {"studio.editor.line-join", "line editing join next line", 0},
    {"studio.editor.line-trim", "line editing trim trailing whitespace", 0},
    {"studio.editor.line-indent", "line editing indent selected lines", 0},
    {"studio.editor.line-outdent", "line editing outdent selected lines", 0},
    {"studio.editor.line-comment", "line editing toggle // comment", 0},
    {"studio.editor.format-lf", "file format encoding line endings unix LF", 0},
    {"studio.editor.format-crlf", "file format encoding line endings windows CRLF", 0},
    {"studio.editor.format-utf8", "file format encoding line endings UTF-8", 0},
    {"studio.editor.format-utf8-bom", "file format encoding line endings UTF-8 byte order mark BOM", 0},
    {"studio.editor.format-utf16-le", "file format encoding line endings UTF-16 little endian", 0},
    {"studio.editor.format-utf16-be", "file format encoding line endings UTF-16 big endian", 0},
    {"studio.editor.match-delimiter", "matching bracket delimiter", 1},
    {"studio.editor.select-delimiter-content", "select inside bracket delimiter", 1},
    {"studio.editor.select-delimiter-pair", "select complete bracket pair", 1},
    {"studio.editor.fold-selection", "fold selected code collapse", 1},
    {"studio.editor.reveal-folds", "show all code reveal unfold expand", 1},
    {"studio.editor.bookmark-toggle", "bookmark current line toggle", 1},
    {"studio.editor.bookmark-next", "bookmark next location", 1},
    {"studio.editor.bookmark-previous", "bookmark previous location", 1}};
static void RuntimeGroupEditorActions(GtkWidget *bar, GtkWidget *after)
{
    GtkWidget *code = NULL, *navigation = NULL;
    if (UmiGtk4ActionMenuCreate("Code tools", &code) != UMI_STATUS_OK)
        return;
    if (UmiGtk4ActionMenuCreate("Navigate and view", &navigation) != UMI_STATUS_OK)
    {
        g_object_ref_sink(code);
        g_object_unref(code);
        return;
    }
    (void)umi_gtk4_automation_tag_widget(code, "studio.editor.code-tools");
    (void)umi_gtk4_automation_tag_widget(navigation, "studio.editor.navigate-tools");
    (void)umi_gtk4_automation_tag_widget(UmiGtk4ActionMenuFilterEntry(code),
                                         "studio.editor.code-tools.filter");
    (void)umi_gtk4_automation_tag_widget(UmiGtk4ActionMenuFilterEntry(navigation),
                                         "studio.editor.navigate-tools.filter");
    gtk_box_insert_child_after(GTK_BOX(bar), code, after);
    gtk_box_insert_child_after(GTK_BOX(bar), navigation, code);
    /* Construct and bind controls using the existing paths, then move only
     * these named actions into shared searchable groups. The same GtkButtons
     * retain their callbacks, weak host identity, tooltips and automation IDs.
     * Add a descriptor here when a future action belongs in one of the groups. */
    for (GtkWidget *child = gtk_widget_get_first_child(bar); child != NULL;)
    {
        GtkWidget *next = gtk_widget_get_next_sibling(child);
        const char *id = g_object_get_data(G_OBJECT(child), "umicom-automation-id");
        if (GTK_IS_BUTTON(child) && id != NULL)
        {
            for (size_t i = 0U;
                 i < sizeof(runtime_editor_action_groups) / sizeof(runtime_editor_action_groups[0]); ++i)
            {
                const RuntimeEditorActionGroup *action = &runtime_editor_action_groups[i];
                if (strcmp(id, action->id) != 0)
                    continue;
                GtkWidget *previous = gtk_widget_get_prev_sibling(child);
                if (previous != NULL)
                    g_object_ref(previous);
                g_object_ref(child);
                gtk_box_remove(GTK_BOX(bar), child);
                UmiStatus status =
                    UmiGtk4ActionMenuAppend(action->navigation ? navigation : code, child, action->keywords);
                /* If adoption fails before parenting, retain the action in
                 * its original toolbar position so no command disappears. */
                if (status != UMI_STATUS_OK && gtk_widget_get_parent(child) == NULL)
                {
                    if (previous != NULL && gtk_widget_get_parent(previous) == bar)
                        gtk_box_insert_child_after(GTK_BOX(bar), child, previous);
                    else
                        gtk_box_append(GTK_BOX(bar), child);
                }
                g_clear_object(&previous);
                g_object_unref(child);
                break;
            }
        }
        child = next;
    }
}
