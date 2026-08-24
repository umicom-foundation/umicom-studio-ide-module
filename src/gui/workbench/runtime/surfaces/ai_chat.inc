/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/ai_chat.inc
 *
 * PURPOSE:
 *   Build the operational provider-neutral AI coding interaction surface.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

static GtkWidget *runtime_build_surface_ai_chat(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("AI Coding");
    GtkWidget *prompt_label;
    GtkWidget *actions;
    GtkWidget *chat;
    GtkWidget *explain;
    GtkWidget *refactor;
    GtkWidget *tests;
    GtkWidget *result_label;
    UmiStudioAiPlatform *ai;
    char summary[256];

    if (panel == NULL || runtime == NULL || runtime->owner == NULL) {
        runtime_append_context_actions(runtime, panel, "ai");

    return panel;
    }

    ai = umi_studio_services_ai_platform(
        umi_studio_ui_services(runtime->owner->ui));

    runtime->ai_summary_label = runtime_label("", 0.0F);
    runtime->ai_prompt_view = gtk_text_view_new();
    runtime->ai_result_view = gtk_text_view_new();

    (void)snprintf(
        summary,
        sizeof(summary),
        "Provider: %s — RAG result limit: %zu",
        ai != NULL
            ? runtime_safe(
                umi_studio_ai_platform_default_provider(ai))
            : "Unavailable",
        ai != NULL
            ? umi_studio_ai_platform_knowledge_result_limit(ai)
            : 0U);

    if (runtime->ai_summary_label != NULL) {
        gtk_label_set_text(
            GTK_LABEL(runtime->ai_summary_label),
            summary);
        gtk_widget_add_css_class(
            runtime->ai_summary_label,
            "umicom-runtime-muted");
        gtk_box_append(
            GTK_BOX(panel),
            runtime->ai_summary_label);
    }

    {
        static const char *const labels[] = {
            "AI Developer",
            "New Chat"
        };
        static const char *const commands[] = {
            "ide.ai.open-overview",
            "ai.developer.new-chat"
        };
        runtime_panel_add_commands(
            runtime,
            panel,
            labels,
            commands,
            2U);
    }

    gtk_box_append(
        GTK_BOX(panel),
        runtime_card_new(
            "Framework AI boundary",
            "Provider execution, tool policy, patches and approvals remain "
            "Framework-owned. Studio presents prompts, responses and governed "
            "developer actions."));

    prompt_label = runtime_section_title("Request");
    if (prompt_label != NULL) gtk_box_append(GTK_BOX(panel), prompt_label);

    if (runtime->ai_prompt_view != NULL) {
        gtk_text_view_set_wrap_mode(
            GTK_TEXT_VIEW(runtime->ai_prompt_view),
            GTK_WRAP_WORD_CHAR);
        gtk_widget_set_size_request(
            runtime->ai_prompt_view,
            -1,
            120);
        gtk_box_append(
            GTK_BOX(panel),
            runtime_scroll(runtime->ai_prompt_view));
    }

    actions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    chat = gtk_button_new_with_label("Chat");
    explain = gtk_button_new_with_label("Explain");
    refactor = gtk_button_new_with_label("Refactor");
    tests = gtk_button_new_with_label("Generate Tests");

    if (actions != NULL && chat != NULL &&
        explain != NULL && refactor != NULL && tests != NULL) {
        g_signal_connect(
            chat, "clicked",
            G_CALLBACK(runtime_on_ai_chat), runtime);
        g_signal_connect(
            explain, "clicked",
            G_CALLBACK(runtime_on_ai_explain), runtime);
        g_signal_connect(
            refactor, "clicked",
            G_CALLBACK(runtime_on_ai_refactor), runtime);
        g_signal_connect(
            tests, "clicked",
            G_CALLBACK(runtime_on_ai_tests), runtime);

        gtk_box_append(GTK_BOX(actions), chat);
        gtk_box_append(GTK_BOX(actions), explain);
        gtk_box_append(GTK_BOX(actions), refactor);
        gtk_box_append(GTK_BOX(actions), tests);
        gtk_box_append(GTK_BOX(panel), actions);
    }

    result_label = runtime_section_title("Framework response / plan");
    if (result_label != NULL) gtk_box_append(GTK_BOX(panel), result_label);

    if (runtime->ai_result_view != NULL) {
        gtk_text_view_set_editable(
            GTK_TEXT_VIEW(runtime->ai_result_view),
            FALSE);
        gtk_text_view_set_wrap_mode(
            GTK_TEXT_VIEW(runtime->ai_result_view),
            GTK_WRAP_WORD_CHAR);
        gtk_widget_set_size_request(
            runtime->ai_result_view,
            -1,
            150);
        gtk_box_append(
            GTK_BOX(panel),
            runtime_scroll(runtime->ai_result_view));
        runtime_ai_set_result(runtime, runtime->last_ai_result);
    }

    return panel;
}
