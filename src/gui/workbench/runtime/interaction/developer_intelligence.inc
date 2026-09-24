/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/interaction/developer_intelligence.inc
 *
 * PURPOSE:
 *   Present Framework Language + AI developer-intelligence readiness through
 *   Studio's existing status strip without duplicating AI or language engines.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/developer_intelligence/service.h"

static double RuntimeDeveloperIntelligenceHealthPercent(
    size_t healthy,
    size_t total)
{
    if (total == 0U) return 0.0;
    if (healthy >= total) return 100.0;
    return ((double)healthy / (double)total) * 100.0;
}

/*
 * Reuse AuthorEngine's authoritative provider/runtime/context snapshot as the
 * evidence for Framework model-routing and context-budget policies.
 */
static void runtime_developer_intelligence_update_status(
    UmiStudioGtkRuntimeChrome *runtime)
{
    UmiDeveloperIntelligenceInput routeInput;
    UmiDeveloperIntelligenceInput budgetInput;
    UmiDeveloperIntelligenceSnapshot routeSnapshot;
    UmiDeveloperIntelligenceSnapshot budgetSnapshot;
    GtkWidget *label;
    double health;
    const char *routeText;
    char text[192];
    char tooltip[256];
    UmiStatus routeStatus;
    UmiStatus budgetStatus;

    if (runtime == NULL) return;
    label = runtime->status_labels[UMI_STUDIO_STATUS_AI];
    if (label == NULL) return;

    health = RuntimeDeveloperIntelligenceHealthPercent(
        runtime->ai_snapshot.healthy_runtimes,
        runtime->ai_snapshot.runtimes);

    UmiDeveloperIntelligenceInputInit(&routeInput);
    routeInput.lexicalScore = health;
    routeInput.semanticScore = health;
    routeInput.contextScore =
        runtime->ai_snapshot.context_sources > 0U ? 90.0 : 55.0;
    routeInput.confidence = health;
    routeInput.risk = runtime->ai_snapshot.remote_allowed ? 15.0 : 5.0;
    routeInput.matched = runtime->ai_snapshot.healthy_runtimes;
    routeInput.total = runtime->ai_snapshot.runtimes;
    routeInput.sourceCount = runtime->ai_snapshot.context_sources;
    routeInput.localAvailable =
        runtime->ai_snapshot.healthy_runtimes > 0U;
    routeInput.remoteAvailable =
        runtime->ai_snapshot.providers > 0U;
    routeInput.remoteAllowed =
        runtime->ai_snapshot.remote_allowed;
    routeInput.approved = 1;
    routeInput.trusted = 1;
    routeInput.active =
        runtime->ai_snapshot.healthy_runtimes > 0U;
    routeInput.revision = runtime->ai_snapshot.revision;

    routeStatus = UmiDeveloperIntelligenceServiceEvaluate(
        "ai.model-route",
        &routeInput,
        &routeSnapshot);

    UmiDeveloperIntelligenceInputInit(&budgetInput);
    budgetInput.lexicalScore = 90.0;
    budgetInput.semanticScore = 90.0;
    budgetInput.contextScore = 90.0;
    budgetInput.confidence = 90.0;
    budgetInput.risk = 5.0;
    budgetInput.usedUnits =
        runtime->ai_snapshot.reserved_output_tokens;
    budgetInput.budgetUnits =
        runtime->ai_snapshot.context_limit;
    budgetInput.localAvailable =
        runtime->ai_snapshot.healthy_runtimes > 0U;
    budgetInput.remoteAvailable =
        runtime->ai_snapshot.providers > 0U;
    budgetInput.remoteAllowed =
        runtime->ai_snapshot.remote_allowed;
    budgetInput.approved = 1;
    budgetInput.trusted = 1;
    budgetInput.active = 1;
    budgetInput.revision = runtime->ai_snapshot.revision;

    budgetStatus = UmiDeveloperIntelligenceServiceEvaluate(
        "ai.context-budget",
        &budgetInput,
        &budgetSnapshot);

    if (routeStatus == UMI_STATUS_OK && routeSnapshot.localPreferred) {
        routeText = "local";
    } else if (routeStatus == UMI_STATUS_OK && routeSnapshot.remoteUsable) {
        routeText = "remote";
    } else {
        routeText = "unavailable";
    }

    (void)snprintf(
        text,
        sizeof(text),
        "AI %u/%u healthy  %u context  %s",
        (unsigned)runtime_bounded_badge(
            runtime->ai_snapshot.healthy_runtimes),
        (unsigned)runtime_bounded_badge(
            runtime->ai_snapshot.runtimes),
        (unsigned)runtime_bounded_badge(
            runtime->ai_snapshot.context_sources),
        routeText);
    gtk_label_set_text(GTK_LABEL(label), text);

    (void)snprintf(
        tooltip,
        sizeof(tooltip),
        "Providers: %u; runtimes: %u; healthy: %u; context limit: %u; "
        "reserved output: %u; remaining context budget: %llu",
        (unsigned)runtime_bounded_badge(runtime->ai_snapshot.providers),
        (unsigned)runtime_bounded_badge(runtime->ai_snapshot.runtimes),
        (unsigned)runtime_bounded_badge(runtime->ai_snapshot.healthy_runtimes),
        runtime->ai_snapshot.context_limit,
        runtime->ai_snapshot.reserved_output_tokens,
        (unsigned long long)(
            budgetStatus == UMI_STATUS_OK
                ? budgetSnapshot.budgetRemaining
                : 0U));
    gtk_widget_set_tooltip_text(label, tooltip);

    if ((routeStatus == UMI_STATUS_OK && !routeSnapshot.ready) ||
        (budgetStatus == UMI_STATUS_OK && budgetSnapshot.attention)) {
        gtk_widget_add_css_class(label, "warning");
    } else {
        gtk_widget_remove_css_class(label, "warning");
    }
}
