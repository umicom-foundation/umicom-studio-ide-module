/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/command_router.inc
 *
 * PURPOSE:
 *   Bridge Framework Application Shell command identifiers to operational
 *   Studio actions when GTK must first collect a file path, folder, commit
 *   message or other user input. All other commands remain on the Framework
 *   Studio Runtime Host Controller.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static UmiStatus runtime_route_vcs_commit_accepted(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *message)
{
    return runtime_execute_registered_command(
        runtime,
        UMI_STUDIO_COMMAND_VCS_COMMIT,
        message);
}

/*
 * Provide the runtime route command operation used by this module and its client
 * applications.
 */
static UmiStatus runtime_route_command(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *command_id,
    const char *argument)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || command_id == NULL ||
        command_id[0] == '\0') {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    /* Layout commands use the Data Server checkpoint, not the older shell
     * session file, and never replace source-document saving. */
    if (strcmp(command_id, "studio.layout.save") == 0)
        return runtime_workspace_save(runtime);
    if (strcmp(command_id, "studio.layout.restore") == 0)
        return runtime_workspace_restore(runtime);
    /* Reject before a legacy named-layout command can mutate shell state. */
    if ((strncmp(command_id, "studio.layout.", 14U) == 0 &&
         strcmp(command_id, "studio.layout.manager") != 0) ||
        strcmp(command_id, "designer.open") == 0) {
        UmiUiWorkspaceCustomisation *model = runtime_workspace_model(runtime);
        if (model != NULL && model->edit_active) {
            runtime_show_notification(runtime, "Apply or cancel this layout edit first.");
            return UMI_STATUS_BUSY;
        }
    }

    /* File/workspace commands requiring native GTK path selection. */
    if (strcmp(command_id, "file.open") == 0) {
        return runtime_choose_file_to_open(runtime);
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "workspace.open-folder") == 0) {
        /* Recent-work actions already know their path, while ordinary menu
         * actions still open the native folder chooser. */
        return argument != NULL && argument[0] != '\0'
            ? runtime_open_workspace_path(runtime, argument)
            : runtime_choose_workspace_folder(runtime);
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "file.save") == 0) {
        return runtime_document_save(runtime);
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "file.save-as") == 0) {
        return runtime_choose_save_as(runtime);
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "file.save-all") == 0) {
        return runtime_execute_registered_command(
            runtime,
            UMI_STUDIO_COMMAND_DOCUMENTS_SAVE_ALL,
            NULL);
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "file.close") == 0) {
        return runtime_document_close(runtime);
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "application.exit") == 0) {
        gtk_window_close(runtime->owner->window);
        return UMI_STATUS_OK;
    }

    /* Build/run commands reuse the canonical Studio service registrations. */
    if (strcmp(command_id, "build.configure") == 0) {
        return runtime_execute_registered_command(
            runtime,
            UMI_STUDIO_COMMAND_BUILD_CONFIGURE,
            argument);
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "build.build") == 0) {
        return runtime_execute_registered_command(
            runtime,
            UMI_STUDIO_COMMAND_BUILD_COMPILE,
            argument);
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "build.clean") == 0) {
        return runtime_execute_registered_command(
            runtime,
            UMI_STUDIO_COMMAND_BUILD_CLEAN,
            argument);
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "build.install") == 0) {
        return runtime_execute_registered_command(
            runtime,
            UMI_STUDIO_COMMAND_BUILD_INSTALL,
            argument);
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "run.start") == 0) {
        return runtime_execute_registered_command(
            runtime,
            UMI_STUDIO_COMMAND_BUILD_RUN,
            argument);
    }

    /* Professional Test Explorer operations. */
    if (strcmp(command_id, "test.run-all") == 0) {
        return runtime_execute_registered_command(
            runtime,
            UMI_STUDIO_COMMAND_TESTS_RUN_ALL,
            argument);
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "test.rerun-failed") == 0) {
        return runtime_execute_registered_command(
            runtime,
            UMI_STUDIO_COMMAND_TESTS_RERUN_FAILED,
            argument);
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "test.stop") == 0) {
        return runtime_execute_registered_command(
            runtime,
            UMI_STUDIO_COMMAND_TESTS_STOP,
            argument);
    }

    /* Debugging commands use Studio's existing DAP-backed service. */
    if (strcmp(command_id, "debug.start") == 0) {
        return runtime_execute_registered_command(
            runtime,
            UMI_STUDIO_COMMAND_DEBUG_START,
            argument);
    }

    /* Source Control menu commands are mapped to the canonical VCS facade. */
    if (strcmp(command_id, "source-control.refresh") == 0) {
        return runtime_execute_registered_command(
            runtime,
            UMI_STUDIO_COMMAND_VCS_REFRESH,
            argument);
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "source-control.stage-all") == 0) {
        return runtime_execute_registered_command(
            runtime,
            UMI_STUDIO_COMMAND_VCS_STAGE_ALL,
            argument);
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "source-control.commit") == 0) {
        return runtime_prompt_text(
            runtime,
            "Commit Changes",
            "Enter the Git commit message.",
            "",
            "Commit",
            runtime_route_vcs_commit_accepted);
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "source-control.pull") == 0) {
        return runtime_execute_registered_command(
            runtime,
            UMI_STUDIO_COMMAND_VCS_PULL,
            argument);
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "source-control.push") == 0) {
        return runtime_execute_registered_command(
            runtime,
            UMI_STUDIO_COMMAND_VCS_PUSH,
            argument);
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "source-control.diff") == 0 &&
        runtime->selected_vcs_path[0] != '\0') {
        UmiStatus status = runtime_execute_registered_command(
            runtime,
            UMI_STUDIO_COMMAND_VCS_DIFF,
            runtime->selected_vcs_path);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) {
            runtime_show_selected_diff(runtime);
        }
        return status;
    }

    /* Local UI entry points are deliberately not phantom registry commands. */
    if (strcmp(command_id, "studio.command.search") == 0) {
        runtime_show_command_palette(runtime);
        return UMI_STATUS_OK;
    }
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(command_id, "studio.layout.manager") == 0) {
        runtime_show_layout_browser(runtime);
        return UMI_STATUS_OK;
    }

    /* Native named presets use the same professional model as the selector.
     * The shell receives only its accepted one-way projection afterward. */
    if (strncmp(command_id, "studio.layout.", 14U) == 0 ||
        strcmp(command_id, "designer.open") == 0) {
        const char *layout_id = strcmp(command_id, "designer.open") == 0
            ? "visual-design" : command_id + 14U;
        UmiStatus status = runtime_workspace_select_layout(runtime, layout_id);
        if (status == UMI_STATUS_OK && strcmp(layout_id, "code-preview") == 0) {
            runtime_workspace_focus_surface(runtime, UMI_STUDIO_SURFACE_LIVE_PREVIEW);
            (void)runtime_live_preview_refresh(runtime, 1);
        } else if (status == UMI_STATUS_OK && strcmp(layout_id, "visual-design") == 0) {
            runtime->designer_active_surface = UMI_STUDIO_SURFACE_VISUAL_DESIGNER;
            runtime_workspace_focus_surface(runtime, UMI_STUDIO_SURFACE_VISUAL_DESIGNER);
        }
        return status;
    }

    /* Mode commands select an already registered central authoring surface.
     * The same surfaces remain available through the Window catalogue. */
    if (strcmp(command_id, "designer.code") == 0) {
        runtime->designer_active_surface = UMI_STUDIO_SURFACE_DESIGN_CODE;
        runtime_show_surface(runtime, UMI_STUDIO_SURFACE_DESIGN_CODE);
        return UMI_STATUS_OK;
    }
    if (strcmp(command_id, "designer.mixed") == 0) {
        runtime->designer_active_surface = UMI_STUDIO_SURFACE_DESIGN_MIXED;
        runtime_show_surface(runtime, UMI_STUDIO_SURFACE_DESIGN_MIXED);
        return UMI_STATUS_OK;
    }
    if (strcmp(command_id, "designer.preview") == 0) {
        runtime->designer_active_surface = UMI_STUDIO_SURFACE_DESIGN_PREVIEW;
        runtime_show_surface(runtime, UMI_STUDIO_SURFACE_DESIGN_PREVIEW);
        return UMI_STATUS_OK;
    }

    /* Live Preview follows the ordinary editor document and remains separate
     * from the declarative visual designer's Preview mode. */
    if (strcmp(command_id, "view.live-preview") == 0) {
        runtime_show_surface(runtime, UMI_STUDIO_SURFACE_LIVE_PREVIEW);
        (void)runtime_live_preview_refresh(runtime, 1);
        return UMI_STATUS_OK;
    }

    /* Help actions reveal operational workbench panels rather than sending
     * users to a disconnected placeholder or a separate application. */
    if (strcmp(command_id, "help.ai-teacher") == 0) {
        runtime_show_surface(runtime, UMI_STUDIO_SURFACE_LEARNING);
        return UMI_STATUS_OK;
    }
    if (strcmp(command_id, "help.documentation") == 0) {
        runtime_show_surface(runtime, UMI_STUDIO_SURFACE_DOCUMENTATION);
        return UMI_STATUS_OK;
    }

    /* View/activity navigation opens a canonical singleton through the same
     * owner used by the catalogue. No legacy shell placement is imported. */
    {
        size_t index;
        for (index = 0U; index < sizeof(RUNTIME_SURFACE_PROFILES) /
                sizeof(RUNTIME_SURFACE_PROFILES[0]); ++index) {
            const UmiStudioGtkSurfaceProfile *profile = &RUNTIME_SURFACE_PROFILES[index];
            if (profile->command_id[0] != '\0' &&
                strcmp(profile->command_id, command_id) == 0)
                return runtime_workspace_open_surface(runtime, profile->kind, NULL);
        }
    }
    return runtime_execute_phase1_host_command(runtime, command_id, argument);
}
