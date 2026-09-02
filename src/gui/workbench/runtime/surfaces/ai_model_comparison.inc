/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/ai_model_comparison.inc
 *
 * PURPOSE:
 *   Present a policy-governed request editor and side-by-side model result
 *   summary using the reusable Framework ensemble report.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

static GtkWidget *runtime_build_surface_ai_model_comparison(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Model Comparison");
    GtkWidget *request_title;
    GtkWidget *compare;
    GtkWidget *results_title;
    UmiStudioAiPlatform *platform;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (panel == NULL || runtime == NULL || runtime->owner == NULL ||
        runtime->owner->ui == NULL) {
        return panel;
    }
    platform = umi_studio_services_ai_platform(
        umi_studio_ui_services(runtime->owner->ui));
    runtime->ai_model_summary_label =
        runtime_label("No comparison has run yet.", 0.0F);
    runtime->ai_model_prompt_view = gtk_text_view_new();
    runtime->ai_model_results_view = gtk_text_view_new();
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->ai_model_summary_label != NULL) {
        gtk_widget_add_css_class(
            runtime->ai_model_summary_label, "umicom-runtime-muted");
        gtk_box_append(
            GTK_BOX(panel), runtime->ai_model_summary_label);
    }

    gtk_box_append(
        GTK_BOX(panel),
        runtime_card_new(
            "Compare approved providers",
            "One request can be sent to several registered local and remote "
            "model adapters. Remote requests require confirmation, and every "
            "result keeps its own status so partial failures stay visible."));

    request_title = runtime_section_title("Comparison request");
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (request_title != NULL) {
        gtk_box_append(GTK_BOX(panel), request_title);
    }
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->ai_model_prompt_view != NULL) {
        gtk_text_view_set_wrap_mode(
            GTK_TEXT_VIEW(runtime->ai_model_prompt_view),
            GTK_WRAP_WORD_CHAR);
        gtk_widget_set_size_request(runtime->ai_model_prompt_view, -1, 110);
        gtk_box_append(
            GTK_BOX(panel), runtime_scroll(runtime->ai_model_prompt_view));
    }
    compare = gtk_button_new_with_label("Compare Models");
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (compare != NULL) {
        gtk_widget_add_css_class(compare, "suggested-action");
        g_signal_connect(
            compare,
            "clicked",
            G_CALLBACK(runtime_on_ai_model_compare),
            runtime);
        gtk_box_append(GTK_BOX(panel), compare);
    }

    results_title = runtime_section_title("Provider results");
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (results_title != NULL) {
        gtk_box_append(GTK_BOX(panel), results_title);
    }
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (runtime->ai_model_results_view != NULL) {
        gtk_text_view_set_editable(
            GTK_TEXT_VIEW(runtime->ai_model_results_view), FALSE);
        gtk_text_view_set_wrap_mode(
            GTK_TEXT_VIEW(runtime->ai_model_results_view),
            GTK_WRAP_WORD_CHAR);
        gtk_widget_set_size_request(runtime->ai_model_results_view, -1, 260);
        gtk_box_append(
            GTK_BOX(panel), runtime_scroll(runtime->ai_model_results_view));
        runtime_ai_model_render(
            runtime,
            umi_studio_ai_platform_model_comparison(platform));
    }
    runtime_append_context_actions(runtime, panel, "ai");
    return panel;
}
