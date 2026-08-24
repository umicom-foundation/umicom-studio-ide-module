/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/ai.inc
 *
 * PURPOSE:
 *   Make the existing provider-neutral Studio/Framework AI coding commands
 *   directly usable from the GTK4 workbench while retaining approval and patch
 *   policy inside Framework.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

static char *runtime_text_view_copy_all(GtkWidget *text_view)
{
    GtkTextBuffer *buffer;
    GtkTextIter start;
    GtkTextIter end;

    if (text_view == NULL) return NULL;

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    return gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
}

static void runtime_ai_set_result(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *text)
{
    GtkTextBuffer *buffer;

    if (runtime == NULL) return;

    runtime_copy_selection(
        runtime->last_ai_result,
        sizeof(runtime->last_ai_result),
        text);

    if (runtime->ai_result_view == NULL) return;

    buffer = gtk_text_view_get_buffer(
        GTK_TEXT_VIEW(runtime->ai_result_view));
    gtk_text_buffer_set_text(
        buffer,
        text != NULL ? text : "",
        -1);
}

static void runtime_ai_execute(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *command_id)
{
    char *prompt;
    char message[2048];
    UmiStatus status;

    if (runtime == NULL || command_id == NULL ||
        runtime->ai_prompt_view == NULL) {
        return;
    }

    prompt = runtime_text_view_copy_all(runtime->ai_prompt_view);
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

    if (status == UMI_STATUS_OK) {
        runtime_ai_set_result(
            runtime,
            message[0] != '\0'
                ? message
                : "AI coding request accepted.");
    } else {
        runtime_ai_set_result(runtime, umi_status_text(status));
    }

    g_free(prompt);
}

static void runtime_on_ai_chat(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    runtime_ai_execute(
        (UmiStudioGtkRuntimeChrome *)user_data,
        UMI_STUDIO_COMMAND_AI_CODE_CHAT);
}

static void runtime_on_ai_explain(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    runtime_ai_execute(
        (UmiStudioGtkRuntimeChrome *)user_data,
        UMI_STUDIO_COMMAND_AI_EXPLAIN_CODE);
}

static void runtime_on_ai_refactor(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    runtime_ai_execute(
        (UmiStudioGtkRuntimeChrome *)user_data,
        UMI_STUDIO_COMMAND_AI_REFACTOR_CODE);
}

static void runtime_on_ai_tests(
    GtkButton *button,
    gpointer user_data)
{
    (void)button;
    runtime_ai_execute(
        (UmiStudioGtkRuntimeChrome *)user_data,
        UMI_STUDIO_COMMAND_AI_GENERATE_TESTS);
}

static UmiStatus runtime_ai_patch_approve_accepted(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *patch_id)
{
    return runtime_execute_registered_command(
        runtime,
        UMI_STUDIO_COMMAND_AI_PATCH_APPROVE,
        patch_id);
}

static UmiStatus runtime_ai_patch_apply_accepted(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *patch_id)
{
    return runtime_execute_registered_command(
        runtime,
        UMI_STUDIO_COMMAND_AI_PATCH_APPLY,
        patch_id);
}

static UmiStatus runtime_ai_patch_revert_accepted(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *patch_id)
{
    return runtime_execute_registered_command(
        runtime,
        UMI_STUDIO_COMMAND_AI_PATCH_REVERT,
        patch_id);
}

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
