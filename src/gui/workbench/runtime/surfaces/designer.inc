/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/designer.inc
 *
 * PURPOSE:
 *   Render Studio's dockable visual designer, searchable component palette,
 *   semantic structure and editable property inspector from one synchronized
 *   Framework workspace model.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

typedef struct UmiStudioGtkDesignerPaletteState {
    UmiStudioGtkRuntimeChrome *runtime;
    GtkWidget *list;
} UmiStudioGtkDesignerPaletteState;

typedef struct UmiStudioGtkDesignerPropertyState {
    UmiStudioGtkRuntimeChrome *runtime;
    GtkWidget *entry;
    char property_name[UMI_DECL_NAME_CAPACITY];
} UmiStudioGtkDesignerPropertyState;

typedef struct UmiStudioGtkDesignerCanvasState {
    UmiStudioDesigner *designer;
} UmiStudioGtkDesignerCanvasState;

/* Keep one debounced editor callback alive for the lifetime of its panel. */
typedef struct UmiStudioGtkDesignerSourceState {
    UmiStudioGtkRuntimeChrome *runtime;
    GtkWidget *text_view;
    GtkWidget *status_label;
    guint debounce_source;
    UmiStudioRuntimeSurfaceKind surface;
} UmiStudioGtkDesignerSourceState;

/* Resolve the application-owned designer while keeping every GTK surface
 * independent from the concrete Studio service structure. */
static UmiStudioDesigner *runtime_designer_service(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioServices *services;

    if (runtime == NULL || runtime->owner == NULL ||
        runtime->owner->ui == NULL) {
        return NULL;
    }
    services = umi_studio_ui_services(runtime->owner->ui);
    return services != NULL ? umi_studio_services_designer(services) : NULL;
}

/* Ask the Studio facade for the single coherent model shared by all four
 * designer surfaces. Callers never read document internals independently. */
static UmiStatus runtime_designer_workspace_model(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *query,
    UmiDesignerWorkspaceModel *out_model)
{
    UmiStudioDesigner *designer = runtime_designer_service(runtime);

    if (designer == NULL || out_model == NULL) {
        return UMI_STATUS_INVALID_STATE;
    }
    return umi_studio_designer_workspace_model(
        designer,
        query != NULL ? query : "",
        out_model);
}

/* Rebuild after GTK finishes dispatching the current event. This prevents an
 * activated row from deleting itself while its callback is still on stack. */
static gboolean runtime_designer_refresh_idle(gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;

    if (runtime == NULL) return G_SOURCE_REMOVE;
    runtime->designer_refresh_source = 0U;
    if (runtime->runtime_ready) {
        if (runtime_rebuild_surface_panels(runtime) == UMI_STATUS_OK) {
            /* Re-select the design workspace after replacing its widgets so
             * the centre stack cannot fall back to the editor unexpectedly. */
            runtime_show_surface(
                runtime, UMI_STUDIO_SURFACE_COMPONENT_PALETTE);
            runtime_show_surface(
                runtime, UMI_STUDIO_SURFACE_COMPONENT_STRUCTURE);
            runtime_show_surface(
                runtime, UMI_STUDIO_SURFACE_OBJECT_INSPECTOR);
            runtime_show_surface(
                runtime, UMI_STUDIO_SURFACE_DESIGN_CODE);
            runtime_show_surface(
                runtime, UMI_STUDIO_SURFACE_DESIGN_MIXED);
            runtime_show_surface(
                runtime, UMI_STUDIO_SURFACE_DESIGN_PREVIEW);
            runtime_show_surface(
                runtime,
                runtime->designer_active_surface >=
                        UMI_STUDIO_SURFACE_VISUAL_DESIGNER &&
                    runtime->designer_active_surface <=
                        UMI_STUDIO_SURFACE_DESIGN_PREVIEW
                    ? runtime->designer_active_surface
                    : UMI_STUDIO_SURFACE_VISUAL_DESIGNER);
        }
    }
    return G_SOURCE_REMOVE;
}

/* Coalesce several related edits into one safe panel refresh. */
static void runtime_queue_designer_refresh(
    UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime == NULL || runtime->designer_refresh_source != 0U) return;
    runtime->designer_refresh_source = g_idle_add(
        runtime_designer_refresh_idle,
        runtime);
}

