/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/developer_ecosystem_contribution.c
 * PURPOSE: Place Framework ecosystem commands and views in Studio.
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/developer_ecosystem_contribution.h"

#include <string.h>

#define COMMAND(id, menu_name, group_name, order)                              \
    {                                                                           \
        (uint32_t)sizeof(UmiStudioDeveloperEcosystemCommandContribution),       \
        UMI_STUDIO_DEVELOPER_ECOSYSTEM_CONTRIBUTION_API_VERSION,                \
        id, menu_name, group_name, order, true                                  \
    }

static const UmiStudioDeveloperEcosystemCommandContribution COMMANDS[] = {
    COMMAND("ecosystem.packages.open", "Tools", "packages", 100),
    COMMAND("ecosystem.packages.search", "Navigate", "packages", 110),
    COMMAND("ecosystem.packages.refresh", "Tools", "packages", 120),
    COMMAND("ecosystem.packages.install", "Tools", "packages", 130),
    COMMAND("ecosystem.packages.update", "Tools", "packages", 140),
    COMMAND("ecosystem.packages.remove", "Tools", "packages", 150),
    COMMAND("ecosystem.packages.enable", "Tools", "packages", 160),
    COMMAND("ecosystem.packages.disable", "Tools", "packages", 170),
    COMMAND("ecosystem.packages.pin", "Tools", "packages", 180),
    COMMAND("ecosystem.packages.unpin", "Tools", "packages", 190),
    COMMAND("ecosystem.sources.open", "Tools", "package-sources", 200),
    COMMAND("ecosystem.sources.add-local", "Tools", "package-sources", 210),
    COMMAND("ecosystem.sources.add-offline", "Tools", "package-sources", 220),
    COMMAND("ecosystem.sources.sync", "Tools", "package-sources", 230),
    COMMAND("ecosystem.sources.enable", "Tools", "package-sources", 240),
    COMMAND("ecosystem.sources.disable", "Tools", "package-sources", 250),
    COMMAND("ecosystem.sources.offline-mode", "Tools", "package-sources", 260),
    COMMAND("ecosystem.marketplace.open", "Tools", "extensions", 300),
    COMMAND("ecosystem.marketplace.publisher", "Tools", "extensions", 310),
    COMMAND("ecosystem.marketplace.provenance", "Tools", "extensions", 320),
    COMMAND("ecosystem.sdk.open", "Tools", "sdk", 400),
    COMMAND("ecosystem.sdk.refresh", "Tools", "sdk", 410),
    COMMAND("ecosystem.sdk.install", "Tools", "sdk", 420),
    COMMAND("ecosystem.sdk.update", "Tools", "sdk", 430),
    COMMAND("ecosystem.sdk.remove", "Tools", "sdk", 440),
    COMMAND("ecosystem.sdk.set-default", "Tools", "sdk", 450),
    COMMAND("ecosystem.sdk.repair", "Tools", "sdk", 460),
    COMMAND("ecosystem.component.open", "View", "components", 500),
    COMMAND("ecosystem.component.new", "File", "components", 510),
    COMMAND("ecosystem.component.preview", "View", "components", 520),
    COMMAND("ecosystem.component.generate", "File", "components", 530),
    COMMAND("ecosystem.component.package", "Build", "components", 540),
    COMMAND("ecosystem.migration.open", "View", "migration", 600),
    COMMAND("ecosystem.migration.analyse", "Tools", "migration", 610),
    COMMAND("ecosystem.migration.apply", "Tools", "migration", 620),
    COMMAND("ecosystem.migration.rollback", "Tools", "migration", 630),
    COMMAND("ecosystem.extensions.open", "View", "extension-host", 700),
    COMMAND("ecosystem.extensions.restart-host", "Tools", "extension-host", 710),
    COMMAND("ecosystem.extensions.quarantine", "Tools", "extension-host", 720),
    COMMAND("ecosystem.extensions.recover", "Tools", "extension-host", 730)
};

#undef COMMAND

