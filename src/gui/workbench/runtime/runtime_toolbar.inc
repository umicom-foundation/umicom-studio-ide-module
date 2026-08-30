/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_toolbar.inc
 *
 * PURPOSE:
 *   Build a command-registry-backed professional developer toolbar. Buttons are
 *   sensitive only when their real Framework command exists and is enabled.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static GtkWidget *runtime_build_toolbar(UmiStudioGtkRuntimeChrome *runtime)
{
    static const char *const PROFILE_IDS[] = {
        "new", "open", "save", "build", "run",
        "test_all", "debug_start", "vcs_view", "ai"
    };
    GtkWidget *bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
    size_t index;

    if (bar == NULL || runtime == NULL) return bar;

    gtk_widget_add_css_class(bar, "umicom-runtime-toolbar");

    for (index = 0U;
         index < sizeof(PROFILE_IDS) / sizeof(PROFILE_IDS[0]);
         ++index) {
        const UmiStudioGtkCommandProfile *profile =
            runtime_command_profile(PROFILE_IDS[index]);
        GtkWidget *button;

        if (profile == NULL) continue;

        button = runtime_make_icon_command_button(
            runtime,
            profile->icon_name,
            profile->command_id,
            profile->tooltip);

        if (button != NULL) gtk_box_append(GTK_BOX(bar), button);
    }

    return bar;
}