/* Add the chosen registered component through Studio's undoable designer
 * facade, then refresh every view of the shared document revision. */
static void runtime_on_designer_add_component(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    UmiStudioDesigner *designer = runtime_designer_service(runtime);
    const char *component_type = (const char *)g_object_get_data(
        G_OBJECT(button),
        "umicom-designer-component-type");
    char node_id[UMI_DECL_ID_CAPACITY];
    UmiStatus status;

    if (designer == NULL || component_type == NULL) return;
    status = umi_studio_designer_add_palette_component(
        designer,
        component_type,
        node_id,
        sizeof(node_id));
    runtime_show_notification(
        runtime,
        status == UMI_STATUS_OK
            ? "Component added to the active design."
            : umi_status_text(status));
    if (status == UMI_STATUS_OK) runtime_queue_designer_refresh(runtime);
}

/* Select a semantic node so the canvas, structure and property inspector all
 * project the same selection on their next refresh. */
static void runtime_on_designer_select_node(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    UmiStudioDesigner *designer = runtime_designer_service(runtime);
    const char *node_id = (const char *)g_object_get_data(
        G_OBJECT(button),
        "umicom-designer-node-id");
    UmiStatus status;

    if (designer == NULL || node_id == NULL) return;
    status = umi_studio_designer_select(designer, node_id);
    if (status == UMI_STATUS_OK) {
        runtime_queue_designer_refresh(runtime);
    } else {
        runtime_show_notification(runtime, umi_status_text(status));
    }
}

/* Apply the text currently shown for one schema-backed property. Validation
 * and history recording remain inside Framework and the Studio facade. */
static void runtime_on_designer_apply_property(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkDesignerPropertyState *state =
        (UmiStudioGtkDesignerPropertyState *)user_data;
    UmiStudioDesigner *designer;
    const char *value;
    UmiStatus status;

    (void)button;
    if (state == NULL || state->runtime == NULL || state->entry == NULL) {
        return;
    }
    designer = runtime_designer_service(state->runtime);
    if (designer == NULL) return;
    value = gtk_editable_get_text(GTK_EDITABLE(state->entry));
    status = umi_studio_designer_set_selected_property(
        designer,
        state->property_name,
        value != NULL ? value : "");
    runtime_show_notification(
        state->runtime,
        status == UMI_STATUS_OK
            ? "Property updated."
            : umi_status_text(status));
    if (status == UMI_STATUS_OK) {
        runtime_queue_designer_refresh(state->runtime);
    }
}

/* Route undo through the designer history service and refresh all projections
 * only when the operation succeeds. */
static void runtime_on_designer_undo(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    UmiStudioDesigner *designer = runtime_designer_service(runtime);
    UmiStatus status;

    (void)button;
    if (designer == NULL) return;
    status = umi_studio_designer_undo(designer);
    runtime_show_notification(runtime, umi_status_text(status));
    if (status == UMI_STATUS_OK) runtime_queue_designer_refresh(runtime);
}

/* Route redo through the designer history service so the UI cannot bypass
 * the same operation sequence used by headless and future frontends. */
static void runtime_on_designer_redo(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    UmiStudioDesigner *designer = runtime_designer_service(runtime);
    UmiStatus status;

    (void)button;
    if (designer == NULL) return;
    status = umi_studio_designer_redo(designer);
    runtime_show_notification(runtime, umi_status_text(status));
    if (status == UMI_STATUS_OK) runtime_queue_designer_refresh(runtime);
}

/* Rebuild only the palette rows while a user types, preserving keyboard focus
 * and avoiding a disruptive whole-workbench reconstruction. */
