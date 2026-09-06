/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_window_catalogue.inc
 *
 * PURPOSE:
 *   Present Studio's real Framework-backed tool surfaces as a searchable New
 *   Window catalogue and let an unlocked workspace move those surfaces between
 *   named dock regions or detached native windows without recreating business
 *   logic inside the catalogue.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* Readable destinations are presentation metadata, not a second layout. */
static const char *runtime_region_text(UmiStudioGtkRuntimeRegion region)
{
    switch (region) {
    case UMI_STUDIO_GTK_REGION_PRIMARY: return "Left";
    case UMI_STUDIO_GTK_REGION_SECONDARY: return "Right";
    case UMI_STUDIO_GTK_REGION_BOTTOM: return "Bottom";
    case UMI_STUDIO_GTK_REGION_CANVAS: return "Canvas";
    default: return "Centre";
    }
}

/* Explicit catalogue placement participates in the current Framework edit. */
static UmiStatus runtime_surface_place(UmiStudioGtkRuntimeChrome *runtime,
    UmiStudioRuntimeSurfaceKind kind, UmiStudioGtkRuntimeRegion region)
{
    const char *placement = region == UMI_STUDIO_GTK_REGION_PRIMARY ? "left" :
        region == UMI_STUDIO_GTK_REGION_SECONDARY ? "right" :
        region == UMI_STUDIO_GTK_REGION_BOTTOM ? "bottom" :
        region == UMI_STUDIO_GTK_REGION_CANVAS ? "canvas" : "centre";
    return runtime_workspace_open_surface(runtime, kind, placement);
}

/* Hiding is an explicit arrangement edit; View commands may reopen a singleton. */
static UmiStatus runtime_surface_hide(UmiStudioGtkRuntimeChrome *runtime,
    UmiStudioRuntimeSurfaceKind kind)
{
    UmiUiWorkspaceCustomisation *model = runtime_workspace_model(runtime);
    UmiStatus status;
    if (model == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    if (!model->edit_active) return UMI_STATUS_PERMISSION_DENIED;
    status = umi_studio_runtime_workspace_set_visible(model, kind, false);
    return status == UMI_STATUS_OK ? runtime_workspace_rebuild(runtime) : status;
}

/* The shared host alone owns detached native windows and their close guards. */
static UmiStatus runtime_surface_float(UmiStudioGtkRuntimeChrome *runtime,
    UmiStudioRuntimeSurfaceKind kind)
{
    return runtime_workspace_open_surface(runtime, kind, "floating");
}

/*
 * Provide the runtime on surface place operation used by this module and its client
 * applications.
 */
static void runtime_on_surface_place(GtkButton *button, gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    UmiStudioRuntimeSurfaceKind kind =
        (UmiStudioRuntimeSurfaceKind)GPOINTER_TO_INT(
            g_object_get_data(G_OBJECT(button), "umicom-surface-kind"));
    UmiStudioGtkRuntimeRegion region =
        (UmiStudioGtkRuntimeRegion)GPOINTER_TO_INT(
            g_object_get_data(G_OBJECT(button), "umicom-surface-region"));
    UmiStatus status = runtime_surface_place(runtime, kind, region);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_PERMISSION_DENIED) {
        runtime_show_notification(
            runtime,
            "Unlock the layout before moving or adding panels.");
    }
}

/*
 * Provide the runtime on surface hide operation used by this module and its client
 * applications.
 */
static void runtime_on_surface_hide(GtkButton *button, gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    UmiStudioRuntimeSurfaceKind kind =
        (UmiStudioRuntimeSurfaceKind)GPOINTER_TO_INT(
            g_object_get_data(G_OBJECT(button), "umicom-surface-kind"));
    UmiStatus status = runtime_surface_hide(runtime, kind);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_PERMISSION_DENIED) {
        runtime_show_notification(
            runtime,
            "Unlock the layout before hiding panels.");
    }
}

/* Handle the catalogue's explicit Float action with the same lock guard as
 * every other layout mutation. */
static void runtime_on_surface_float(GtkButton *button, gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    UmiStudioRuntimeSurfaceKind kind =
        (UmiStudioRuntimeSurfaceKind)GPOINTER_TO_INT(
            g_object_get_data(G_OBJECT(button), "umicom-surface-kind"));
    UmiStatus status = runtime_surface_float(runtime, kind);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_PERMISSION_DENIED) {
        runtime_show_notification(
            runtime, "Unlock the layout before floating a panel.");
    }
}

/*
 * Provide the runtime on window catalogue search operation used by this module and its
 * client applications.
 */
