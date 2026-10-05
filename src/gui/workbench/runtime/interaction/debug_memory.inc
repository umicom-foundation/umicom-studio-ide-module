/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/debug_memory.inc
 * PURPOSE: Display explicit paused-memory captures with editable offsets and a fixed byte budget.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

/* Parse decimal offsets without locale, floating-point rounding or accepted
 * trailing text. The bound is the protocol's exact JSON integer range. */
static int RuntimeMemoryOffset(const char *text, int64_t *out)
{
    int negative = *text == '-';
    const unsigned char *cursor = (const unsigned char *)text + (negative ? 1 : 0);
    if (*cursor == 0)
        return 0;
    uint64_t value = 0U;
    for (; *cursor; ++cursor)
    {
        if (*cursor < '0' || *cursor > '9')
            return 0;
        uint64_t digit = (uint64_t)(*cursor - '0');
        if (value > ((uint64_t)UMI_DEBUG_MEMORY_OFFSET_LIMIT - digit) / 10U)
            return 0;
        value = value * 10U + digit;
    }
    *out = negative ? -(int64_t)value : (int64_t)value;
    return 1;
}
static void RuntimeMemoryRead(GtkButton *button, gpointer data)
{
    UmiStudioGtkRuntimeChrome *runtime = data;
    RuntimeVariableRow *row = g_object_get_data(G_OBJECT(button), "umicom-variable-row");
    if (row == NULL || row->readingMemory)
        return;
    GtkWidget *owner = g_object_ref(row->owner);
    GtkLabel *statusLabel = g_object_ref(row->memoryStatus);
    GtkTextBuffer *buffer = g_object_ref(gtk_text_view_get_buffer(row->memoryOutput));
    row->readingMemory = 1;
    int64_t offset = 0;
    int64_t requested = 0;
    /* Read the visible draft rather than an older spin-button numeric value.
     * Invalid text must not silently reuse or clamp the previous byte count. */
    UmiStatus status =
        RuntimeMemoryOffset(gtk_editable_get_text(GTK_EDITABLE(row->memoryOffset)), &offset) &&
                RuntimeMemoryOffset(gtk_editable_get_text(GTK_EDITABLE(row->memoryCount)), &requested) &&
                requested >= 1 && requested <= UMI_DEBUG_MEMORY_CAPTURE_CAPACITY
            ? UMI_STATUS_OK
            : UMI_STATUS_INVALID_ARGUMENT;
    uint32_t count = status == UMI_STATUS_OK ? (uint32_t)requested : 0U;
    UmiDebugMemoryCapture *capture = NULL;
    char *text = NULL;
    if (status == UMI_STATUS_OK)
    {
        UmiStudioDebuggerService *service =
            umi_studio_services_debugger(umi_studio_ui_services(runtime->owner->ui));
        status = UmiStudioDebuggerInspectMemory(service, row->target, offset, count, &capture);
    }
    if (status == UMI_STATUS_OK)
    {
        size_t needed = 0U;
        status = UmiDebugMemoryCaptureFormat(capture, NULL, 0U, &needed);
        if (status == UMI_STATUS_CAPACITY_EXCEEDED && needed > 0U)
        {
            text = g_try_malloc(needed);
            status = text != NULL ? UmiDebugMemoryCaptureFormat(capture, text, needed, &needed)
                                  : UMI_STATUS_OUT_OF_MEMORY;
        }
    }
    UmiDebugMemoryCaptureDestroy(capture);
    /* Updating either control can notify an accessibility observer. Nothing
     * below borrows the runtime; retained controls and the row own all storage. */
    if (status == UMI_STATUS_OK)
    {
        gtk_label_set_text(
            statusLabel,
            "Captured bytes. Read memory again to refresh; changing fields alone does not read.");
        gtk_text_buffer_set_text(buffer, text, -1);
    }
    else
    {
        char message[384];
        (void)snprintf(message, sizeof message,
                       "Memory was not replaced: %s. Earlier output remains an older capture. Use a decimal "
                       "offset, or inspect the current scope again.",
                       umi_status_text(status));
        gtk_label_set_text(statusLabel, message);
    }
    g_free(text);
    row->readingMemory = 0;
    g_object_unref(buffer);
    g_object_unref(statusLabel);
    g_object_unref(owner);
}
static void RuntimeMemoryControls(UmiStudioGtkRuntimeChrome *runtime, RuntimeVariableRow *row,
                                  const UmiDebugRuntimeVariable *value, GtkWidget *box)
{
    /* Legacy root records have no memory reference. Explicit scope/child reads
     * retain it, including scalar values with no expandable child reference. */
    if (row->target == NULL || value->memory_reference[0] == '\0')
        return;
    GtkWidget *expander = gtk_expander_new("Memory bytes");
    GtkWidget *panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *offset = gtk_entry_new();
    row->memoryOffset = GTK_ENTRY(offset);
    gtk_editable_set_text(GTK_EDITABLE(offset), "0");
    gtk_entry_set_max_length(row->memoryOffset, 32);
    gtk_accessible_update_property(GTK_ACCESSIBLE(offset), GTK_ACCESSIBLE_PROPERTY_LABEL,
                                   "Memory offset in bytes", -1);
    gtk_widget_set_tooltip_text(
        offset,
        "Signed decimal byte offset from the adapter's variable reference. Negative offsets are allowed.");
    GtkWidget *count = gtk_spin_button_new_with_range(1, UMI_DEBUG_MEMORY_CAPTURE_CAPACITY, 1);
    row->memoryCount = GTK_SPIN_BUTTON(count);
    gtk_spin_button_set_value(row->memoryCount, 256);
    gtk_spin_button_set_numeric(row->memoryCount, TRUE);
    gtk_accessible_update_property(GTK_ACCESSIBLE(count), GTK_ACCESSIBLE_PROPERTY_LABEL,
                                   "Number of memory bytes", -1);
    GtkWidget *button = gtk_button_new_with_label("Read memory");
    GtkWidget *output = gtk_text_view_new();
    row->memoryOutput = GTK_TEXT_VIEW(output);
    gtk_text_view_set_editable(row->memoryOutput, FALSE);
    gtk_text_view_set_monospace(row->memoryOutput, TRUE);
    gtk_accessible_update_property(GTK_ACCESSIBLE(output), GTK_ACCESSIBLE_PROPERTY_LABEL,
                                   "Captured memory hex and ASCII", -1);
    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_widget_set_size_request(scroll, -1, 180);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), output);
    GtkWidget *label =
        runtime_label("Choose an offset and byte count, then read once. No memory is written.", 0.0F);
    row->memoryStatus = GTK_LABEL(label);
    gtk_label_set_wrap(row->memoryStatus, TRUE);
    RuntimeVariableTag(offset, "memory-offset", row->path);
    RuntimeVariableTag(count, "memory-count", row->path);
    RuntimeVariableTag(button, "memory-read", row->path);
    RuntimeVariableTag(output, "memory-output", row->path);
    RuntimeVariableTag(label, "memory-status", row->path);
    UmiStudioDebuggerService *service =
        umi_studio_services_debugger(umi_studio_ui_services(runtime->owner->ui));
    UmiStatus available = UmiStudioDebuggerCheckMemory(service, row->target);
    gtk_widget_set_sensitive(button, available == UMI_STATUS_OK);
    gtk_widget_set_tooltip_text(button, available == UMI_STATUS_NOT_IMPLEMENTED
                                            ? "This adapter does not advertise memory reads."
                                            : "Read up to 4096 bytes once at this unchanged stop. The "
                                              "adapter request can take up to 1.5 seconds.");
    g_object_set_data(G_OBJECT(button), "umicom-variable-row", row);
    runtime_bind_editor_action(runtime, button, RuntimeMemoryRead);
    gtk_box_append(GTK_BOX(panel), runtime_label("Offset in bytes", 0.0F));
    gtk_box_append(GTK_BOX(panel), offset);
    gtk_box_append(GTK_BOX(panel), runtime_label("Number of bytes", 0.0F));
    gtk_box_append(GTK_BOX(panel), count);
    gtk_box_append(GTK_BOX(panel), button);
    gtk_box_append(GTK_BOX(panel), label);
    gtk_box_append(GTK_BOX(panel), scroll);
    gtk_expander_set_child(GTK_EXPANDER(expander), panel);
    gtk_box_append(GTK_BOX(box), expander);
}