static void runtime_designer_palette_populate(
    UmiStudioGtkDesignerPaletteState *state,
    const char *query)
{
    UmiDesignerWorkspaceModel model;
    size_t index;

    if (state == NULL || state->runtime == NULL || state->list == NULL) return;
    runtime_clear_box(state->list);
    if (runtime_designer_workspace_model(
            state->runtime,
            query,
            &model) != UMI_STATUS_OK) {
        gtk_box_append(
            GTK_BOX(state->list),
            runtime_label("The component catalogue is unavailable.", 0.0F));
        return;
    }

    for (index = 0U; index < model.palette_count; ++index) {
        const UmiDesignerWorkspacePaletteItem *item =
            &model.palette_items[index];
        GtkWidget *button = gtk_button_new();
        GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
        GtkWidget *name = runtime_label(item->display_name, 0.0F);
        char detail[320];

        (void)snprintf(
            detail,
            sizeof(detail),
            "%s · %s%s",
            item->component_type,
            item->category,
            item->container ? " · container" : "");
        if (content == NULL || button == NULL || name == NULL) continue;
        gtk_box_append(GTK_BOX(content), name);
        gtk_box_append(GTK_BOX(content), runtime_label(detail, 0.0F));
        gtk_button_set_child(GTK_BUTTON(button), content);
        gtk_widget_set_tooltip_text(button, "Add this component to the design");
        g_object_set_data_full(
            G_OBJECT(button),
            "umicom-designer-component-type",
            g_strdup(item->component_type),
            g_free);
        g_signal_connect(
            button,
            "clicked",
            G_CALLBACK(runtime_on_designer_add_component),
            state->runtime);
        gtk_box_append(GTK_BOX(state->list), button);
    }
}

/* Keep the bounded search text in presentation state and immediately ask the
 * Framework model for ranked palette results. */
static void runtime_on_designer_palette_search(
    GtkSearchEntry *entry,
    gpointer user_data)
{
    UmiStudioGtkDesignerPaletteState *state =
        (UmiStudioGtkDesignerPaletteState *)user_data;
    const char *query;

    if (state == NULL || state->runtime == NULL) return;
    query = gtk_editable_get_text(GTK_EDITABLE(entry));
    (void)g_strlcpy(
        state->runtime->designer_palette_query,
        query != NULL ? query : "",
        sizeof(state->runtime->designer_palette_query));
    runtime_designer_palette_populate(state, query);
}

/* Build the searchable palette as an ordinary dockable workbench surface. */
static GtkWidget *runtime_build_surface_component_palette(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Components");
    GtkWidget *search;
    GtkWidget *list;
    GtkWidget *scroll;
    UmiStudioGtkDesignerPaletteState *state;

    if (panel == NULL || runtime == NULL) return panel;
    search = gtk_search_entry_new();
    list = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    scroll = runtime_scroll(list);
    state = g_try_new0(UmiStudioGtkDesignerPaletteState, 1);
    if (search == NULL || list == NULL || scroll == NULL || state == NULL) {
        g_free(state);
        return panel;
    }
    state->runtime = runtime;
    state->list = list;
    gtk_search_entry_set_placeholder_text(
        GTK_SEARCH_ENTRY(search),
        "Search components");
    gtk_editable_set_text(
        GTK_EDITABLE(search),
        runtime->designer_palette_query);
    gtk_box_append(GTK_BOX(panel), search);
    gtk_box_append(GTK_BOX(panel), scroll);
    g_object_set_data_full(
        G_OBJECT(panel),
        "umicom-designer-palette-state",
        state,
        g_free);
    g_signal_connect(
        search,
        "search-changed",
        G_CALLBACK(runtime_on_designer_palette_search),
        state);
    runtime_designer_palette_populate(
        state,
        runtime->designer_palette_query);
    return panel;
}

/* Build the semantic hierarchy. Indentation comes from Framework depth data,
 * and selection remains a designer operation rather than GTK-owned state. */
static GtkWidget *runtime_build_surface_component_structure(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Structure");
    UmiDesignerWorkspaceModel model;
    size_t index;

    if (panel == NULL || runtime == NULL) return panel;
    if (runtime_designer_workspace_model(
            runtime,
            runtime->designer_palette_query,
            &model) != UMI_STATUS_OK) {
        gtk_box_append(
            GTK_BOX(panel),
            runtime_label("The design structure is unavailable.", 0.0F));
        return runtime_scroll(panel);
    }
    for (index = 0U; index < model.hierarchy.count; ++index) {
        const UmiDesignerTreeNode *node = &model.hierarchy.nodes[index];
        GtkWidget *button;
        char label[320];

        (void)snprintf(
            label,
            sizeof(label),
            "%*s%s  [%s]",
            (int)(node->depth * 2U),
            "",
            node->node_id,
            node->component_type);
        button = gtk_button_new_with_label(label);
        if (button == NULL) continue;
        gtk_widget_set_halign(button, GTK_ALIGN_FILL);
        if (node->selected) gtk_widget_add_css_class(button, "suggested-action");
        g_object_set_data_full(
            G_OBJECT(button),
            "umicom-designer-node-id",
            g_strdup(node->node_id),
            g_free);
        g_signal_connect(
            button,
            "clicked",
            G_CALLBACK(runtime_on_designer_select_node),
            runtime);
        gtk_box_append(GTK_BOX(panel), button);
    }
    return runtime_scroll(panel);
}