static void runtime_on_window_catalogue_search(
    GtkSearchEntry *entry,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *query;
    GtkWidget *row;
    char *query_folded;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->window_catalogue_list == NULL) return;
    query = gtk_editable_get_text(GTK_EDITABLE(entry));
    query_folded = g_utf8_casefold(query != NULL ? query : "", -1);
    row = gtk_widget_get_first_child(runtime->window_catalogue_list);
    /*
     * Continue only while work remains available; the loop body advances the state on each
     * pass.
     */
    while (row != NULL) {
        GtkWidget *content = gtk_list_box_row_get_child(GTK_LIST_BOX_ROW(row));
        const char *search_text = content != NULL
            ? (const char *)g_object_get_data(
                  G_OBJECT(content), "umicom-window-search-text")
            : NULL;
        char *search_folded = g_utf8_casefold(
            search_text != NULL ? search_text : "", -1);
        GtkWidget *next = gtk_widget_get_next_sibling(row);
        int visible = query_folded == NULL || query_folded[0] == '\0' ||
            (search_folded != NULL && strstr(search_folded, query_folded) != NULL);
        gtk_widget_set_visible(row, visible);
        g_free(search_folded);
        row = next;
    }
    g_free(query_folded);
}

/*
 * Provide the runtime surface place button operation used by this module and its client
 * applications.
 */
static GtkWidget *runtime_surface_place_button(
    UmiStudioGtkRuntimeChrome *runtime,
    UmiStudioRuntimeSurfaceKind kind,
    UmiStudioGtkRuntimeRegion region,
    const char *label)
{
    GtkWidget *button = gtk_button_new_with_label(label);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (button == NULL) return NULL;
    gtk_widget_add_css_class(button, "flat");
    g_object_set_data(
        G_OBJECT(button), "umicom-surface-kind", GINT_TO_POINTER((int)kind));
    g_object_set_data(
        G_OBJECT(button), "umicom-surface-region", GINT_TO_POINTER((int)region));
    g_signal_connect(
        button, "clicked", G_CALLBACK(runtime_on_surface_place), runtime);
    return button;
}

/*
 * Provide the runtime build window catalogue operation used by this module and its client
 * applications.
 */
