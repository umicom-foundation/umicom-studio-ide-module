/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_welcome.inc
 *
 * PURPOSE:
 *   Render a calm startup centre for creating, opening, learning and resuming
 *   work. Commands remain owned by Framework and Studio services; this file
 *   only composes their GTK presentation.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/developer_workbench/start_centre.h"

/* Create one reusable welcome card with a heading and wrapped explanation. */
static GtkWidget *runtime_welcome_card(
    const char *title,
    const char *description)
{
    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *heading = runtime_label(title, 0.0F);
    GtkWidget *detail = runtime_label(description, 0.0F);

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (card == NULL || heading == NULL || detail == NULL) return card;
    gtk_widget_add_css_class(card, "umicom-welcome-card");
    gtk_widget_set_hexpand(card, TRUE);
    /*
     * The previous welcome layout stretched every card to the complete centre
     * height. It is retained for review because it was the established
     * behaviour, but natural-height cards keep actions grouped near their
     * headings and leave unused workspace below the cards instead.
     */
#if 0
    gtk_widget_set_vexpand(card, TRUE);
#endif
    gtk_widget_set_vexpand(card, FALSE);
    gtk_widget_set_valign(card, GTK_ALIGN_START);
    gtk_widget_add_css_class(heading, "umicom-welcome-card-title");
    gtk_widget_add_css_class(detail, "umicom-runtime-muted");
    gtk_label_set_wrap(GTK_LABEL(detail), TRUE);
    gtk_label_set_wrap_mode(GTK_LABEL(detail), PANGO_WRAP_WORD_CHAR);
    gtk_label_set_max_width_chars(GTK_LABEL(detail), 38);
    gtk_box_append(GTK_BOX(card), heading);
    gtk_box_append(GTK_BOX(card), detail);
    return card;
}

/* Add a Framework command to a card and optionally mark it as the main action. */
static void runtime_welcome_add_action(
    UmiStudioGtkRuntimeChrome *runtime,
    GtkWidget *card,
    const char *label,
    const char *command_id,
    const char *description,
    int primary)
{
    GtkWidget *button = runtime_make_command_button(
        runtime, label, command_id, description);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (button == NULL || card == NULL) return;
    gtk_widget_set_halign(button, GTK_ALIGN_FILL);
    /* Apply this branch only when its contract condition is satisfied. */
    if (primary) gtk_widget_add_css_class(button, "suggested-action");
    gtk_box_append(GTK_BOX(card), button);
}

/*
 * Present one recent workspace with its full path and current availability.
 * Opening still routes through the existing workspace command and service.
 */
static GtkWidget *runtime_welcome_recent_workspace(
    UmiStudioGtkRuntimeChrome *runtime,
    const UmiDeveloperWorkbenchRecentWorkItem *item)
{
    GtkWidget *row;
    GtkWidget *open;
    GtkWidget *path;
    GtkWidget *state;
    const char *state_text;

    if (runtime == NULL || item == NULL) return NULL;

    row = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    open = runtime_make_command_button(
        runtime,
        item->recent.label[0] != '\0'
            ? item->recent.label : item->recent.uri,
        "workspace.open-folder",
        item->recent.uri);
    path = runtime_label(item->recent.uri, 0.0F);
    state_text = item->available
        ? (item->recent.pinned ? "Pinned workspace" : "Recent workspace")
        : "Folder not found";
    state = runtime_label(state_text, 0.0F);

    if (row == NULL || open == NULL || path == NULL || state == NULL) {
        return row;
    }

    gtk_widget_add_css_class(row, "umicom-runtime-command-row");
    gtk_widget_add_css_class(path, "umicom-runtime-muted");
    gtk_widget_add_css_class(state, "umicom-runtime-muted");
    gtk_label_set_ellipsize(GTK_LABEL(path), PANGO_ELLIPSIZE_MIDDLE);
    gtk_widget_set_tooltip_text(path, item->recent.uri);
    gtk_widget_set_halign(open, GTK_ALIGN_FILL);
    gtk_widget_set_sensitive(open, item->available != 0);
    gtk_widget_set_tooltip_text(
        open,
        item->available
            ? item->recent.uri
            : "This recent workspace directory is no longer available.");

    g_object_set_data_full(
        G_OBJECT(open),
        "umicom-command-argument",
        g_strdup(item->recent.uri),
        g_free);

    gtk_box_append(GTK_BOX(row), open);
    gtk_box_append(GTK_BOX(row), path);
    gtk_box_append(GTK_BOX(row), state);
    return row;
}