/* Find an existing value for a schema property. Missing properties use their
 * declared default without inventing a second source of truth in GTK. */
static const char *runtime_designer_property_value(
    const UmiDesignerWorkspaceModel *model,
    const UmiDeclPropertyDescriptor *property)
{
    size_t index;

    if (model == NULL || property == NULL) return "";
    for (index = 0U; index < model->inspector.attribute_count; ++index) {
        if (strcmp(
                model->inspector.attributes[index].name,
                property->name) == 0) {
            return model->inspector.attributes[index].value.text;
        }
    }
    return property->default_value;
}

/* Build an editable row for one typed property from the Framework schema. */
static GtkWidget *runtime_designer_property_row(
    UmiStudioGtkRuntimeChrome *runtime,
    const UmiDesignerWorkspaceModel *model,
    const UmiDeclPropertyDescriptor *property)
{
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    GtkWidget *heading;
    GtkWidget *controls = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    GtkWidget *entry = gtk_entry_new();
    GtkWidget *apply = gtk_button_new_with_label("Apply");
    UmiStudioGtkDesignerPropertyState *state;
    char title[256];

    if (box == NULL || controls == NULL || entry == NULL || apply == NULL) {
        return box;
    }
    (void)snprintf(
        title,
        sizeof(title),
        "%s · %s%s",
        property->name,
        umi_decl_value_kind_text(property->kind),
        property->required ? " · required" : "");
    heading = runtime_label(title, 0.0F);
    gtk_editable_set_text(
        GTK_EDITABLE(entry),
        runtime_designer_property_value(model, property));
    gtk_widget_set_hexpand(entry, TRUE);
    if (heading != NULL) gtk_box_append(GTK_BOX(box), heading);
    gtk_box_append(GTK_BOX(controls), entry);
    gtk_box_append(GTK_BOX(controls), apply);
    gtk_box_append(GTK_BOX(box), controls);

    state = g_try_new0(UmiStudioGtkDesignerPropertyState, 1);
    if (state != NULL) {
        state->runtime = runtime;
        state->entry = entry;
        (void)g_strlcpy(
            state->property_name,
            property->name,
            sizeof(state->property_name));
        g_object_set_data_full(
            G_OBJECT(apply),
            "umicom-designer-property-state",
            state,
            g_free);
        g_signal_connect(
            apply,
            "clicked",
            G_CALLBACK(runtime_on_designer_apply_property),
            state);
    } else {
        gtk_widget_set_sensitive(apply, FALSE);
    }
    return box;
}

/* Build the property inspector from both current values and the registered
 * component schema, allowing valid properties to be added for the first time. */
static GtkWidget *runtime_build_surface_object_inspector(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Properties");
    UmiDesignerWorkspaceModel model;
    size_t index;
    char summary[320];

    if (panel == NULL || runtime == NULL) return panel;
    if (runtime_designer_workspace_model(
            runtime,
            runtime->designer_palette_query,
            &model) != UMI_STATUS_OK ||
        !model.has_inspector || !model.has_inspector_schema) {
        gtk_box_append(
            GTK_BOX(panel),
            runtime_label(
                "Select a component to inspect and edit its properties.",
                0.0F));
        return runtime_scroll(panel);
    }
    (void)snprintf(
        summary,
        sizeof(summary),
        "%s · %s",
        model.inspector.node_id,
        model.inspector.component_type);
    gtk_box_append(GTK_BOX(panel), runtime_section_title(summary));
    for (index = 0U;
         index < model.inspector_schema.property_count;
         ++index) {
        GtkWidget *row = runtime_designer_property_row(
            runtime,
            &model,
            &model.inspector_schema.properties[index]);
        if (row != NULL) gtk_box_append(GTK_BOX(panel), row);
    }
    return runtime_scroll(panel);
}

