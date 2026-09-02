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

    return runtime_execute_phase1_host_command(runtime, command_id, argument);
}
