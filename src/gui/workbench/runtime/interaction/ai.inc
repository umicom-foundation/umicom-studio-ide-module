/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/ai.inc
 *
 * PURPOSE:
 *   Make the existing provider-neutral Studio/Framework AI coding commands
 *   directly usable from the GTK4 workbench while retaining approval and patch
 *   policy inside Framework.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static char *runtime_text_view_copy_all(GtkWidget *text_view)
{
    GtkTextBuffer *buffer;
    GtkTextIter start;
    GtkTextIter end;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (text_view == NULL) return NULL;

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    return gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
}

/*
 * Provide the runtime ai set result operation used by this module and its client
 * applications.
 */
static void runtime_ai_set_result(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *text)
{
    GtkTextBuffer *buffer;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL) return;

    runtime_copy_selection(
        runtime->last_ai_result,
        sizeof(runtime->last_ai_result),
        text);

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->ai_result_view == NULL) return;

    buffer = gtk_text_view_get_buffer(
        GTK_TEXT_VIEW(runtime->ai_result_view));
    gtk_text_buffer_set_text(
        buffer,
        text != NULL ? text : "",
        -1);
}

/*
 * Perform runtime ai through the module contract so client applications do not duplicate
 * its policy.
 */
static void runtime_ai_execute(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *command_id)
{
    char *prompt;
    char message[2048];
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || command_id == NULL ||
        runtime->ai_prompt_view == NULL) {
        return;
    }

    prompt = runtime_text_view_copy_all(runtime->ai_prompt_view);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (prompt == NULL || prompt[0] == '\0') {
        g_free(prompt);
        runtime_show_notification(
            runtime,
            "Enter an AI coding request first.");
        return;
    }

    (void)memset(message, 0, sizeof(message));
    status = runtime_execute_registered_command_capture(
        runtime,
        command_id,
        prompt,
        message,
        sizeof(message));

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        runtime_ai_set_result(
            runtime,
            message[0] != '\0'
                ? message
                : "AI coding request accepted.");
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        runtime_ai_set_result(runtime, umi_status_text(status));
    }

    g_free(prompt);
}

/*
 * Provide the runtime on ai chat operation used by this module and its client
 * applications.
 */
static void runtime_on_ai_chat(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    runtime_ai_execute(
        (UmiStudioGtkRuntimeChrome *)user_data,
        UMI_STUDIO_COMMAND_AI_CODE_CHAT);
}

/*
 * Provide the runtime on ai explain operation used by this module and its client
 * applications.
 */
static void runtime_on_ai_explain(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    runtime_ai_execute(
        (UmiStudioGtkRuntimeChrome *)user_data,
        UMI_STUDIO_COMMAND_AI_EXPLAIN_CODE);
}

/*
 * Provide the runtime on ai refactor operation used by this module and its client
 * applications.
 */
static void runtime_on_ai_refactor(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    runtime_ai_execute(
        (UmiStudioGtkRuntimeChrome *)user_data,
        UMI_STUDIO_COMMAND_AI_REFACTOR_CODE);
}

/*
 * Provide the runtime on ai tests operation used by this module and its client
 * applications.
 */
static void runtime_on_ai_tests(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    runtime_ai_execute(
        (UmiStudioGtkRuntimeChrome *)user_data,
        UMI_STUDIO_COMMAND_AI_GENERATE_TESTS);
}

/*
 * Provide the runtime ai model render operation used by this module and its client
 * applications.
 */
