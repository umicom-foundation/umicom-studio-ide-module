/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/data_workbench_contribution.c
 *
 * PURPOSE:
 *   Define Studio-only command and view placement for the Framework Database
 *   workbench. All operational behaviour remains Framework-owned.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/data_workbench_contribution.h"

#include <string.h>

#define COMMAND(identifier, menu, group, position)                            \
    {                                                                          \
        (uint32_t)sizeof(UmiStudioDataWorkbenchCommandContribution),           \
        UMI_STUDIO_DATA_WORKBENCH_CONTRIBUTION_API_VERSION,                    \
        identifier, menu, group, position, 1                                   \
    }

static const UmiStudioDataWorkbenchCommandContribution COMMANDS[] = {
    COMMAND("data.workbench.connection.new", "Database", "connections", 100),
    COMMAND("data.workbench.connection.edit", "Database", "connections", 110),
    COMMAND("data.workbench.connection.activate", "Database", "connections", 120),
    COMMAND("data.workbench.connection.refresh", "Database", "connections", 130),
    COMMAND("data.workbench.navigator.refresh", "Database", "navigator", 200),
    COMMAND("data.workbench.navigator.filter", "Database", "navigator", 210),
    COMMAND("data.workbench.sql.new", "File", "sql", 300),
    COMMAND("data.workbench.sql.execute", "Run", "sql", 310),
    COMMAND("data.workbench.sql.explain", "Run", "sql", 320),
    COMMAND("data.workbench.sql.cancel", "Run", "sql", 330),
    COMMAND("data.workbench.transaction.begin", "Database", "transaction", 400),
    COMMAND("data.workbench.transaction.commit", "Database", "transaction", 410),
    COMMAND("data.workbench.transaction.rollback", "Database", "transaction", 420),
    COMMAND("data.workbench.results.next-page", "Navigate", "results", 500),
    COMMAND("data.workbench.results.previous-page", "Navigate", "results", 510),
    COMMAND("data.workbench.results.copy", "Edit", "results", 520),
    COMMAND("data.workbench.results.export", "File", "export", 530),
    COMMAND("data.workbench.builder.open", "Database", "query-builder", 600),
    COMMAND("data.workbench.builder.preview", "Database", "query-builder", 610),
    COMMAND("data.workbench.schema.compare", "Database", "schema", 700),
    COMMAND("data.workbench.schema.generate-migration", "Database", "schema", 710),
    COMMAND("data.workbench.er.open", "Database", "modelling", 800),
    COMMAND("data.workbench.er.auto-layout", "Database", "modelling", 810),
    COMMAND("data.workbench.data.edit", "Database", "data-editor", 900),
    COMMAND("data.workbench.data.validate", "Database", "data-editor", 910),
    COMMAND("data.workbench.data.apply", "Database", "data-editor", 920),
    COMMAND("data.workbench.data.discard", "Database", "data-editor", 930),
    COMMAND("data.workbench.transfer.import", "File", "import", 1000),
    COMMAND("data.workbench.transfer.export", "File", "export", 1010),
    COMMAND("data.workbench.transfer.dry-run", "Database", "transfer", 1020),
    COMMAND("data.workbench.admin.open", "Tools", "data-server", 1100),
    COMMAND("data.workbench.admin.refresh", "Tools", "data-server", 1110),
    COMMAND("data.workbench.admin.maintenance", "Tools", "data-server", 1120),
    COMMAND("data.workbench.history.clear", "Database", "history", 1200)
};

#undef COMMAND

#define VIEW(identifier, label, contract, region, position)                   \
    {                                                                          \
        (uint32_t)sizeof(UmiStudioDataWorkbenchViewContribution),              \
        UMI_STUDIO_DATA_WORKBENCH_CONTRIBUTION_API_VERSION,                    \
        identifier, label, contract, region, position, 1, 1                    \
    }

static const UmiStudioDataWorkbenchViewContribution VIEWS[] = {
    VIEW("studio.data-workbench.connections", "Connections",
         "umicom.data.workbench.connection-model", "left-sidebar", 300),
    VIEW("studio.data-workbench.navigator", "Database Navigator",
         "umicom.data.workbench.navigator-model", "left-sidebar", 310),
    VIEW("studio.data-workbench.sql", "SQL Editor",
         "umicom.data.workbench.sql-document", "editor-area", 320),
    VIEW("studio.data-workbench.results", "Query Results",
         "umicom.data.workbench.result-model", "bottom-panel", 330),
    VIEW("studio.data-workbench.query-builder", "Visual Query Builder",
         "umicom.data.workbench.query-builder", "editor-area", 340),
    VIEW("studio.data-workbench.schema-compare", "Schema Compare",
         "umicom.data.workbench.schema-compare", "editor-area", 350),
    VIEW("studio.data-workbench.er-diagram", "ER Diagram",
         "umicom.data.workbench.er-model", "editor-area", 360),
    VIEW("studio.data-workbench.data-editor", "Data Editor",
         "umicom.data.workbench.data-editor", "editor-area", 370),
    VIEW("studio.data-workbench.transfers", "Import and Export",
         "umicom.data.workbench.transfer", "bottom-panel", 380),
    VIEW("studio.data-workbench.administration", "Data Server Administration",
         "umicom.data.workbench.admin-model", "secondary-sidebar", 390),
    VIEW("studio.data-workbench.history", "Query History",
         "umicom.data.query-history", "bottom-panel", 400)
};

#undef VIEW

size_t umi_studio_data_workbench_command_count(void)
{
    return sizeof(COMMANDS) / sizeof(COMMANDS[0]);
}

const UmiStudioDataWorkbenchCommandContribution *
umi_studio_data_workbench_command_at(size_t index)
{
    return index < umi_studio_data_workbench_command_count()
        ? &COMMANDS[index] : NULL;
}

const UmiStudioDataWorkbenchCommandContribution *
umi_studio_data_workbench_command_find(const char *framework_command_id)
{
    size_t index;
    if (framework_command_id == NULL) return NULL;
    for (index = 0U; index < umi_studio_data_workbench_command_count(); ++index) {
        if (strcmp(COMMANDS[index].framework_command_id,
                   framework_command_id) == 0) {
            return &COMMANDS[index];
        }
    }
    return NULL;
}

size_t umi_studio_data_workbench_view_count(void)
{
    return sizeof(VIEWS) / sizeof(VIEWS[0]);
}

const UmiStudioDataWorkbenchViewContribution *
umi_studio_data_workbench_view_at(size_t index)
{
    return index < umi_studio_data_workbench_view_count()
        ? &VIEWS[index] : NULL;
}

const UmiStudioDataWorkbenchViewContribution *
umi_studio_data_workbench_view_find(const char *view_id)
{
    size_t index;
    if (view_id == NULL) return NULL;
    for (index = 0U; index < umi_studio_data_workbench_view_count(); ++index) {
        if (strcmp(VIEWS[index].view_id, view_id) == 0) return &VIEWS[index];
    }
    return NULL;
}

UmiStatus umi_studio_data_workbench_create(
    UmiDatabaseExplorer *explorer,
    UmiDataWorkbenchRuntime **out_runtime)
{
    return umi_data_workbench_runtime_create(explorer, out_runtime);
}
