/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/search.inc
 *
 * PURPOSE:
 *   Build the Framework-backed search presentation surface.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

static GtkWidget *runtime_build_surface_search(UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Search");
    GtkWidget *button;

    if (panel == NULL || runtime == NULL) return panel;

    gtk_box_append(
        GTK_BOX(panel),
        runtime_card_new(
            "Search Everywhere",
            "Command/menu search is backed by UmiCommandRegistry and the "
            "Framework Application Shell. Use Ctrl+Shift+P or the button below."));

    button = runtime_make_local_button(
        runtime,
        "Open Command Search",
        "Search commands and menu contributions",
        0);
    if (button != NULL) gtk_box_append(GTK_BOX(panel), button);

    return runtime_scroll(panel);
}