static UmiStatus runtime_build_window_catalogue(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *root;
    GtkWidget *heading;
    GtkWidget *help;
    GtkWidget *scroll;
    size_t index;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->owner == NULL ||
        runtime->owner->window == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    runtime->window_catalogue_window = GTK_WIDGET(gtk_window_new());
    runtime->window_catalogue_search = gtk_search_entry_new();
    runtime->window_catalogue_list = gtk_list_box_new();
    root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    heading = runtime_section_title("New Window");
    help = runtime_label(
         "Place panels on the canvas, dock left, right, bottom or centre, or detach into native "
        "windows. Unlock the current layout before changing its arrangement.",
        0.0F);
    scroll = gtk_scrolled_window_new();
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->window_catalogue_window == NULL ||
        runtime->window_catalogue_search == NULL ||
        runtime->window_catalogue_list == NULL || root == NULL ||
        heading == NULL || help == NULL || scroll == NULL) {
        return UMI_STATUS_OUT_OF_MEMORY;
    }

    gtk_window_set_title(
        GTK_WINDOW(runtime->window_catalogue_window), "Studio New Window");
    gtk_window_set_default_size(
        GTK_WINDOW(runtime->window_catalogue_window), 720, 620);
    gtk_window_set_modal(GTK_WINDOW(runtime->window_catalogue_window), TRUE);
    gtk_window_set_transient_for(
        GTK_WINDOW(runtime->window_catalogue_window), runtime->owner->window);
    gtk_widget_set_margin_top(root, 12);
    gtk_widget_set_margin_bottom(root, 12);
    gtk_widget_set_margin_start(root, 12);
    gtk_widget_set_margin_end(root, 12);
    gtk_widget_add_css_class(help, "umicom-runtime-muted");
    gtk_search_entry_set_placeholder_text(
        GTK_SEARCH_ENTRY(runtime->window_catalogue_search),
        "Search Explorer, Problems, Terminal, AI, Debug and other panels");
    gtk_list_box_set_selection_mode(
        GTK_LIST_BOX(runtime->window_catalogue_list), GTK_SELECTION_NONE);

    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U;
         index < sizeof(RUNTIME_SURFACE_PROFILES) /
                 sizeof(RUNTIME_SURFACE_PROFILES[0]);
         ++index) {
        const UmiStudioGtkSurfaceProfile *profile =
            &RUNTIME_SURFACE_PROFILES[index];
        GtkWidget *row;
        GtkWidget *identity;
        GtkWidget *title;
        GtkWidget *location;
        GtkWidget *actions;
        GtkWidget *left;
        GtkWidget *right;
        GtkWidget *bottom;
        GtkWidget *centre;
        GtkWidget *canvas;
        GtkWidget *floating;
        GtkWidget *hide;
        const char *component_id;
        const UmiApplicationComponentDefinition *definition;
        char *search_text;

        /* Apply this branch only when its contract condition is satisfied. */
        /* Editor is a normal singleton and can be reopened from a blank canvas. */
        component_id = runtime_surface_component_id(profile->kind);
        definition = component_id != NULL
            ? umi_application_component_catalogue_find(component_id)
            : NULL;
        row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
        identity = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
        title = runtime_label(profile->title, 0.0F);
        location = runtime_label(
            runtime_region_text(profile->region), 0.0F);
        actions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
        left = runtime_surface_place_button(
            runtime, profile->kind, UMI_STUDIO_GTK_REGION_PRIMARY, "Left");
        right = runtime_surface_place_button(
            runtime, profile->kind, UMI_STUDIO_GTK_REGION_SECONDARY, "Right");
        bottom = runtime_surface_place_button(
            runtime, profile->kind, UMI_STUDIO_GTK_REGION_BOTTOM, "Bottom");
        centre = runtime_surface_place_button(
            runtime, profile->kind, UMI_STUDIO_GTK_REGION_EDITOR, "Centre");
        canvas = runtime_surface_place_button(
            runtime, profile->kind, UMI_STUDIO_GTK_REGION_CANVAS, "Canvas");
        floating = gtk_button_new_with_label("Float");
        hide = gtk_button_new_with_label("Hide");
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (row == NULL || identity == NULL || title == NULL ||
            location == NULL || actions == NULL || left == NULL ||
            right == NULL || bottom == NULL || centre == NULL ||
            canvas == NULL || floating == NULL || hide == NULL) {
            continue;
        }
        runtime->surface_location_labels[profile->kind] = location;
        gtk_widget_add_css_class(location, "umicom-runtime-muted");
        gtk_widget_set_hexpand(identity, TRUE);
        gtk_box_append(GTK_BOX(identity), title);
        gtk_box_append(GTK_BOX(identity), location);
        gtk_box_append(GTK_BOX(actions), left);
        gtk_box_append(GTK_BOX(actions), right);
        gtk_box_append(GTK_BOX(actions), bottom);
        gtk_box_append(GTK_BOX(actions), centre);
        gtk_box_append(GTK_BOX(actions), canvas);
        gtk_widget_add_css_class(floating, "flat");
        g_object_set_data(
            G_OBJECT(floating), "umicom-surface-kind",
            GINT_TO_POINTER((int)profile->kind));
        g_signal_connect(
            floating,
            "clicked",
            G_CALLBACK(runtime_on_surface_float),
            runtime);
        gtk_box_append(GTK_BOX(actions), floating);
        gtk_widget_add_css_class(hide, "flat");
        g_object_set_data(
            G_OBJECT(hide), "umicom-surface-kind",
            GINT_TO_POINTER((int)profile->kind));
        g_signal_connect(
            hide, "clicked", G_CALLBACK(runtime_on_surface_hide), runtime);
        gtk_box_append(GTK_BOX(actions), hide);
        gtk_box_append(GTK_BOX(row), identity);
        gtk_box_append(GTK_BOX(row), actions);
        /* Framework aliases and descriptions let users find a surface by the
         * job they want to perform, not only by remembering its exact title. */
        search_text = g_strdup_printf(
            "%s %s %s %s %s",
            profile->title,
            profile->stack_name,
            runtime_region_text(profile->region),
            definition != NULL ? definition->description : "",
            definition != NULL ? definition->search_terms : "");
        if (definition != NULL)
            gtk_widget_set_tooltip_text(row, definition->description);
        g_object_set_data_full(
            G_OBJECT(row),
            "umicom-window-search-text",
            search_text,
            g_free);
        gtk_list_box_append(GTK_LIST_BOX(runtime->window_catalogue_list), row);
    }

    g_signal_connect(
        runtime->window_catalogue_search,
        "search-changed",
        G_CALLBACK(runtime_on_window_catalogue_search),
        runtime);
    gtk_scrolled_window_set_policy(
        GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_child(
        GTK_SCROLLED_WINDOW(scroll), runtime->window_catalogue_list);
    gtk_box_append(GTK_BOX(root), heading);
    gtk_box_append(GTK_BOX(root), help);
    gtk_box_append(GTK_BOX(root), runtime->window_catalogue_search);
    gtk_box_append(GTK_BOX(root), scroll);
    gtk_window_set_child(GTK_WINDOW(runtime->window_catalogue_window), root);
    g_signal_connect(
        runtime->window_catalogue_window,
        "close-request",
        G_CALLBACK(runtime_on_palette_close),
        runtime);
    gtk_widget_set_visible(runtime->window_catalogue_window, FALSE);
    return UMI_STATUS_OK;
}

/*
 * Provide the runtime show window catalogue operation used by this module and its client
 * applications.
 */
static void runtime_show_window_catalogue(UmiStudioGtkRuntimeChrome *runtime)
{
    int kind;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->window_catalogue_window == NULL) return;
    /* Visit each bounded item once so every record receives the same rule. */
    for (kind = (int)UMI_STUDIO_SURFACE_EXPLORER;
         kind <= (int)UMI_STUDIO_SURFACE_LAST;
         ++kind) {
        runtime_surface_location_refresh(
            runtime, (UmiStudioRuntimeSurfaceKind)kind);
    }
    gtk_window_present(GTK_WINDOW(runtime->window_catalogue_window));
}
