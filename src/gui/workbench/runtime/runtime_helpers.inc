/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_helpers.inc
 *
 * PURPOSE:
 *   Small bounded GTK/runtime helpers shared by the private Studio host.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static void runtime_clear_box(GtkWidget *box)
{
    GtkWidget *child;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (box == NULL) return;

    child = gtk_widget_get_first_child(box);
    /*
     * Continue only while work remains available; the loop body advances the state on each
     * pass.
     */
    while (child != NULL) {
        GtkWidget *next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(box), child);
        child = next;
    }
}

/*
 * Provide the runtime clear list box operation used by this module and its client
 * applications.
 */
static void runtime_clear_list_box(GtkWidget *list)
{
    GtkWidget *child;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (list == NULL) return;

    child = gtk_widget_get_first_child(list);
    /*
     * Continue only while work remains available; the loop body advances the state on each
     * pass.
     */
    while (child != NULL) {
        GtkWidget *next = gtk_widget_get_next_sibling(child);
        gtk_list_box_remove(GTK_LIST_BOX(list), child);
        child = next;
    }
}

/* Provide the runtime label operation used by this module and its client applications. */
static GtkWidget *runtime_label(const char *text, float xalign)
{
    GtkWidget *label = gtk_label_new(text != NULL ? text : "");
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (label != NULL) {
        gtk_label_set_xalign(GTK_LABEL(label), xalign);
        gtk_label_set_wrap(GTK_LABEL(label), TRUE);
        gtk_widget_set_hexpand(label, TRUE);
    }
    return label;
}

/*
 * Provide the runtime section title operation used by this module and its client
 * applications.
 */
static GtkWidget *runtime_section_title(const char *title)
{
    GtkWidget *label = runtime_label(title, 0.0F);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (label != NULL) gtk_widget_add_css_class(label, "umicom-runtime-section-title");
    return label;
}

/* Provide the runtime card new operation used by this module and its client applications. */
static GtkWidget *runtime_card_new(const char *title, const char *body)
{
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    GtkWidget *heading = runtime_section_title(title);
    GtkWidget *detail = runtime_label(body, 0.0F);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (box == NULL || heading == NULL || detail == NULL) return box;
    gtk_widget_add_css_class(box, "umicom-runtime-card");
    gtk_box_append(GTK_BOX(box), heading);
    gtk_box_append(GTK_BOX(box), detail);
    return box;
}

/* Provide the runtime scroll operation used by this module and its client applications. */
static GtkWidget *runtime_scroll(GtkWidget *child)
{
    GtkWidget *scroll = gtk_scrolled_window_new();
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (scroll == NULL) return NULL;
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), child);
    gtk_widget_set_hexpand(scroll, TRUE);
    gtk_widget_set_vexpand(scroll, TRUE);
    return scroll;
}

/*
 * Provide the runtime mapped studio command operation used by this module and its client
 * applications.
 */
static const char *runtime_mapped_studio_command(const char *command_id)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (command_id == NULL) return NULL;

    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "file.save-all") == 0) {
        return UMI_STUDIO_COMMAND_DOCUMENTS_SAVE_ALL;
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "build.configure") == 0) {
        return UMI_STUDIO_COMMAND_BUILD_CONFIGURE;
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "build.build") == 0) {
        return UMI_STUDIO_COMMAND_BUILD_COMPILE;
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "build.clean") == 0) {
        return UMI_STUDIO_COMMAND_BUILD_CLEAN;
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "build.install") == 0) {
        return UMI_STUDIO_COMMAND_BUILD_INSTALL;
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "run.start") == 0) {
        return UMI_STUDIO_COMMAND_BUILD_RUN;
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "test.run-all") == 0) {
        return UMI_STUDIO_COMMAND_TESTS_RUN_ALL;
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "test.rerun-failed") == 0) {
        return UMI_STUDIO_COMMAND_TESTS_RERUN_FAILED;
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "test.stop") == 0) {
        return UMI_STUDIO_COMMAND_TESTS_STOP;
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "debug.start") == 0) {
        return UMI_STUDIO_COMMAND_DEBUG_START;
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "source-control.refresh") == 0) {
        return UMI_STUDIO_COMMAND_VCS_REFRESH;
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "source-control.stage-all") == 0) {
        return UMI_STUDIO_COMMAND_VCS_STAGE_ALL;
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "source-control.commit") == 0) {
        return UMI_STUDIO_COMMAND_VCS_COMMIT;
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "source-control.pull") == 0) {
        return UMI_STUDIO_COMMAND_VCS_PULL;
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "source-control.push") == 0) {
        return UMI_STUDIO_COMMAND_VCS_PUSH;
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "source-control.diff") == 0) {
        return UMI_STUDIO_COMMAND_VCS_DIFF;
    }

    return NULL;
}

/*
 * Provide the runtime command enabled operation used by this module and its client
 * applications.
 */
static int runtime_command_enabled(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *command_id)
{
    UmiStudioRuntimeBindings *bindings;
    const char *mapped;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL ||
        runtime->runtime_bootstrap == NULL ||
        runtime->runtime_bootstrap->platform == NULL ||
        command_id == NULL ||
        command_id[0] == '\0') {
        return 0;
    }

    bindings = umi_studio_runtime_platform_bindings(
        runtime->runtime_bootstrap->platform);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (bindings == NULL || bindings->commands == NULL) return 0;

    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (umi_command_registry_contains(bindings->commands, command_id)) {
        return umi_command_registry_is_enabled(
            bindings->commands,
            command_id,
            NULL);
    }

    /*
     * File and workspace pickers are operational GTK entry points whose path
     * selection happens before the Framework document/workspace service call.
     */
    if (strcmp(command_id, "file.open") == 0 ||
        strcmp(command_id, "workspace.open-folder") == 0 ||
        strcmp(command_id, "application.exit") == 0 ||
        strcmp(command_id, "studio.command.search") == 0 ||
        strcmp(command_id, "studio.layout.manager") == 0 ||
        strcmp(command_id, "help.ai-teacher") == 0 ||
        strcmp(command_id, "help.documentation") == 0) {
        return 1;
    }

    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "file.save") == 0 ||
        strcmp(command_id, "file.close") == 0) {
        UmiDocumentWorkingCopySnapshot active;
        return runtime->owner != NULL &&
            runtime->owner->ui != NULL &&
            umi_document_coordinator_active_snapshot(
                umi_studio_ui_documents(runtime->owner->ui),
                &active) == UMI_STATUS_OK;
    }

    mapped = runtime_mapped_studio_command(command_id);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (mapped != NULL &&
        umi_command_registry_contains(bindings->commands, mapped)) {
        /*
         * Argument-requiring mapped commands such as Commit and Diff are
         * operational even when their enabled predicate cannot be evaluated
         * until GTK has collected the actual message/path.
         */
        if (strcmp(command_id, "source-control.commit") == 0) {
            return 1;
        }
        /* Use the stable identifier comparison to choose the matching record or policy. */
        if (strcmp(command_id, "source-control.diff") == 0) {
            return runtime->selected_vcs_path[0] != '\0';
        }
        return umi_command_registry_is_enabled(
            bindings->commands,
            mapped,
            NULL);
    }

    return 0;
}

/* Provide the runtime safe operation used by this module and its client applications. */
static const char *runtime_safe(const char *text)
{
    return text != NULL ? text : "";
}

/*
 * Provide the runtime bounded badge operation used by this module and its client
 * applications.
 */
static uint32_t runtime_bounded_badge(size_t value)
{
    return value > UINT32_MAX ? UINT32_MAX : (uint32_t)value;
}
