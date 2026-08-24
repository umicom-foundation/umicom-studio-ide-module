/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_service_snapshots.inc
 *
 * PURPOSE:
 *   Read existing authoritative Studio/Framework service snapshots for GTK
 *   presentation. No service is recreated and no business state is cached here
 *   beyond the most recent presentation snapshot.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

static UmiStatus runtime_refresh_service_snapshots(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioServices *services;
    UmiStudioViewModels *models;
    UmiStudioTestService *tests;
    UmiStudioSourceControlService *source_control;
    UmiStudioDebuggerService *debugger;
    UmiStudioTerminalService *terminal;
    UmiStudioAiPlatform *ai;
    UmiStatus first_error = UMI_STATUS_OK;
    UmiStatus status;

    if (runtime == NULL || runtime->owner == NULL ||
        runtime->owner->ui == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

    services = umi_studio_ui_services(runtime->owner->ui);
    models = umi_studio_ui_view_models(runtime->owner->ui);

    if (models != NULL) {
        status = umi_studio_view_models_refresh(models);
        if (status == UMI_STATUS_OK) {
            status = umi_studio_view_models_snapshot(
                models,
                &runtime->view_model_snapshot);
        }
        if (status != UMI_STATUS_OK && first_error == UMI_STATUS_OK) {
            first_error = status;
        }
    }

    tests = services != NULL
        ? umi_studio_services_tests(services)
        : NULL;
    if (tests != NULL) {
        status = umi_studio_test_service_snapshot(
            tests,
            &runtime->test_snapshot);
        if (status != UMI_STATUS_OK && first_error == UMI_STATUS_OK) {
            first_error = status;
        }
    }

    source_control = services != NULL
        ? umi_studio_services_source_control(services)
        : NULL;
    if (source_control != NULL) {
        status = umi_studio_source_control_service_snapshot(
            source_control,
            &runtime->source_control_snapshot);
        if (status != UMI_STATUS_OK && first_error == UMI_STATUS_OK) {
            first_error = status;
        }
    }

    debugger = services != NULL
        ? umi_studio_services_debugger(services)
        : NULL;
    if (debugger != NULL) {
        status = umi_studio_debugger_service_snapshot(
            debugger,
            &runtime->debugger_snapshot);
        if (status != UMI_STATUS_OK && first_error == UMI_STATUS_OK) {
            first_error = status;
        }
    }

    terminal = services != NULL
        ? umi_studio_services_terminal(services)
        : NULL;
    if (terminal != NULL) {
        status = umi_studio_terminal_service_snapshot(
            terminal,
            &runtime->terminal_snapshot);
        if (status != UMI_STATUS_OK && first_error == UMI_STATUS_OK) {
            first_error = status;
        }
    }

    ai = services != NULL
        ? umi_studio_services_ai_platform(services)
        : NULL;
    if (ai != NULL) {
        status = umi_studio_ai_platform_snapshot(
            ai,
            &runtime->ai_snapshot);
        if (status != UMI_STATUS_OK && first_error == UMI_STATUS_OK) {
            first_error = status;
        }
    }

    return first_error;
}
