/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_keyboard.inc
 *
 * PURPOSE:
 *   Provide familiar IDE keyboard entry points for command search and layouts.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static gboolean runtime_on_key_pressed(
    GtkEventControllerKey *controller,
    guint keyval,
    guint keycode,
    GdkModifierType state,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    size_t index;

    (void)controller;
    (void)keycode;

    if (runtime == NULL) return FALSE;

    for (index = 0U;
         index < sizeof(RUNTIME_KEY_PROFILES) /
                 sizeof(RUNTIME_KEY_PROFILES[0]);
         ++index) {
        const UmiStudioGtkKeyProfile *profile = &RUNTIME_KEY_PROFILES[index];
        const GdkModifierType normalized =
            state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_ALT_MASK);

        if (keyval != profile->key_lower && keyval != profile->key_upper) {
            continue;
        }
        if (normalized != profile->modifiers) continue;

        if (strcmp(profile->command_id, "studio.command.search") == 0) {
            runtime_show_command_palette(runtime);
            return TRUE;
        }
        if (strcmp(profile->command_id, "studio.layout.manager") == 0) {
            runtime_show_layout_browser(runtime);
            return TRUE;
        }

        if (runtime_command_enabled(runtime, profile->command_id)) {
            (void)runtime_execute_command(
                runtime,
                profile->command_id,
                NULL);
            return TRUE;
        }
    }

    return FALSE;
}

static UmiStatus runtime_install_keyboard(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkEventController *controller;

    if (runtime == NULL || runtime->owner == NULL ||
        runtime->owner->window == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    controller = gtk_event_controller_key_new();
    if (controller == NULL) return UMI_STATUS_OUT_OF_MEMORY;

    g_signal_connect(
        controller,
        "key-pressed",
        G_CALLBACK(runtime_on_key_pressed),
        runtime);
    gtk_widget_add_controller(
        GTK_WIDGET(runtime->owner->window),
        controller);

    return UMI_STATUS_OK;
}