/* Build the responsive startup surface that introduces common Studio tasks. */
static GtkWidget *runtime_build_welcome_surface(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioWelcomeCentreSnapshot snapshot;
    GtkWidget *page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 18);
    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *heading = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *quick_tools = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    GtkWidget *title = runtime_label("Welcome to Umicom Studio", 0.0F);
    GtkWidget *summary;
    GtkWidget *card_flow = gtk_flow_box_new();
    GtkWidget *create;
    GtkWidget *recent;
    GtkWidget *learn;
    GtkWidget *scroll;
    GtkWidget *empty_recent;
#if 0
    UmiRecentItemSnapshot recent_items[5] = {{0}};
    UmiRecentItemQuery recent_query = {0};
#endif
    UmiRecentItemRegistry *recent_registry;
#if 0
    size_t recent_count = 0U;
#endif
    UmiDeveloperWorkbenchRecentWorkSnapshot recent_work = {0};
    size_t recent_index;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (page == NULL || runtime == NULL || runtime->owner == NULL ||
        header == NULL || heading == NULL || quick_tools == NULL ||
        title == NULL || card_flow == NULL) {
        return page;
    }
    (void)memset(&snapshot, 0, sizeof(snapshot));
    (void)umi_studio_welcome_centre_snapshot(
        umi_studio_ui_services(runtime->owner->ui), &snapshot);
    summary = runtime_label(
        snapshot.summary[0] != '\0'
            ? snapshot.summary
            : "Create a project, open existing work or learn Umicom Framework.",
        0.0F);

    gtk_widget_add_css_class(page, "umicom-welcome-page");
    gtk_widget_add_css_class(title, "umicom-welcome-title");
    gtk_box_append(GTK_BOX(heading), title);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (summary != NULL) {
        gtk_widget_add_css_class(summary, "umicom-welcome-summary");
        gtk_box_append(GTK_BOX(heading), summary);
    }
    gtk_widget_set_hexpand(heading, TRUE);
    runtime_welcome_add_action(
        runtime, quick_tools, "Extensions", "extensions.open",
        "Browse installed and available Studio extensions", 0);
    runtime_welcome_add_action(
        runtime, quick_tools, "Toolchains", "toolchains.open",
        "Inspect compilers, SDKs and build tools", 0);
    runtime_welcome_add_action(
        runtime, quick_tools, "Settings", "settings.open",
        "Change Studio and Framework preferences", 0);
    /* Keeping quick actions below the heading lets the welcome surface narrow
     * without forcing the central work area wider than the monitor. */
    gtk_widget_set_valign(quick_tools, GTK_ALIGN_START);
    gtk_widget_set_halign(quick_tools, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(header), heading);
    gtk_box_append(GTK_BOX(header), quick_tools);
    gtk_box_append(GTK_BOX(page), header);

    /* FlowBox automatically changes from three columns to two or one as the
     * central area narrows, so every card stays visible without clipping. */
    gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(card_flow), 14U);
    gtk_flow_box_set_row_spacing(GTK_FLOW_BOX(card_flow), 14U);
    gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(card_flow), 1U);
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(card_flow), 3U);
    gtk_flow_box_set_homogeneous(GTK_FLOW_BOX(card_flow), TRUE);
    gtk_flow_box_set_selection_mode(
        GTK_FLOW_BOX(card_flow), GTK_SELECTION_NONE);
    gtk_widget_set_hexpand(card_flow, TRUE);
    gtk_widget_set_vexpand(card_flow, TRUE);
    /* Extra centre height belongs below the cards rather than inside them. */
    gtk_widget_set_valign(card_flow, GTK_ALIGN_START);

    create = runtime_welcome_card(
        "Start",
        "Begin with a project template or open files already on your computer.");
    runtime_welcome_add_action(
        runtime, create, "Create New Project", "project.new",
        "Open the Framework project wizard", 1);
    runtime_welcome_add_action(
        runtime, create, "Open Project or File", "file.open",
        "Open a project or source file", 0);
    runtime_welcome_add_action(
        runtime, create, "Open Folder", "workspace.open-folder",
        "Open and index an existing workspace", 0);

    recent = runtime_welcome_card(
        "Recent Work",
        "Projects and folders you open will appear here for quick access.");
    recent_registry = umi_studio_services_recent_items(
        umi_studio_ui_services(runtime->owner->ui));

    /*
     * The previous direct registry rendering is retained for review. Framework
     * now owns availability projection so Studio does not duplicate filesystem
     * checks or the pinned/recent ordering contract.
     */
