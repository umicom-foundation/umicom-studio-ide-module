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
#if defined(UMICOM_STUDIO_HAS_KANBAN)
#include "umicom/ui/gtk4/kanban_window.h"
#endif

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

typedef struct RuntimeWelcomeRecentBinding {
    UmiStudioFileDialogRequest *owner;
    GWeakRef list;
    char id[128];
    uint64_t revision;
} RuntimeWelcomeRecentBinding;
static void RuntimeWelcomeRecentAction(GtkButton *button, gpointer context);
static void RuntimeWelcomeRecentBindingFree(gpointer context, GClosure *closure)
{
    (void)closure;
    RuntimeWelcomeRecentBinding *binding = context;
    runtime_file_dialog_request_free(binding->owner);
    g_weak_ref_clear(&binding->list);
    g_free(binding);
}
static GtkWidget *RuntimeWelcomeRecentButton(UmiStudioGtkRuntimeChrome *runtime,
    GtkWidget *list, const UmiRecentItemSnapshot *item)
{
    RuntimeWelcomeRecentBinding *binding = g_try_new0(RuntimeWelcomeRecentBinding, 1);
    if (binding == NULL) return NULL;
    binding->owner = runtime_file_dialog_request_new(runtime, 0U);
    if (binding->owner == NULL) { g_free(binding); return NULL; }
    g_weak_ref_init(&binding->list, G_OBJECT(list));
    if (item != NULL) {
        memcpy(binding->id, item->id, sizeof(binding->id));
        binding->revision = item->revision;
    }
    GtkWidget *button = gtk_button_new_with_label(item != NULL ? "Remove from recent projects" : "Refresh recent projects");
    char tag[192];
    if (item != NULL)
        (void)snprintf(tag, sizeof(tag), "studio.welcome.recent.remove.%s", item->id);
    else
        (void)snprintf(tag, sizeof(tag), "%s", "studio.welcome.recent.refresh");
    (void)umi_gtk4_automation_tag_widget(button, tag);
    gtk_widget_set_tooltip_text(button, item != NULL ?
        "Forget this local history entry. Project files and open documents are kept." :
        "Read the current local recent-project list again.");
    g_signal_connect_data(button, "clicked", G_CALLBACK(RuntimeWelcomeRecentAction), binding,
        RuntimeWelcomeRecentBindingFree, 0);
    return button;
}

/* Reuse the copied identity and weak owner held by the history button. The
 * extra action value is copied before replacing rows can destroy that button. */
static GtkWidget *RuntimeWelcomeRecentPinButton(UmiStudioGtkRuntimeChrome *runtime,
    GtkWidget *list, const UmiRecentItemSnapshot *item)
{
    GtkWidget *button = RuntimeWelcomeRecentButton(runtime, list, item);
    if (button == NULL) return NULL;
    gtk_button_set_label(GTK_BUTTON(button), item->pinned ? "Unpin project" : "Pin project");
    gtk_widget_set_tooltip_text(button, item->pinned ?
        "Return this project to recent-use ordering. Project files are kept." :
        "Keep this project ahead of unpinned history. This does not grant workspace trust.");
    char tag[192];
    (void)snprintf(tag, sizeof(tag), "studio.welcome.recent.pin.%s", item->id);
    (void)umi_gtk4_automation_tag_widget(button, tag);
    g_object_set_data(G_OBJECT(button), "studio-recent-pin-action",
        GINT_TO_POINTER(item->pinned ? 2 : 1));
    return button;
}

/*
 * Present one recent workspace with its full path and current availability.
 * Opening still routes through the existing workspace command and service.
 */
static GtkWidget *runtime_welcome_recent_workspace(
    UmiStudioGtkRuntimeChrome *runtime,
    const UmiDeveloperWorkbenchRecentWorkItem *item, GtkWidget *list)
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
    /* Availability belongs to this row as well as the global command policy. */
    g_object_set_data(G_OBJECT(open), "studio-command-unavailable",
        item->available ? NULL : GINT_TO_POINTER(1));
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
    GtkWidget *pin = RuntimeWelcomeRecentPinButton(runtime, list, &item->recent);
    if (pin != NULL) gtk_box_append(GTK_BOX(row), pin);
    GtkWidget *remove = RuntimeWelcomeRecentButton(runtime, list, &item->recent);
    if (remove != NULL) gtk_box_append(GTK_BOX(row), remove);
    return row;
}

/* Build a detached replacement before changing the visible list. Retained
 * buttons carry copied identities and weak host/list references, not a service
 * pointer or a request to delete project files. */