#define VIEW(id, view_label, contract, placement, order)                       \
    {                                                                           \
        (uint32_t)sizeof(UmiStudioDeveloperEcosystemViewContribution),          \
        UMI_STUDIO_DEVELOPER_ECOSYSTEM_CONTRIBUTION_API_VERSION,                \
        id, view_label, contract, placement, order, true, true                  \
    }

static const UmiStudioDeveloperEcosystemViewContribution VIEWS[] = {
    VIEW("studio.ecosystem.packages", "Package Manager",
        "umicom.developer.ecosystem.package-catalogue", "left-sidebar", 700),
    VIEW("studio.ecosystem.marketplace", "Extension Marketplace",
        "umicom.developer.ecosystem.package-query", "left-sidebar", 710),
    VIEW("studio.ecosystem.installed", "Installed Extensions",
        "umicom.plugin.extension-host.extension-manager-state", "left-sidebar", 720),
    VIEW("studio.ecosystem.sources", "Package Sources",
        "umicom.developer.ecosystem.source-catalogue", "secondary-sidebar", 730),
    VIEW("studio.ecosystem.sdk-manager", "SDK Manager",
        "umicom.developer.ecosystem.sdk-inventory", "editor-area", 740),
    VIEW("studio.ecosystem.components", "Component Catalogue",
        "umicom.developer.ecosystem.component-wizard", "left-sidebar", 750),
    VIEW("studio.ecosystem.component-wizard", "New Component",
        "umicom.developer.ecosystem.component-scaffold-plan", "editor-area", 760),
    VIEW("studio.ecosystem.install-review", "Package Install Review",
        "umicom.developer.ecosystem.install-review", "editor-area", 770),
    VIEW("studio.ecosystem.updates", "Package Updates",
        "umicom.developer.ecosystem.update-review", "bottom-panel", 780),
    VIEW("studio.ecosystem.provenance", "Package Provenance",
        "umicom.developer.ecosystem.provenance-review", "secondary-sidebar", 790),
    VIEW("studio.ecosystem.migrations", "Migration Centre",
        "umicom.developer.ecosystem.migration-review", "bottom-panel", 800),
    VIEW("studio.ecosystem.extension-host", "Extension Host Health",
        "umicom.plugin.extension-host.extension-snapshot", "bottom-panel", 810)
};

#undef VIEW

size_t umi_studio_developer_ecosystem_command_count(void)
{
    return sizeof(COMMANDS) / sizeof(COMMANDS[0]);
}

const UmiStudioDeveloperEcosystemCommandContribution *
umi_studio_developer_ecosystem_command_at(size_t index)
{
    return index < umi_studio_developer_ecosystem_command_count()
        ? &COMMANDS[index] : NULL;
}

const UmiStudioDeveloperEcosystemCommandContribution *
umi_studio_developer_ecosystem_command_find(const char *framework_command_id)
{
    size_t index;
    if (framework_command_id == NULL) return NULL;
    for (index = 0U; index < umi_studio_developer_ecosystem_command_count(); ++index) {
        if (strcmp(COMMANDS[index].framework_command_id,
            framework_command_id) == 0) return &COMMANDS[index];
    }
    return NULL;
}

size_t umi_studio_developer_ecosystem_view_count(void)
{
    return sizeof(VIEWS) / sizeof(VIEWS[0]);
}

const UmiStudioDeveloperEcosystemViewContribution *
umi_studio_developer_ecosystem_view_at(size_t index)
{
    return index < umi_studio_developer_ecosystem_view_count()
        ? &VIEWS[index] : NULL;
}

const UmiStudioDeveloperEcosystemViewContribution *
umi_studio_developer_ecosystem_view_find(const char *view_id)
{
    size_t index;
    if (view_id == NULL) return NULL;
    for (index = 0U; index < umi_studio_developer_ecosystem_view_count(); ++index) {
        if (strcmp(VIEWS[index].view_id, view_id) == 0) return &VIEWS[index];
    }
    return NULL;
}

UmiStatus umi_studio_developer_ecosystem_create(
    UmiEcosystemRuntime **out_runtime)
{
    return umi_ecosystem_runtime_create(out_runtime);
}