#if 0
    recent_query.kind = "workspace";
    recent_query.limit = 5U;
    if (recent_registry != NULL) {
        (void)umi_platform_recent_items_registry_query(
            recent_registry,
            &recent_query,
            recent_items,
            sizeof(recent_items) / sizeof(recent_items[0]),
            &recent_count);
    }

    if (recent_count == 0U) {
        empty_recent = runtime_label(
            "No recent project is recorded yet.", 0.0F);
        if (recent != NULL && empty_recent != NULL) {
            gtk_widget_add_css_class(empty_recent, "umicom-runtime-muted");
            gtk_box_append(GTK_BOX(recent), empty_recent);
        }
    } else if (recent != NULL) {
        /* Each button stores its own copied path. The shared command router
         * opens it directly, so resuming work does not show another chooser. */
        for (recent_index = 0U; recent_index < recent_count; ++recent_index) {
            GtkWidget *recent_button = runtime_make_command_button(
                runtime,
                recent_items[recent_index].label,
                "workspace.open-folder",
                recent_items[recent_index].uri);
            if (recent_button == NULL) continue;
            g_object_set_data_full(
                G_OBJECT(recent_button),
                "umicom-command-argument",
                g_strdup(recent_items[recent_index].uri),
                g_free);
            gtk_widget_set_halign(recent_button, GTK_ALIGN_FILL);
            gtk_box_append(GTK_BOX(recent), recent_button);
        }
    }
#endif

    if (recent_registry != NULL) {
        (void)umi_developer_workbench_start_centre_recent_work_snapshot(
            recent_registry,
            "workspace",
            8U,
            &recent_work);
    }

    if (recent_work.count == 0U) {
        empty_recent = runtime_label(
            "No recent project is recorded yet.", 0.0F);
        if (recent != NULL && empty_recent != NULL) {
            gtk_widget_add_css_class(empty_recent, "umicom-runtime-muted");
            gtk_box_append(GTK_BOX(recent), empty_recent);
        }
    } else if (recent != NULL) {
        for (recent_index = 0U;
             recent_index < recent_work.count;
             ++recent_index) {
            GtkWidget *recent_row = runtime_welcome_recent_workspace(
                runtime, &recent_work.items[recent_index]);
            if (recent_row != NULL) {
                gtk_box_append(GTK_BOX(recent), recent_row);
            }
        }
    }

    runtime_welcome_add_action(
        runtime, recent, "Browse for a Workspace", "workspace.open-folder",
        "Choose a folder to add to recent work", 0);

    learn = runtime_welcome_card(
        "Learn and Explore",
        "Build confidence with guided C, GTK4 and Umicom Framework lessons.");
    runtime_welcome_add_action(
        runtime, learn, "Open Interactive Learning", "help.ai-teacher",
        "Open the Studio learning assistant", 1);
    runtime_welcome_add_action(
        runtime, learn, "Read Documentation", "help.documentation",
        "Browse Umicom developer documentation", 0);
    runtime_welcome_add_action(
        runtime, learn, "Keyboard Shortcuts", "help.shortcuts",
        "Review IDE keyboard shortcuts", 0);

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (create != NULL) {
        gtk_flow_box_append(GTK_FLOW_BOX(card_flow), create);
    }
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (recent != NULL) {
        gtk_flow_box_append(GTK_FLOW_BOX(card_flow), recent);
    }
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (learn != NULL) {
        gtk_flow_box_append(GTK_FLOW_BOX(card_flow), learn);
    }
    gtk_box_append(GTK_BOX(page), card_flow);

    scroll = runtime_scroll(page);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (scroll != NULL) {
        gtk_widget_add_css_class(scroll, "umicom-welcome-scroll");
        return scroll;
    }
    return page;
}

/* Show the welcome surface only while no ordinary source document is active. */
static void runtime_welcome_update_mode(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiUiWorkbenchState state;
    int show_welcome;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->centre_stack == NULL ||
        runtime->owner == NULL || runtime->owner->ui == NULL) {
        return;
    }

    umi_ui_workbench_state_init(&state);
    /* Apply this branch only when its contract condition is satisfied. */
    if (umi_ui_workbench_state_snapshot(
            umi_studio_ui_workbench(runtime->owner->ui),
            &state) != UMI_STATUS_OK) {
        return;
    }

    show_welcome = state.active_document[0] == '\0' ||
        strcmp(state.active_document, "studio.editor.welcome") == 0;
    gtk_stack_set_visible_child_name(
        GTK_STACK(runtime->centre_stack),
        show_welcome ? "welcome" : "workbench");
}