static void runtime_ai_model_render(
    UmiStudioGtkRuntimeChrome *runtime,
    const UmiAiModelEnsembleReport *report)
{
    GtkTextBuffer *buffer;
    GString *text;
    size_t index;
    char summary[160];

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->ai_model_results_view == NULL ||
        report == NULL || report->structure_size < sizeof(*report) ||
        report->api_version != UMI_AI_MODEL_ENSEMBLE_API_VERSION ||
        report->result_count > UMI_AI_MODEL_ENSEMBLE_MAX_TARGETS) {
        return;
    }

    /* GString grows safely, avoiding fixed output buffers for model answers. */
    text = g_string_new(NULL);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (text == NULL) return;
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < report->result_count; ++index) {
        const UmiAiModelResult *result = &report->results[index];
        g_string_append_printf(
            text,
            "%s%s\nProvider: %s\nModel: %s\nStatus: %s\n",
            index == report->selected_index ? "Selected · " : "",
            result->target.label[0] != '\0'
                ? result->target.label
                : "Model answer",
            result->target.provider_id,
            result->target.model_id,
            umi_status_text(result->status));
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (result->status == UMI_STATUS_OK) {
            g_string_append(text, result->response.text);
        } else /* Preserve the original failure result so the caller can respond to the correct cause. */ if (result->status == UMI_STATUS_PERMISSION_DENIED) {
            g_string_append(
                text,
                "This provider was blocked by the current approval policy.");
        } /* Use this fallback path when the earlier condition does not apply. */ else {
            g_string_append(
                text,
                "No answer was returned by this provider adapter.");
        }
        g_string_append(text, "\n\n");
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (report->result_count == 0U) {
        g_string_append(
            text,
            "No provider has run yet. Register a local or remote adapter, "
            "enter a request and choose Compare Models.");
    }

    buffer = gtk_text_view_get_buffer(
        GTK_TEXT_VIEW(runtime->ai_model_results_view));
    gtk_text_buffer_set_text(buffer, text->str, -1);
    g_string_free(text, TRUE);

    (void)snprintf(
        summary,
        sizeof(summary),
        "%zu provider result%s · %zu successful",
        report->result_count,
        report->result_count == 1U ? "" : "s",
        report->success_count);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->ai_model_summary_label != NULL) {
        gtk_label_set_text(
            GTK_LABEL(runtime->ai_model_summary_label), summary);
    }
}

/*
 * Provide the runtime ai has remote provider operation used by this module and its client
 * applications.
 */
static int runtime_ai_has_remote_provider(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiStudioAiPlatform *platform;
    UmiAiRuntime *ai_runtime;
    size_t index;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->owner == NULL ||
        runtime->owner->ui == NULL) {
        return 0;
    }
    platform = umi_studio_services_ai_platform(
        umi_studio_ui_services(runtime->owner->ui));
    ai_runtime = umi_studio_ai_platform_runtime(platform);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (ai_runtime == NULL) return 0;
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < ai_runtime->providers.count; ++index) {
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (ai_runtime->providers.providers[index].kind ==
            UMI_AI_PROVIDER_REMOTE) {
            return 1;
        }
    }
    return 0;
}

/*
 * Perform runtime ai model compare through the module contract so client applications do
 * not duplicate its policy.
 */
static UmiStatus runtime_ai_model_compare_execute(
    UmiStudioGtkRuntimeChrome *runtime,
    int remote_approved)
{
    UmiStudioAiPlatform *platform;
    UmiAiRuntime *ai_runtime;
    UmiStudioAiWorkbenchProfile profile;
    UmiAiModelTarget targets[UMI_AI_MODEL_ENSEMBLE_MAX_TARGETS];
    char *prompt;
    size_t target_count = 0U;
    size_t index;
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime == NULL || runtime->owner == NULL ||
        runtime->owner->ui == NULL ||
        runtime->ai_model_prompt_view == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    prompt = runtime_text_view_copy_all(runtime->ai_model_prompt_view);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (prompt == NULL || prompt[0] == '\0') {
        g_free(prompt);
        runtime_show_notification(
            runtime, "Enter a comparison request first.");
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    platform = umi_studio_services_ai_platform(
        umi_studio_ui_services(runtime->owner->ui));
    ai_runtime = umi_studio_ai_platform_runtime(platform);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (platform == NULL || ai_runtime == NULL) {
        g_free(prompt);
        return UMI_STATUS_INVALID_STATE;
    }
    (void)memset(&profile, 0, sizeof(profile));
    (void)umi_studio_ai_platform_workbench_profile(platform, &profile);

    /* Build targets only from registered adapters; descriptors alone cannot run. */
    for (index = 0U;
         index < ai_runtime->providers.count &&
         target_count < UMI_AI_MODEL_ENSEMBLE_MAX_TARGETS;
         ++index) {
        const UmiAiProvider *provider =
            &ai_runtime->providers.providers[index];
        const char *model_id = provider->provider_id;
        const char *label = umi_ai_provider_kind_text(provider->kind);
        /*
         * Protect caller-owned memory by checking that required state is available before it is
         * used.
         */
        if (provider->provider_id == NULL || provider->provider_id[0] == '\0') {
            continue;
        }
        /* Apply this branch only when its contract condition is satisfied. */
        if (provider->kind == UMI_AI_PROVIDER_REMOTE &&
            profile.remote_configured &&
            strcmp(provider->provider_id, profile.remote_provider_id) == 0 &&
            profile.remote_model_id[0] != '\0') {
            model_id = profile.remote_model_id;
        } else /* Use the stable identifier comparison to choose the matching record or policy. */ if (strcmp(provider->provider_id, "studio.local") == 0) {
            model_id = "studio-reference";
        }
        status = umi_ai_model_target_initialize(
            &targets[target_count], provider->provider_id, model_id, label,
            provider->kind == UMI_AI_PROVIDER_REMOTE && remote_approved);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) {
            g_free(prompt);
            return status;
        }
        ++target_count;
    }
    /* Apply this branch only when its contract condition is satisfied. */
    if (target_count == 0U) {
        g_free(prompt);
        runtime_show_notification(runtime, "No AI provider adapter is registered.");
        return UMI_STATUS_NOT_FOUND;
    }

    status = umi_studio_ai_platform_compare_models(
        platform, prompt, targets, target_count);
    runtime_ai_model_render(
        runtime, umi_studio_ai_platform_model_comparison(platform));
    runtime_show_notification(
        runtime,
        status == UMI_STATUS_OK
            ? "Model comparison completed."
            : umi_status_text(status));
    g_free(prompt);
    return status;
}

