/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/ai_chat.inc
 *
 * PURPOSE:
 *   Build the Framework-backed ai chat presentation surface.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

static GtkWidget *runtime_build_surface_ai_chat(UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("AI Coding Chat");
    UmiStudioAiPlatform *ai;
    char value[96];

    if (panel == NULL || runtime == NULL || runtime->owner == NULL) return panel;

    ai = umi_studio_services_ai_platform(
        umi_studio_ui_services(runtime->owner->ui));

    runtime_panel_add_stat(
        panel,
        "Provider",
        ai != NULL
            ? runtime_safe(umi_studio_ai_platform_default_provider(ai))
            : "Unavailable");

    if (ai != NULL) {
        (void)snprintf(value, sizeof(value), "%zu",
                       umi_studio_ai_platform_knowledge_result_limit(ai));
        runtime_panel_add_stat(panel, "RAG result limit", value);
    }

    {
        static const char *const labels[] = {"AI Developer", "New Chat"};
        static const char *const commands[] = {
            "ide.ai.open-overview", "ai.developer.new-chat"
        };
        runtime_panel_add_commands(runtime, panel, labels, commands, 2U);
    }

    gtk_box_append(
        GTK_BOX(panel),
        runtime_card_new(
            "Framework AI boundary",
            "Provider execution, tool policy, patches and approvals remain "
            "Framework-owned. Studio presents and dispatches user intent."));
    return runtime_scroll(panel);
}