/* Draw semantic component geometry from Framework. GTK owns pixels only; it
 * never becomes the source of component position, size or selection. */
static void runtime_designer_draw_canvas(
    GtkDrawingArea *area,
    cairo_t *cr,
    int width,
    int height,
    gpointer user_data)
{
    UmiStudioGtkDesignerCanvasState *state =
        (UmiStudioGtkDesignerCanvasState *)user_data;
    UmiDesignerWorkspaceModel model;
    size_t index;

    (void)area;
    if (state == NULL || state->designer == NULL) return;
    cairo_set_source_rgb(cr, 0.10, 0.12, 0.16);
    cairo_rectangle(cr, 0.0, 0.0, (double)width, (double)height);
    cairo_fill(cr);
    if (umi_studio_designer_workspace_model(
            state->designer,
            "",
            &model) != UMI_STATUS_OK) {
        return;
    }
    for (index = 0U; index < model.hierarchy.count; ++index) {
        const UmiDesignerTreeNode *node = &model.hierarchy.nodes[index];
        UmiDesignerRect rect;

        if (umi_designer_surface_get_rect(
                umi_studio_designer_document(state->designer),
                node->node_id,
                &rect) != UMI_STATUS_OK) {
            continue;
        }
        cairo_set_source_rgb(
            cr,
            node->selected ? 0.86 : 0.31,
            node->selected ? 0.35 : 0.61,
            node->selected ? 0.38 : 0.86);
        cairo_set_line_width(cr, node->selected ? 3.0 : 1.5);
        cairo_rectangle(
            cr,
            (double)rect.x,
            (double)rect.y,
            (double)rect.width,
            (double)rect.height);
        cairo_stroke(cr);
        cairo_move_to(cr, (double)rect.x + 6.0, (double)rect.y + 17.0);
        cairo_show_text(cr, node->node_id);
    }
}

/* Create a canvas that borrows the designer service and owns only its small
 * drawing callback record. This helper is shared by three workbench modes. */
static GtkWidget *runtime_designer_canvas_new(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *canvas = gtk_drawing_area_new();
    UmiStudioGtkDesignerCanvasState *state;

    if (canvas == NULL || runtime_designer_service(runtime) == NULL) {
        return canvas;
    }
    state = g_try_new0(UmiStudioGtkDesignerCanvasState, 1);
    if (state != NULL) {
        state->designer = runtime_designer_service(runtime);
        gtk_drawing_area_set_draw_func(
            GTK_DRAWING_AREA(canvas),
            runtime_designer_draw_canvas,
            state,
            g_free);
    }
    gtk_widget_set_size_request(canvas, 360, 260);
    gtk_widget_set_hexpand(canvas, TRUE);
    gtk_widget_set_vexpand(canvas, TRUE);
    return canvas;
}

/* Cancel a pending timeout before releasing the state referenced by it. */
static void runtime_designer_source_state_free(gpointer user_data)
{
    UmiStudioGtkDesignerSourceState *state =
        (UmiStudioGtkDesignerSourceState *)user_data;

    if (state == NULL) return;
    if (state->debounce_source != 0U) {
        g_source_remove(state->debounce_source);
        state->debounce_source = 0U;
    }
    g_free(state);
}

/* Apply the most recent complete source buffer after typing has paused. An
 * invalid edit updates the message but leaves the last valid design visible. */
