/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/editor_document_format.inc
 * PURPOSE: Expose shared document-format choices without adding application-owned save or Undo logic.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

static void RuntimeDocumentFormat(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    const UmiDocumentFormatOptions *options = g_object_get_data(G_OBJECT(button), "document-format-options");
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL)
    {
        runtime_show_notification(runtime, "Could not retain the editor window.");
        return;
    }
    int reports = UmiGtk4AdapterDocumentHasCompletion(runtime->owner->adapter);
    UmiStatus status = UmiGtk4AdapterSetDocumentFormat(runtime->owner->adapter, options);
    /* Format changes can refresh the native editor. Resolve its weak window
     * again before using application state after the shared completion. */
    GObject *window = NULL;
    runtime = runtime_file_dialog_owner(request, &window);
    if (runtime != NULL && status != UMI_STATUS_OK && !reports)
        runtime_show_notification(runtime, "The file format could not be changed. Keep the draft open and "
                                           "check that it is writable and within the supported text size.");
    g_clear_object(&window);
    runtime_file_dialog_request_free(request);
}
static void RuntimeAppendDocumentFormats(UmiStudioGtkRuntimeChrome *runtime, GtkWidget *bar)
{
    static const struct
    {
        const char *id, *label, *help;
        UmiDocumentFormatOptions options;
    } actions[] = {
        {"format-lf",
         "Use LF line endings",
         "Convert line endings and use LF on the next Save. Undo restores the previous choice.",
         {UMI_DOCUMENT_ENCODING_UNKNOWN, UMI_DOCUMENT_LINE_ENDING_LF, 0}},
        {"format-crlf",
         "Use CRLF line endings",
         "Convert line endings and use CRLF on the next Save. Undo restores the previous choice.",
         {UMI_DOCUMENT_ENCODING_UNKNOWN, UMI_DOCUMENT_LINE_ENDING_CRLF, 0}},
        {"format-utf8",
         "Use UTF-8 for Save",
         "Choose UTF-8 without a byte-order mark for the next Save. Source stays open and Undo restores the "
         "choice.",
         {UMI_DOCUMENT_ENCODING_UTF8, UMI_DOCUMENT_LINE_ENDING_NONE, 0}},
        {"format-utf8-bom",
         "Use UTF-8 with BOM for Save",
         "Choose UTF-8 with a byte-order mark for the next Save. This command does not write the file.",
         {UMI_DOCUMENT_ENCODING_UTF8_BOM, UMI_DOCUMENT_LINE_ENDING_NONE, 0}},
        {"format-utf16-le",
         "Use UTF-16 LE for Save",
         "Choose little-endian UTF-16 with a byte-order mark for the next Save. Confirm your consuming tool "
         "supports it.",
         {UMI_DOCUMENT_ENCODING_UTF16_LE, UMI_DOCUMENT_LINE_ENDING_NONE, 0}},
        {"format-utf16-be",
         "Use UTF-16 BE for Save",
         "Choose big-endian UTF-16 with a byte-order mark for the next Save. Confirm your consuming tool "
         "supports it.",
         {UMI_DOCUMENT_ENCODING_UTF16_BE, UMI_DOCUMENT_LINE_ENDING_NONE, 0}}};
    /* Descriptors select Framework policy. They contain no persistence paths,
     * codecs or independent text state; contributors extend the shared owner. */
    for (size_t i = 0U; i < sizeof(actions) / sizeof(actions[0]); ++i)
    {
        GtkWidget *button = gtk_button_new_with_label(actions[i].label);
        char id[128];
        (void)snprintf(id, sizeof(id), "studio.editor.%s", actions[i].id);
        (void)umi_gtk4_automation_tag_widget(button, id);
        g_object_set_data(G_OBJECT(button), "document-format-options", (gpointer)&actions[i].options);
        gtk_widget_set_tooltip_text(button, actions[i].help);
        runtime_bind_editor_action(runtime, button, RuntimeDocumentFormat);
        gtk_box_append(GTK_BOX(bar), button);
    }
}