static GtkWidget *RuntimeWelcomeRecentRows(UmiStudioGtkRuntimeChrome *runtime, GtkWidget *list)
{
    GtkWidget *rows = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    UmiDeveloperWorkbenchRecentWorkSnapshot snapshot = {0};
    UmiRecentItemRegistry *registry = umi_studio_services_recent_items(
        umi_studio_ui_services(runtime->owner->ui));
    UmiStatus status = registry != NULL ?
        umi_developer_workbench_start_centre_recent_work_snapshot(registry, "workspace", 5U,
            &snapshot) : UMI_STATUS_UNAVAILABLE;
    if (status != UMI_STATUS_OK || snapshot.count == 0U) {
        GtkWidget *notice = runtime_label(status == UMI_STATUS_OK ?
            "No recent project is recorded yet." : "Recent projects could not be read.", 0.0F);
        gtk_box_append(GTK_BOX(rows), notice);
    } else {
        for (size_t i = 0U; i < snapshot.count; ++i) {
            GtkWidget *row = runtime_welcome_recent_workspace(runtime, &snapshot.items[i], list);
            if (row != NULL) gtk_box_append(GTK_BOX(rows), row);
        }
    }
    return rows;
}
static void RuntimeWelcomeRecentAction(GtkButton *button, gpointer context)
{
    int pinAction = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button), "studio-recent-pin-action"));
    RuntimeWelcomeRecentBinding *binding = context;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(binding->owner, &window);
    GtkWidget *list = GTK_WIDGET(g_weak_ref_get(&binding->list));
    if (runtime == NULL || list == NULL) { g_clear_object(&window); g_clear_object(&list); return; }
    /* Replacing rows can destroy this button and its signal closure. Copy all
     * command data and own a separate weak binding before publishing widgets. */
    UmiStudioFileDialogRequest *owner = runtime_file_dialog_request_new(runtime, 0U);
    if (owner == NULL) { g_clear_object(&window); g_object_unref(list); return; }
    char id[sizeof(binding->id)]; memcpy(id, binding->id, sizeof(id));
    uint64_t revision = binding->revision;
    UmiStatus status = UMI_STATUS_OK;
    /* Preserve the removal-only dispatch for review. Both actions now use
     * Framework transactions while the copied row revision rejects stale clicks. */
#if 0
    if (id[0] != '\0') status = UmiStudioServicesForgetRecentWorkspace(
        umi_studio_ui_services(runtime->owner->ui), id, revision);
#endif
    if (id[0] != '\0') {
        UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
        status = pinAction != 0 ?
            UmiStudioServicesPinRecentWorkspace(services, id, revision, pinAction == 1) :
            UmiStudioServicesForgetRecentWorkspace(services, id, revision);
    }
    if (status == UMI_STATUS_OK || status == UMI_STATUS_INVALID_STATE || status == UMI_STATUS_NOT_FOUND) {
        GtkWidget *rows = RuntimeWelcomeRecentRows(runtime, list);
        g_object_ref_sink(rows);
        GtkWidget *old = gtk_widget_get_first_child(list);
        if (old != NULL) gtk_box_remove(GTK_BOX(list), old);
        GObject *current = NULL;
        runtime = runtime_file_dialog_owner(owner, &current);
        if (runtime != NULL) gtk_box_append(GTK_BOX(list), rows);
        g_clear_object(&current);
        g_object_unref(rows);
    }
    GObject *current = NULL;
    runtime = runtime_file_dialog_owner(owner, &current);
    if (runtime != NULL && id[0] != '\0') {
        /* Keep the former removal wording for review; pinning has its own
         * outcome and shares the same stale-view and storage-failure notices. */
#if 0
        runtime_show_notification(runtime, status == UMI_STATUS_OK ?
            "Removed from recent projects. Project files and open documents were kept." :
            (status == UMI_STATUS_INVALID_STATE || status == UMI_STATUS_NOT_FOUND) ?
            "Recent projects changed. Review the refreshed list before removing an entry." :
            "The history change could not be saved. The recent entry and project files were kept.");
#endif
        const char *success = pinAction == 1 ? "Project pinned. Workspace trust was not changed." :
            pinAction == 2 ? "Project unpinned. Project files and open documents were kept." :
            "Removed from recent projects. Project files and open documents were kept.";
        runtime_show_notification(runtime, status == UMI_STATUS_OK ? success :
            (status == UMI_STATUS_INVALID_STATE || status == UMI_STATUS_NOT_FOUND) ?
            "Recent projects changed. Review the refreshed list before changing an entry." :
            "The history change could not be saved. The recent entry and project files were kept.");
    }
    g_clear_object(&current);
    runtime_file_dialog_request_free(owner);
    g_clear_object(&window);g_object_unref(list);
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
#if 0 /* Previous inline history presentation retained below. */
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
#endif

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
    /* Make workspace navigation discoverable at startup. The existing native
     * actions retain weak host ownership, so welcome controls cannot reopen a
     * palette or layout window after this workbench has been destroyed. */
    GtkWidget *workspace_tools = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    GtkWidget *command_search = runtime_make_local_button(runtime,
        "Command Search", "Find Studio commands and their keyboard shortcuts", 0);
    GtkWidget *layouts = runtime_make_local_button(runtime,
        "Layouts", "Choose and review a workspace layout", 1);
    if (command_search != NULL)
        gtk_box_append(GTK_BOX(workspace_tools), command_search);
    if (layouts != NULL) gtk_box_append(GTK_BOX(workspace_tools), layouts);
    gtk_widget_set_halign(workspace_tools, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(header), workspace_tools);
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
    /* Share Framework's source preview and creation workflow with the CLI;
     * this entry point does not start a build or modify repository history. */
    runtime_welcome_add_action(
        runtime, create, "New Umicom Application", "project.new-application",
        "Review and create an application module in a folder you choose", 0);