static gboolean runtime_designer_apply_source_timeout(gpointer user_data)
{
    UmiStudioGtkDesignerSourceState *state =
        (UmiStudioGtkDesignerSourceState *)user_data;
    UmiStudioDesigner *designer;
    GtkTextBuffer *buffer;
    GtkTextIter start;
    GtkTextIter end;
    UmiDeclDiagnosticList diagnostics;
    char *source;
    UmiStatus status;

    if (state == NULL || state->runtime == NULL ||
        state->text_view == NULL) {
        return G_SOURCE_REMOVE;
    }
    state->debounce_source = 0U;
    designer = runtime_designer_service(state->runtime);
    if (designer == NULL) return G_SOURCE_REMOVE;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(state->text_view));
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    source = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
    if (source == NULL) return G_SOURCE_REMOVE;
    umi_decl_diagnostics_clear(&diagnostics);
    status = umi_studio_designer_apply_live_source(
        designer,
        source,
        (uint64_t)(g_get_monotonic_time() / 1000),
        &diagnostics);
    g_free(source);

    if (state->status_label != NULL) {
        gtk_label_set_text(
            GTK_LABEL(state->status_label),
            status == UMI_STATUS_OK
                ? "Preview updated from the latest valid source."
                : diagnostics.count > 0U
                    ? diagnostics.items[0].message
                    : umi_status_text(status));
    }
    if (status == UMI_STATUS_OK) {
        state->runtime->designer_active_surface = state->surface;
        runtime_queue_designer_refresh(state->runtime);
    }
    return G_SOURCE_REMOVE;
}

/* Restart the short idle timer after every edit so incomplete keystrokes do
 * not repeatedly parse or render the document. */
static void runtime_on_designer_source_changed(
    GtkTextBuffer *buffer,
    gpointer user_data)
{
    UmiStudioGtkDesignerSourceState *state =
        (UmiStudioGtkDesignerSourceState *)user_data;

    (void)buffer;
    if (state == NULL) return;
    if (state->debounce_source != 0U) {
        g_source_remove(state->debounce_source);
    }
    state->debounce_source = g_timeout_add(
        UMI_DESIGNER_LIVE_DEFAULT_DEBOUNCE_MS,
        runtime_designer_apply_source_timeout,
        state);
    if (state->status_label != NULL) {
        gtk_label_set_text(
            GTK_LABEL(state->status_label),
            "Waiting briefly for typing to pause…");
    }
}

/* Build an editable source view from Framework's canonical live-source
 * snapshot. Code and Mixed surfaces call the same helper. */
static GtkWidget *runtime_designer_source_editor_new(
    UmiStudioGtkRuntimeChrome *runtime,
    UmiStudioRuntimeSurfaceKind surface)
{
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *status_label = runtime_label(
        "Valid edits refresh the design after a short pause.",
        0.0F);
    GtkWidget *text_view = gtk_text_view_new();
    GtkWidget *scroll = runtime_scroll(text_view);
    UmiDesignerLiveSourceWorkspace workspace;
    UmiStudioGtkDesignerSourceState *state;

    if (box == NULL || text_view == NULL || scroll == NULL ||
        runtime == NULL) {
        return box;
    }
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(text_view), TRUE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_NONE);
    gtk_widget_set_hexpand(scroll, TRUE);
    gtk_widget_set_vexpand(scroll, TRUE);
    if (umi_studio_designer_live_source_snapshot(
            runtime_designer_service(runtime),
            &workspace) == UMI_STATUS_OK) {
        gtk_text_buffer_set_text(
            gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view)),
            workspace.source,
            (gint)workspace.source_length);
    }
    if (status_label != NULL) gtk_box_append(GTK_BOX(box), status_label);
    gtk_box_append(GTK_BOX(box), scroll);

    state = g_try_new0(UmiStudioGtkDesignerSourceState, 1);
    if (state != NULL) {
        state->runtime = runtime;
        state->text_view = text_view;
        state->status_label = status_label;
        state->surface = surface;
        g_object_set_data_full(
            G_OBJECT(box),
            "umicom-designer-source-state",
            state,
            runtime_designer_source_state_free);
        g_signal_connect(
            gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view)),
            "changed",
            G_CALLBACK(runtime_on_designer_source_changed),
            state);
    } else {
        gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    }
    return box;
}

