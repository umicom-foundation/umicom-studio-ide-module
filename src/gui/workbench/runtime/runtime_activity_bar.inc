/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_activity_bar.inc
 *
 * PURPOSE:
 *   Create a familiar application activity rail backed by real
 *   Framework commands.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static GtkWidget *runtime_build_activity_bar(
    UmiStudioGtkRuntimeChrome *runtime)
{
    static const UmiStudioRuntimeSurfaceKind KINDS[] = {
        UMI_STUDIO_SURFACE_EXPLORER,
        UMI_STUDIO_SURFACE_SEARCH,
        UMI_STUDIO_SURFACE_SOURCE_CONTROL,
        UMI_STUDIO_SURFACE_DEBUG,
        UMI_STUDIO_SURFACE_TEST_EXPLORER,
        UMI_STUDIO_SURFACE_AI_CHAT
    };
    GtkWidget *bar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    size_t index;

    if (bar == NULL || runtime == NULL) return bar;

    gtk_widget_add_css_class(bar, "umicom-runtime-activity");

    for (index = 0U; index < sizeof(KINDS) / sizeof(KINDS[0]); ++index) {
        const UmiStudioGtkSurfaceProfile *profile =
            runtime_surface_profile(KINDS[index]);
        GtkWidget *button;

        if (profile == NULL) continue;

        button = runtime_make_icon_command_button(
            runtime,
            profile->icon_name,
            profile->command_id,
            profile->title);

        if (button == NULL) continue;

        runtime->activity_buttons[profile->kind] = button;
        gtk_box_append(GTK_BOX(bar), button);
    }

    return bar;
}

static void runtime_update_activity_selection(
    UmiStudioGtkRuntimeChrome *runtime,
    UmiStudioRuntimeSurfaceKind active)
{
    int kind;

    if (runtime == NULL) return;

    for (kind = (int)UMI_STUDIO_SURFACE_EXPLORER;
         kind <= (int)UMI_STUDIO_SURFACE_LAST;
         ++kind) {
        GtkWidget *button = runtime->activity_buttons[kind];

        if (button == NULL) continue;

        if (kind == (int)active) {
            gtk_widget_add_css_class(button, "suggested-action");
        } else {
            gtk_widget_remove_css_class(button, "suggested-action");
        }
    }
}