/*
 * Provide the runtime ai model compare remote accepted operation used by this module and
 * its client applications.
 */
static UmiStatus runtime_ai_model_compare_remote_accepted(
    UmiStudioGtkRuntimeChrome *runtime)
{
    /* The confirmation click is the explicit approval for this remote request. */
    return runtime_ai_model_compare_execute(runtime, 1);
}

/*
 * Provide the runtime on ai model compare operation used by this module and its client
 * applications.
 */
static void runtime_on_ai_model_compare(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    (void)button;
    /* Apply this branch only when its contract condition is satisfied. */
    if (runtime_ai_has_remote_provider(runtime)) {
        (void)runtime_confirm(
            runtime,
            "Send Request to Approved Remote Models?",
            "This comparison includes a remote provider. The request and "
            "approved workspace context may leave this computer. Continue?",
            "Compare Models",
            runtime_ai_model_compare_remote_accepted);
    } /* Use this fallback path when the earlier condition does not apply. */ else {
        (void)runtime_ai_model_compare_execute(runtime, 0);
    }
}

/*
 * Provide the runtime ai patch approve accepted operation used by this module and its
 * client applications.
 */
static UmiStatus runtime_ai_patch_approve_accepted(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *patch_id)
{
    return runtime_execute_registered_command(
        runtime,
        UMI_STUDIO_COMMAND_AI_PATCH_APPROVE,
        patch_id);
}

/*
 * Provide the runtime ai patch apply accepted operation used by this module and its client
 * applications.
 */
static UmiStatus runtime_ai_patch_apply_accepted(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *patch_id)
{
    return runtime_execute_registered_command(
        runtime,
        UMI_STUDIO_COMMAND_AI_PATCH_APPLY,
        patch_id);
}

/*
 * Provide the runtime ai patch revert accepted operation used by this module and its
 * client applications.
 */
static UmiStatus runtime_ai_patch_revert_accepted(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *patch_id)
{
    return runtime_execute_registered_command(
        runtime,
        UMI_STUDIO_COMMAND_AI_PATCH_REVERT,
        patch_id);
}

/*
 * Provide the runtime on ai patch approve operation used by this module and its client
 * applications.
 */
static void runtime_on_ai_patch_approve(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    (void)runtime_prompt_text(
        (UmiStudioGtkRuntimeChrome *)user_data,
        "Approve AI Patch",
        "Enter the Framework patch identifier to approve.",
        "",
        "Approve",
        runtime_ai_patch_approve_accepted);
}

/*
 * Perform runtime on ai patch through the module contract so client applications do not
 * duplicate its policy.
 */
static void runtime_on_ai_patch_apply(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    (void)runtime_prompt_text(
        (UmiStudioGtkRuntimeChrome *)user_data,
        "Apply AI Patch",
        "Enter the approved Framework patch identifier to apply.",
        "",
        "Apply",
        runtime_ai_patch_apply_accepted);
}

/*
 * Provide the runtime on ai patch revert operation used by this module and its client
 * applications.
 */
static void runtime_on_ai_patch_revert(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    (void)runtime_prompt_text(
        (UmiStudioGtkRuntimeChrome *)user_data,
        "Revert AI Patch",
        "Enter the Framework patch identifier to revert.",
        "",
        "Revert",
        runtime_ai_patch_revert_accepted);
}