/* Build the central Design surface with undo and redo controls. */
static GtkWidget *runtime_build_surface_visual_designer(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Design");
    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    GtkWidget *undo = gtk_button_new_with_label("Undo");
    GtkWidget *redo = gtk_button_new_with_label("Redo");
    GtkWidget *canvas = runtime_designer_canvas_new(runtime);

    if (panel == NULL || toolbar == NULL || undo == NULL || redo == NULL ||
        canvas == NULL || runtime_designer_service(runtime) == NULL) {
        if (panel != NULL) {
            gtk_box_append(
                GTK_BOX(panel),
                runtime_label("The visual designer is unavailable.", 0.0F));
        }
        return panel;
    }
    gtk_box_append(GTK_BOX(toolbar), undo);
    gtk_box_append(GTK_BOX(toolbar), redo);
    gtk_box_append(GTK_BOX(panel), toolbar);
    gtk_box_append(GTK_BOX(panel), canvas);
    g_signal_connect(
        undo,
        "clicked",
        G_CALLBACK(runtime_on_designer_undo),
        runtime);
    g_signal_connect(
        redo,
        "clicked",
        G_CALLBACK(runtime_on_designer_redo),
        runtime);
    return panel;
}

/* Build a full-size canonical source editor as its own dockable surface. */
static GtkWidget *runtime_build_surface_design_code(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Code");
    GtkWidget *editor = runtime_designer_source_editor_new(
        runtime,
        UMI_STUDIO_SURFACE_DESIGN_CODE);

    if (panel != NULL && editor != NULL) {
        gtk_box_append(GTK_BOX(panel), editor);
    }
    return panel;
}

/* Build the Mixed surface with source and design sharing a resizable sash. */
static GtkWidget *runtime_build_surface_design_mixed(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Mixed");
    GtkWidget *split = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    GtkWidget *editor = runtime_designer_source_editor_new(
        runtime,
        UMI_STUDIO_SURFACE_DESIGN_MIXED);
    GtkWidget *canvas = runtime_designer_canvas_new(runtime);

    if (panel == NULL || split == NULL || editor == NULL || canvas == NULL) {
        return panel;
    }
    gtk_paned_set_start_child(GTK_PANED(split), editor);
    gtk_paned_set_end_child(GTK_PANED(split), canvas);
    gtk_paned_set_resize_start_child(GTK_PANED(split), TRUE);
    gtk_paned_set_resize_end_child(GTK_PANED(split), TRUE);
    gtk_paned_set_shrink_start_child(GTK_PANED(split), FALSE);
    gtk_paned_set_shrink_end_child(GTK_PANED(split), FALSE);
    gtk_widget_set_hexpand(split, TRUE);
    gtk_widget_set_vexpand(split, TRUE);
    gtk_box_append(GTK_BOX(panel), split);
    return panel;
}

/* Build a read-only live preview with visual output above an inspectable
 * renderer result. This remains useful in headless and future adapters. */
static GtkWidget *runtime_build_surface_design_preview(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Preview");
    GtkWidget *split = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    GtkWidget *canvas = runtime_designer_canvas_new(runtime);
    GtkWidget *text_view = gtk_text_view_new();
    GtkWidget *scroll = runtime_scroll(text_view);
    UmiDesignerLiveSourceWorkspace workspace;
    char summary[256];

    if (panel == NULL || split == NULL || canvas == NULL ||
        text_view == NULL || scroll == NULL) {
        return panel;
    }
    (void)memset(&workspace, 0, sizeof(workspace));
    if (umi_studio_designer_live_source_snapshot(
            runtime_designer_service(runtime),
            &workspace) == UMI_STATUS_OK) {
        (void)snprintf(
            summary,
            sizeof(summary),
            "Revision %llu · %s",
            (unsigned long long)workspace.preview_revision,
            umi_designer_preview_health_text(workspace.health));
        gtk_box_append(GTK_BOX(panel), runtime_label(summary, 0.0F));
        gtk_text_buffer_set_text(
            gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view)),
            workspace.preview,
            (gint)workspace.preview_length);
    }
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(text_view), TRUE);
    gtk_paned_set_start_child(GTK_PANED(split), canvas);
    gtk_paned_set_end_child(GTK_PANED(split), scroll);
    gtk_paned_set_resize_start_child(GTK_PANED(split), TRUE);
    gtk_paned_set_resize_end_child(GTK_PANED(split), TRUE);
    gtk_widget_set_hexpand(split, TRUE);
    gtk_widget_set_vexpand(split, TRUE);
    gtk_box_append(GTK_BOX(panel), split);
    return panel;
}