/* Separate source-file editing from project-folder opening; the previous combined caption advertised project adoption that file.open never performed. */
#if 0
    runtime_welcome_add_action(
        runtime, create, "Open Project or File", "file.open",
        "Open a project or source file", 0);
    runtime_welcome_add_action(
        runtime, create, "Open Folder", "workspace.open-folder",
        "Open and index an existing workspace", 0);
#endif
    runtime_welcome_add_action(
        runtime, create, "New File", "file.new",
        "Start an editable source file and choose its location when saving", 0);
    runtime_welcome_add_action(
        runtime, create, "Open File", "file.open",
        "Edit an existing source file without opening a project", 0);
    runtime_welcome_add_action(
        runtime, create, "Open Project Folder", "workspace.open-folder",
        "Choose the project folder containing CMakeLists.txt; review Build settings to enable execution", 0);

    recent = runtime_welcome_card(
        "Recent Work",
        "Projects and folders you open will appear here for quick access.");
#if 0 /* Registry access now belongs to the replaceable recent list. */
    recent_registry = umi_studio_services_recent_items(
        umi_studio_ui_services(runtime->owner->ui));
#endif

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

    /* Preserve the former fixed rows for review. A replaceable list now makes
     * history removal and refresh visible without reopening the application. */
#if 0
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

#endif
    GtkWidget *recent_list = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_box_append(GTK_BOX(recent_list), RuntimeWelcomeRecentRows(runtime, recent_list));
    gtk_box_append(GTK_BOX(recent), recent_list);
    GtkWidget *refresh_recent = RuntimeWelcomeRecentButton(runtime, recent_list, NULL);
    if (refresh_recent != NULL) gtk_box_append(GTK_BOX(recent), refresh_recent);

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

#ifdef UMICOM_STUDIO_HAS_LEARNING_CATALOGUE
    /* These local routes use the same verified reader as Guided Learning.
     * They need neither a project nor a configured AI provider. */
    runtime_welcome_add_action(runtime, learn, "Browse local learning libraries",
        "help.learning.catalogue", "Open the shared local learning catalogue", 0);
    runtime_welcome_add_action(runtime, learn, "Read the Studio learning guide",
        "help.learning.guide", "Read the admitted Studio learning guide", 0);
    for (size_t i = 0U; i < sizeof(runtime_learning_product_guides) /
        sizeof(runtime_learning_product_guides[0]); ++i) {
        const RuntimeLearningProductGuide *guide = &runtime_learning_product_guides[i];
        runtime_welcome_add_action(runtime, learn, guide->title, guide->command,
            "Read this topic in the local Studio guide library", 0);
    }
#endif

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
#if defined(UMICOM_STUDIO_HAS_KANBAN)
    /* Framework owns board rules, files and window lifetime. Studio provides
     * an entry point; its editor/workspace lifecycle cannot discard board edits. */
    GtkWidget *planning = runtime_welcome_card("Plan development",
        "Move cards between To do, Doing, Testing, Blocked and Done. Assign developers by name and prepare a reviewed public roadmap.");
    GtkWidget *launch_board = UmiKanbanGtk4CreateLauncher(runtime->owner->window);
    if (planning != NULL && launch_board != NULL) {
        gtk_box_append(GTK_BOX(planning), launch_board);
        GtkWidget *project_board = runtime_make_command_button(runtime, "Open project board", "project.board",
            "Return to this project’s board while choosing its private storage folder separately");
        if (project_board != NULL) {
            g_object_set_data(G_OBJECT(project_board), "umicom-automation-id", "studio.project-board");
            gtk_box_append(GTK_BOX(planning), project_board);
        }
        gtk_flow_box_append(GTK_FLOW_BOX(card_flow), planning);
    }
#endif
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
