/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/editor_line_edits.inc
 * PURPOSE: Expose shared line transformations through the document coordinator and the existing weak editor binding.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

static void RuntimeLineEdit(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    UmiEditorEditCommandKind kind = (UmiEditorEditCommandKind)GPOINTER_TO_INT(
        g_object_get_data(G_OBJECT(button), "document-line-edit-kind"));
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL)
    {
        runtime_show_notification(runtime, "Could not retain the editor window.");
        return;
    }
    int reports = UmiGtk4AdapterDocumentHasCompletion(runtime->owner->adapter);
    UmiStatus status = UmiGtk4AdapterEditLines(runtime->owner->adapter, kind, NULL);
    /* Applying a draft can refresh or close its native host. Resolve the weak
     * window identity again before displaying a result in Studio. */
    GObject *window = NULL;
    runtime = runtime_file_dialog_owner(request, &window);
    if (runtime != NULL && status != UMI_STATUS_OK && !reports)
        runtime_show_notification(runtime,
                                  "The line command could not be applied. Use a writable UTF-8 draft with LF "
                                  "or CRLF line endings; moving or joining needs a neighboring line.");
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
}
static void RuntimeAppendLineEdits(UmiStudioGtkRuntimeChrome *runtime, GtkWidget *bar)
{
    static const struct
    {
        const char *id, *label, *help;
        UmiEditorEditCommandKind kind;
    } actions[] = {
        {"studio.editor.line-delete", "Delete current line",
         "Remove the caret line. Undo restores it; Save is separate.", UMI_EDITOR_EDIT_COMMAND_DELETE_LINE},
        {"studio.editor.line-duplicate", "Duplicate current line",
         "Copy the caret line and move to the copy. A selection does not expand this command.",
         UMI_EDITOR_EDIT_COMMAND_DUPLICATE_LINE},
        {"studio.editor.line-move-up", "Move current line up",
         "Move the caret line above its neighbor and keep the caret column.",
         UMI_EDITOR_EDIT_COMMAND_MOVE_LINE_UP},
        {"studio.editor.line-move-down", "Move current line down",
         "Move the caret line below its neighbor and keep the caret column.",
         UMI_EDITOR_EDIT_COMMAND_MOVE_LINE_DOWN},
        {"studio.editor.line-join", "Join next line",
         "Join the caret line with the next line, removing its leading indentation.",
         UMI_EDITOR_EDIT_COMMAND_JOIN_LINE_WITH_NEXT},
        {"studio.editor.line-trim", "Trim trailing whitespace",
         "Remove spaces and tabs at line ends throughout the draft. The caret returns to the start.",
         UMI_EDITOR_EDIT_COMMAND_TRIM_TRAILING_WHITESPACE},
        {"studio.editor.line-indent", "Indent selected lines",
         "Add four spaces to each selected line, or to the caret line when nothing is selected.",
         UMI_EDITOR_EDIT_COMMAND_INDENT_LINES},
        {"studio.editor.line-outdent", "Outdent selected lines",
         "Remove one leading tab or up to four spaces from each selected line.",
         UMI_EDITOR_EDIT_COMMAND_OUTDENT_LINES},
        {"studio.editor.line-comment", "Toggle // comment",
         "Add or remove the literal // prefix on selected nonblank lines. This does not infer the "
         "programming language.",
         UMI_EDITOR_EDIT_COMMAND_TOGGLE_LINE_COMMENT}};
    /* Keep the application layer declarative. Framework prepares complete
     * edits and owns publication, selection policy and Undo. Add another
     * descriptor only after its shared document behavior is available. */
    for (size_t i = 0U; i < sizeof(actions) / sizeof(actions[0]); ++i)
    {
        GtkWidget *button = gtk_button_new_with_label(actions[i].label);
        (void)umi_gtk4_automation_tag_widget(button, actions[i].id);
        g_object_set_data(G_OBJECT(button), "document-line-edit-kind", GINT_TO_POINTER((int)actions[i].kind));
        gtk_widget_set_tooltip_text(button, actions[i].help);
        runtime_bind_editor_action(runtime, button, RuntimeLineEdit);
        gtk_box_append(GTK_BOX(bar), button);
    }
}
