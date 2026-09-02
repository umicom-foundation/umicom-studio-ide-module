/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/services.c
 *
 * PURPOSE:
 *   Construct and own the shared Framework service container used by Studio.
 *   Typed settings are created first so their validated values can configure
 *   later services, including recent-work persistence and the retained
 *   diagnostic-store capacity.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/services.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "umicom/studio/ai_platform.h"
#include "umicom/studio/compatibility_platform.h"
#include "umicom/studio/ai_tools.h"
#include "umicom/studio/knowledge_settings.h"
#include "umicom/studio/developer_platform.h"
#include "umicom/studio/declarative.h"
#include "umicom/studio/delivery_platform.h"
#include "umicom/studio/designer.h"
#include "umicom/studio/extension_platform.h"
#include "umicom/studio/product_centre.h"
#include "umicom/studio/fabric.h"
#include "umicom/studio/operations.h"
#include "umicom/studio/session.h"
#include "umicom/studio/trading.h"
#include "umicom/studio/version.h"
#include "umicom/studio/web_platform.h"

struct UmiStudioServices {
    UmiDiagnosticHub diagnostic_hub;
    UmiDiagnosticStore *diagnostic_store;
    UmiDiagnosticPipeline *diagnostic_pipeline;
    UmiSettings *settings;
    UmiTaskQueue *task_queue;
    UmiDocumentStore *documents;
    UmiSessionStore *session;
    UmiRecentItemRegistry *recent_items;
    UmiRecoveryManager *recovery;
    UmiWorkspaceGraph *workspace;
    UmiFileIndex *file_index;
    UmiWatcher *watcher;
    UmiProcessSupervisor *process_supervisor;
    UmiDataServer *data_server;
    UmiStore store;
    UmiSchemaRegistry *schemas;
    UmiDispatcher *dispatcher;
    UmiInbox *inbox;
    UmiOutbox *outbox;
    UmiDeadLetterStore *dead_letters;
    UmiTopicRegistry *topics;
    UmiMessageStore *message_store;
    UmiJournalStore journal;
    UmiMessageMetricsCounter *message_metrics;
    UmiStudioOperations *operations;
    UmiStudioAiPlatform *ai_platform;
    UmiStudioDeveloperPlatform *developer_platform;
    UmiStudioTradingService *trading;
    UmiTerminalController *terminal_controller;
    UmiStudioDeclarative *declarative;
    UmiStudioDesigner *designer;
    UmiStudioWebPlatform *web_platform;
    UmiStudioDeliveryPlatform *delivery_platform;
    UmiStudioCompatibilityPlatform *compatibility_platform;
    UmiStudioExtensionPlatform *extension_platform;
    UmiStudioProductCentre *product_centre;
    UmiClock clock;
    int recent_items_persistence_enabled;
    int recent_items_dirty;
    int published;
};

static const char *STUDIO_RECENT_ITEMS_PATH = ".umicom/studio-recent-items";

/* Record a workspace through the shared Framework model. The path is
 * normalised before hashing so equivalent spellings do not create duplicates. */
static UmiStatus studio_remember_workspace(UmiStudioServices *services,
                                           const char *root)
{
    UmiRecentItemSnapshot item = {0};
    UmiRecentItemSnapshot existing = {0};
    UmiStatus status;

    if (services == NULL || services->recent_items == NULL ||
        root == NULL || root[0] == '\0') {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    status = umi_path_normalise(root, item.uri, sizeof(item.uri));
    if (status != UMI_STATUS_OK) return status;
    status = umi_path_basename(item.uri, item.label, sizeof(item.label));
    if (status != UMI_STATUS_OK) return status;

    status = umi_platform_recent_item_id_from_uri(
        "studio-workspace", item.uri, item.id, sizeof(item.id));
    if (status != UMI_STATUS_OK) return status;
    (void)snprintf(item.kind, sizeof(item.kind), "%s", "workspace");
    item.struct_size = (uint32_t)sizeof(item);
    item.api_version = 1U;
    item.last_opened = services->clock.wall_nanoseconds != NULL
        ? services->clock.wall_nanoseconds(&services->clock)
        : 0U;
    item.open_count = 1U;

    if (umi_platform_recent_items_registry_find(
            services->recent_items, item.id, &existing) == UMI_STATUS_OK) {
        item.open_count = existing.open_count < UINT64_MAX
            ? existing.open_count + 1U
            : UINT64_MAX;
        item.pinned = existing.pinned;
    }
    status = umi_platform_recent_items_registry_upsert(
        services->recent_items, &item);
    if (status == UMI_STATUS_OK) {
        status = umi_platform_recent_items_registry_trim(
            services->recent_items, 50U);
    }
    return status;
}

/* Provide the studio watch sink operation used by this module and its client applications. */
static void studio_watch_sink(const UmiWatchEvent *event, void *user_data)
{
    UmiStudioServices *services = (UmiStudioServices *)user_data;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL || services->file_index == NULL || event == NULL) {
        return;
    }
    /* Apply this branch only when its contract condition is satisfied. */
    if (event->kind == UMI_WATCH_OVERFLOW ||
        event->kind == UMI_WATCH_RESCAN_REQUIRED || event->directory) {
        (void)umi_file_index_rebuild(services->file_index);
    } else /* Apply this branch only when its contract condition is satisfied. */ if (event->kind == UMI_WATCH_DELETED) {
        (void)umi_file_index_remove(services->file_index, event->path);
    } else /* Apply this branch only when its contract condition is satisfied. */ if (event->kind == UMI_WATCH_CREATED ||
               event->kind == UMI_WATCH_MODIFIED) {
        (void)umi_file_index_update(services->file_index, event->path);
    }
}

/* Provide the destroy partial operation used by this module and its client applications. */
static void destroy_partial(UmiStudioServices *services)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL) {
        return;
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services->watcher != NULL) {
        (void)umi_watcher_stop(services->watcher);
    }
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services->process_supervisor != NULL) {
        (void)umi_process_supervisor_shutdown(services->process_supervisor);
    }
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services->task_queue != NULL) {
        (void)umi_task_queue_shutdown(services->task_queue, 1);
    }
    umi_studio_product_centre_destroy(services->product_centre);
    services->product_centre = NULL;
    umi_studio_compatibility_platform_destroy(services->compatibility_platform);
    services->compatibility_platform = NULL;
    umi_studio_extension_platform_destroy(services->extension_platform);
    services->extension_platform = NULL;
    umi_studio_delivery_platform_destroy(services->delivery_platform);
    services->delivery_platform = NULL;
    umi_studio_web_platform_destroy(services->web_platform);
    services->web_platform = NULL;
    umi_studio_designer_destroy(services->designer);
    services->designer = NULL;
    umi_studio_declarative_destroy(services->declarative);
    services->declarative = NULL;
    umi_studio_ai_platform_destroy(services->ai_platform);
    services->ai_platform = NULL;
    umi_terminal_controller_destroy(services->terminal_controller);
    services->terminal_controller = NULL;
    umi_studio_trading_service_destroy(services->trading);
    services->trading = NULL;
    umi_studio_developer_platform_destroy(services->developer_platform);
    services->developer_platform = NULL;
    umi_studio_operations_destroy(services->operations);
    services->operations = NULL;
    umi_watcher_destroy(services->watcher);
    services->watcher = NULL;
    umi_file_index_destroy(services->file_index);
    services->file_index = NULL;
    umi_workspace_graph_destroy(services->workspace);
    services->workspace = NULL;
    free(services->message_metrics);
    services->message_metrics = NULL;
    umi_journal_store_dispose(&services->journal);
    umi_message_store_destroy(services->message_store);
    services->message_store = NULL;
    umi_topic_registry_destroy(services->topics);
    services->topics = NULL;
    umi_dead_letter_store_destroy(services->dead_letters);
    services->dead_letters = NULL;
    umi_outbox_destroy(services->outbox);
    services->outbox = NULL;
    umi_inbox_destroy(services->inbox);
    services->inbox = NULL;
    umi_dispatcher_destroy(services->dispatcher);
    services->dispatcher = NULL;
    umi_schema_registry_destroy(services->schemas);
    services->schemas = NULL;
    umi_data_server_destroy(services->data_server);
    services->data_server = NULL;
    umi_process_supervisor_destroy(services->process_supervisor);
    services->process_supervisor = NULL;
    umi_recovery_manager_destroy(services->recovery);
    services->recovery = NULL;
    /* Save recent work before releasing it. A registry loaded from malformed
     * data is deliberately not allowed to overwrite the file that needs
     * diagnosis or recovery. */
    if (services->recent_items != NULL) {
        if (services->recent_items_persistence_enabled != 0 &&
            services->recent_items_dirty != 0) {
            (void)umi_platform_recent_items_registry_save(
                services->recent_items, STUDIO_RECENT_ITEMS_PATH);
        }
        umi_platform_recent_items_registry_destroy(services->recent_items);
        services->recent_items = NULL;
    }
    umi_session_store_destroy(services->session);
    services->session = NULL;
    umi_document_store_destroy(services->documents);
    services->documents = NULL;
    umi_task_queue_destroy(services->task_queue);
    services->task_queue = NULL;
    umi_diagnostic_pipeline_destroy(services->diagnostic_pipeline);
    services->diagnostic_pipeline = NULL;
    umi_diagnostic_store_destroy(services->diagnostic_store);
    services->diagnostic_store = NULL;
    umi_settings_destroy(services->settings);
    services->settings = NULL;
    umi_clock_dispose(&services->clock);
    free(services);
}

/*
 * Initialise studio services from caller-provided values so later operations receive a
 * known state.
 */
UmiStatus umi_studio_services_create(
    UmiDiagnosticSink initial_sink,
    void *initial_user_data,
    UmiStudioServices **out_services)
{
    UmiDiagnosticStoreConfig store_config;
    UmiDiagnosticPipelineConfig pipeline_config;
    UmiTaskQueueConfig task_config;
    UmiStudioServices *services;
    UmiStatus status;
    int settings_loaded = 0;
    int session_loaded = 0;
    int recent_items_loaded = 0;
    int64_t diagnostic_capacity = 0;
    int64_t parallel_jobs = 0;
    int64_t ai_context_tokens = 0;
    int64_t ai_output_tokens = 0;
    int64_t ai_coding_context_tokens = 0;
    int64_t ai_coding_patch_files = 0;
    int64_t ai_coding_patch_lines = 0;
    int ai_allow_remote = 0;
    int ai_persist_sessions = 0;
    int ai_coding_allow_create = 0;
    int ai_coding_allow_delete = 0;
    int ai_coding_require_approval = 0;
    int ai_rag_enabled = 0;
    int ai_stream_responses = 0;
    char current_directory[UMI_PATH_CAPACITY];
    UmiFileIndexConfig file_index_config;
    UmiWatcherConfig watcher_config;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (out_services == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    *out_services = NULL;

    services = (UmiStudioServices *)calloc(1U, sizeof(*services));
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL) {
        return UMI_STATUS_OUT_OF_MEMORY;
    }

    status = umi_studio_settings_create(&services->settings);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        destroy_partial(services);
        return status;
    }

    status = umi_studio_settings_load_if_present(
        services->settings,
        umi_studio_settings_default_path(),
        &settings_loaded
    );
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        destroy_partial(services);
        return status;
    }
    (void)settings_loaded;

    status = umi_settings_get_integer(
        services->settings,
        UMI_STUDIO_SETTING_DIAGNOSTIC_CAPACITY,
        &diagnostic_capacity
    );
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (status != UMI_STATUS_OK || diagnostic_capacity <= 0) {
        destroy_partial(services);
        return status != UMI_STATUS_OK ? status : UMI_STATUS_INVALID_STATE;
    }

    status = umi_settings_get_integer(
        services->settings,
        UMI_STUDIO_SETTING_BUILD_PARALLEL_JOBS,
        &parallel_jobs
    );
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if (status != UMI_STATUS_OK || parallel_jobs <= 0) {
        destroy_partial(services);
        return status != UMI_STATUS_OK ? status : UMI_STATUS_INVALID_STATE;
    }

    umi_diagnostic_hub_init(&services->diagnostic_hub);
    services->clock = umi_clock_system();
    status = umi_studio_operations_create(&services->clock,
                                          &services->operations);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        destroy_partial(services);
        return status;
    }
    status = umi_studio_extension_platform_create(
        umi_studio_operations_plugins(services->operations),
        &services->extension_platform);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        destroy_partial(services);
        return status;
    }

    store_config = umi_diagnostic_store_config_default();
    store_config.capacity = (size_t)diagnostic_capacity;
    status = umi_diagnostic_store_create(&store_config,
                                         &services->diagnostic_store);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        destroy_partial(services);
        return status;
    }

    status = umi_diagnostic_hub_add(&services->diagnostic_hub,
                                    umi_diagnostic_store_sink,
                                    services->diagnostic_store);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        destroy_partial(services);
        return status;
    }

    pipeline_config = umi_diagnostic_pipeline_config_default();
    pipeline_config.diagnostic_capacity = (size_t)diagnostic_capacity;
    pipeline_config.output_capacity = (size_t)diagnostic_capacity;
    status = umi_diagnostic_pipeline_create(&pipeline_config,
                                            &services->diagnostic_pipeline);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_diagnostic_hub_add(&services->diagnostic_hub,
                                        umi_diagnostic_pipeline_sink,
                                        services->diagnostic_pipeline);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        destroy_partial(services);
        return status;
    }

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (initial_sink != NULL) {
        status = umi_diagnostic_hub_add(&services->diagnostic_hub,
                                        initial_sink,
                                        initial_user_data);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) {
            destroy_partial(services);
            return status;
        }
    }

    task_config = umi_task_queue_config_default();
    task_config.worker_count = (size_t)parallel_jobs;
    task_config.capacity = 512U;
    status = umi_task_queue_create(&task_config, &services->task_queue);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        destroy_partial(services);
        return status;
    }

    status = umi_document_store_create(&services->documents);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        destroy_partial(services);
        return status;
    }

    status = umi_session_store_create(&services->session);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        destroy_partial(services);
        return status;
    }
    status = umi_session_store_load(services->session,
                                    umi_studio_session_default_path(),
                                    &session_loaded);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        destroy_partial(services);
        return status;
    }
    (void)session_loaded;

    status = umi_platform_recent_items_registry_load(
        STUDIO_RECENT_ITEMS_PATH,
        &services->recent_items,
        &recent_items_loaded);
    if (status == UMI_STATUS_OK) {
        services->recent_items_persistence_enabled = 1;
    } else {
        /* Keep Studio usable when the optional history file is damaged. The
         * original file is preserved because this replacement model is marked
         * read-only for the lifetime of the service container. */
        services->recent_items_persistence_enabled = 0;
        status = umi_platform_recent_items_registry_create(
            &services->recent_items);
    }
    if (status != UMI_STATUS_OK) {
        destroy_partial(services);
        return status;
    }
    (void)recent_items_loaded;

    status = umi_recovery_manager_create(umi_studio_recovery_default_root(),
                                         &services->recovery);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        destroy_partial(services);
        return status;
    }

    status = umi_process_supervisor_create(NULL,
                                           &services->process_supervisor);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        destroy_partial(services);
        return status;
    }

    {
        const char *data_path = getenv("UMICOM_STUDIO_DATA_PATH");
        status = data_path != NULL && data_path[0] != '\0'
            ? umi_data_server_create_sqlite(data_path, &services->data_server)
            : umi_data_server_create_memory(&services->data_server);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_store_from_data_server(services->data_server,
                                            &services->store);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_schema_registry_create(&services->schemas);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_dispatcher_create(services->schemas,
                                       &services->dispatcher);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_inbox_create(4096U, &services->inbox);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_outbox_create(4096U, &services->outbox);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_dead_letter_store_create(2048U,
                                              &services->dead_letters);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_topic_registry_create(256U, &services->topics);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_message_store_create(&services->store,
                                          "studio-history",
                                          &services->message_store);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_journal_store_init(&services->journal,
                                        &services->store,
                                        "studio-journal");
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        services->message_metrics = (UmiMessageMetricsCounter *)calloc(
            1U, umi_message_metrics_counter_size());
        status = services->message_metrics != NULL
            ? UMI_STATUS_OK : UMI_STATUS_OUT_OF_MEMORY;
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        umi_message_metrics_init(services->message_metrics);
        status = umi_studio_fabric_register_defaults(services);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        destroy_partial(services);
        return status;
    }

    status = umi_workspace_graph_create(&services->workspace);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK ||
        umi_fs_current_directory(current_directory,
                                 sizeof(current_directory)) != UMI_STATUS_OK) {
        destroy_partial(services);
        return status != UMI_STATUS_OK ? status : UMI_STATUS_IO_ERROR;
    }
    status = umi_workspace_graph_open(services->workspace,
                                      current_directory,
                                      0);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        destroy_partial(services);
        return status;
    }

    file_index_config = umi_file_index_config_default(current_directory);
    status = umi_file_index_create(&file_index_config,
                                   &services->file_index);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        destroy_partial(services);
        return status;
    }

    watcher_config = umi_watcher_config_default(current_directory);
    watcher_config.sink = studio_watch_sink;
    watcher_config.sink_user_data = services;
    status = umi_watcher_create(&watcher_config, &services->watcher);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        destroy_partial(services);
        return status;
    }

    status = umi_studio_developer_platform_create(
        current_directory,
        &services->clock,
        &services->developer_platform
    );
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_build_service_bind_task_queue(
            umi_studio_developer_platform_build(
                services->developer_platform),
            services->task_queue);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_build_service_prepare_default_graph(
            umi_studio_developer_platform_build(
                services->developer_platform),
            1);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        UmiTerminalControllerConfig terminal_config =
            umi_terminal_controller_config_default();
        terminal_config.manager = umi_studio_terminal_service_manager(
            umi_studio_developer_platform_terminal(
                services->developer_platform));
        terminal_config.process_supervisor = services->process_supervisor;
        terminal_config.task_queue = services->task_queue;
        terminal_config.diagnostic_pipeline = services->diagnostic_pipeline;
        terminal_config.operation_graph = umi_studio_build_service_graph(
            umi_studio_developer_platform_build(
                services->developer_platform));
        terminal_config.clock = &services->clock;
        terminal_config.initial_session_id = "studio.primary";
        terminal_config.initial_title = "Umicom Studio";
        terminal_config.working_directory = current_directory;
        status = umi_terminal_controller_create(
            &terminal_config, &services->terminal_controller);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        destroy_partial(services);
        return status;
    }

    status = umi_studio_trading_service_create(&services->trading);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        destroy_partial(services);
        return status;
    }

    status = umi_studio_declarative_create(&services->declarative);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_designer_create(services->declarative,
                                            &services->designer);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_web_platform_create(&services->web_platform);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_delivery_platform_create(&services->delivery_platform);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_product_centre_create(&services->product_centre);
    }

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_compatibility_platform_create(
            &services->compatibility_platform
        );
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        UmiStudioAiPlatformConfig ai_config =
            umi_studio_ai_platform_config_default();
        status = umi_settings_get_text(
            services->settings, UMI_STUDIO_SETTING_AUTHORENGINE_EXECUTABLE,
            ai_config.authorengine_executable,
            sizeof(ai_config.authorengine_executable));
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) {
            status = umi_settings_get_text(
                services->settings, UMI_STUDIO_SETTING_AUTHORENGINE_WORKSPACE,
                ai_config.workspace, sizeof(ai_config.workspace));
        }
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK && strcmp(ai_config.workspace, ".") == 0) {
            (void)snprintf(ai_config.workspace, sizeof(ai_config.workspace),
                           "%s", current_directory);
        }
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) status = umi_settings_get_integer(
            services->settings, UMI_STUDIO_SETTING_AI_CONTEXT_TOKENS,
            &ai_context_tokens);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) status = umi_settings_get_integer(
            services->settings, UMI_STUDIO_SETTING_AI_RESERVED_OUTPUT_TOKENS,
            &ai_output_tokens);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) status = umi_settings_get_boolean(
            services->settings, UMI_STUDIO_SETTING_AI_ALLOW_REMOTE,
            &ai_allow_remote);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) status = umi_settings_get_boolean(
            services->settings, UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS,
            &ai_persist_sessions);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) status = umi_settings_get_integer(
            services->settings, UMI_STUDIO_SETTING_AI_CODING_CONTEXT_TOKENS,
            &ai_coding_context_tokens);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) status = umi_settings_get_integer(
            services->settings, UMI_STUDIO_SETTING_AI_CODING_MAX_PATCH_FILES,
            &ai_coding_patch_files);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) status = umi_settings_get_integer(
            services->settings, UMI_STUDIO_SETTING_AI_CODING_MAX_PATCH_LINES,
            &ai_coding_patch_lines);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) status = umi_settings_get_boolean(
            services->settings, UMI_STUDIO_SETTING_AI_CODING_ALLOW_CREATE,
            &ai_coding_allow_create);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) status = umi_settings_get_boolean(
            services->settings, UMI_STUDIO_SETTING_AI_CODING_ALLOW_DELETE,
            &ai_coding_allow_delete);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) status = umi_settings_get_boolean(
            services->settings, UMI_STUDIO_SETTING_AI_CODING_REQUIRE_APPROVAL,
            &ai_coding_require_approval);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) status = umi_settings_get_text(
            services->settings, UMI_STUDIO_SETTING_AI_PREFERRED_RUNTIME,
            ai_config.preferred_runtime_id,
            sizeof(ai_config.preferred_runtime_id));
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) status = umi_settings_get_text(
            services->settings, UMI_STUDIO_SETTING_AI_REMOTE_PROVIDER,
            ai_config.remote_provider_id,
            sizeof(ai_config.remote_provider_id));
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) status = umi_settings_get_text(
            services->settings, UMI_STUDIO_SETTING_AI_REMOTE_ENDPOINT,
            ai_config.remote_endpoint, sizeof(ai_config.remote_endpoint));
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) status = umi_settings_get_text(
            services->settings, UMI_STUDIO_SETTING_AI_REMOTE_MODEL,
            ai_config.remote_model_id, sizeof(ai_config.remote_model_id));
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) status = umi_settings_get_text(
            services->settings, UMI_STUDIO_SETTING_AI_REMOTE_SECRET_REFERENCE,
            ai_config.remote_secret_reference,
            sizeof(ai_config.remote_secret_reference));
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) status = umi_settings_get_boolean(
            services->settings, UMI_STUDIO_SETTING_AI_RAG_ENABLED,
            &ai_rag_enabled);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) status = umi_settings_get_boolean(
            services->settings, UMI_STUDIO_SETTING_AI_STREAM_RESPONSES,
            &ai_stream_responses);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) {
            status = umi_studio_knowledge_settings_apply(
                services->settings, &ai_config);
        }
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK &&
            (ai_context_tokens <= 0 || ai_context_tokens > UINT32_MAX ||
             ai_output_tokens <= 0 || ai_output_tokens > UINT32_MAX ||
             ai_output_tokens >= ai_context_tokens ||
             ai_coding_context_tokens <= 0 ||
             ai_coding_context_tokens > UINT32_MAX ||
             ai_coding_patch_files <= 0 ||
             ai_coding_patch_files > UMI_AI_CODING_PATCH_FILE_MAX ||
             ai_coding_patch_lines <= 0 ||
             ai_coding_patch_lines > UINT32_MAX)) {
            status = UMI_STATUS_INVALID_STATE;
        }
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status == UMI_STATUS_OK) {
            ai_config.context_tokens = (uint32_t)ai_context_tokens;
            ai_config.reserved_output_tokens = (uint32_t)ai_output_tokens;
            ai_config.allow_remote = ai_allow_remote;
            ai_config.persist_sessions = ai_persist_sessions;
            ai_config.coding_context_tokens =
                (uint32_t)ai_coding_context_tokens;
            ai_config.maximum_patch_files = (size_t)ai_coding_patch_files;
            ai_config.maximum_patch_lines = (uint32_t)ai_coding_patch_lines;
            ai_config.allow_patch_create = ai_coding_allow_create;
            ai_config.allow_patch_delete = ai_coding_allow_delete;
            ai_config.require_patch_approval = ai_coding_require_approval;
            ai_config.rag_enabled = ai_rag_enabled;
            ai_config.stream_responses = ai_stream_responses;
            status = umi_studio_ai_platform_create_configured(
                &ai_config, &services->ai_platform);
        }
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_ai_tools_register_defaults(services->ai_platform);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        destroy_partial(services);
        return status;
    }

    *out_services = services;
    return UMI_STATUS_OK;
}

/* Release or reset state held by studio services so the same storage can be reused safely. */
void umi_studio_services_destroy(UmiStudioServices *services)
{
    destroy_partial(services);
}

/* Provide the publish service operation used by this module and its client applications. */
static UmiStatus publish_service(UmiServiceRegistry *registry,
                                 const char *service_id,
                                 void *service,
                                 uint32_t flags)
{
    UmiServiceDescriptor descriptor;

    (void)memset(&descriptor, 0, sizeof(descriptor));
    descriptor.structure_size = (uint32_t)sizeof(descriptor);
    descriptor.service_id = service_id;
    descriptor.service_version = umi_studio_version();
    descriptor.provider_module_id = "org.umicom.studio.services";
    descriptor.service = service;
    descriptor.flags = flags;
    return umi_service_registry_register(registry, &descriptor);
}

/*
 * Provide the studio services publish operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_services_publish(
    UmiStudioServices *services,
    UmiMasterController *master)
{
    UmiServiceRegistry *registry;
    UmiHealthRegistry *health;
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL || master == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    /* Apply this branch only when its contract condition is satisfied. */
    if (services->published) {
        return UMI_STATUS_ALREADY_EXISTS;
    }

    registry = umi_master_controller_services(master);
    health = umi_master_controller_health(master);
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (registry == NULL || health == NULL) {
        return UMI_STATUS_INVALID_STATE;
    }

#define PUBLISH(id, service, flags)                                         \
    do {                                                                    \
        status = publish_service(registry, (id), (service), (flags));       \
        if (status != UMI_STATUS_OK) return status;                         \
    } while (0)

    PUBLISH("umicom.studio.settings",
            services->settings,
            UMI_SERVICE_SINGLETON);
    PUBLISH("umicom.studio.diagnostics.store",
            services->diagnostic_store,
            UMI_SERVICE_SINGLETON | UMI_SERVICE_THREAD_SAFE);
    PUBLISH("umicom.studio.diagnostics.pipeline",
            services->diagnostic_pipeline,
            UMI_SERVICE_SINGLETON | UMI_SERVICE_THREAD_SAFE);
    PUBLISH("umicom.studio.tasks",
            services->task_queue,
            UMI_SERVICE_SINGLETON | UMI_SERVICE_THREAD_SAFE);
    PUBLISH("umicom.studio.documents",
            services->documents,
            UMI_SERVICE_SINGLETON | UMI_SERVICE_THREAD_SAFE);
    PUBLISH("umicom.studio.session",
            services->session,
            UMI_SERVICE_SINGLETON | UMI_SERVICE_THREAD_SAFE);
    PUBLISH("umicom.studio.recent-items",
            services->recent_items,
            UMI_SERVICE_SINGLETON);
    PUBLISH("umicom.studio.recovery",
            services->recovery,
            UMI_SERVICE_SINGLETON | UMI_SERVICE_THREAD_SAFE);
    PUBLISH("umicom.studio.workspace",
            services->workspace,
            UMI_SERVICE_SINGLETON | UMI_SERVICE_THREAD_SAFE);
    PUBLISH("umicom.studio.file-index",
            services->file_index,
            UMI_SERVICE_SINGLETON | UMI_SERVICE_THREAD_SAFE);
    PUBLISH("umicom.studio.watcher",
            services->watcher,
            UMI_SERVICE_SINGLETON | UMI_SERVICE_THREAD_SAFE);
    PUBLISH("umicom.studio.process-supervisor",
            services->process_supervisor,
            UMI_SERVICE_SINGLETON | UMI_SERVICE_THREAD_SAFE);
    PUBLISH("umicom.studio.data-server",
            services->data_server,
            UMI_SERVICE_SINGLETON | UMI_SERVICE_THREAD_SAFE);
    PUBLISH("umicom.studio.message-schemas",
            services->schemas,
            UMI_SERVICE_SINGLETON | UMI_SERVICE_THREAD_SAFE);
    PUBLISH("umicom.studio.message-dispatcher",
            services->dispatcher,
            UMI_SERVICE_SINGLETON | UMI_SERVICE_THREAD_SAFE);
    PUBLISH("umicom.studio.message-inbox",
            services->inbox,
            UMI_SERVICE_SINGLETON | UMI_SERVICE_THREAD_SAFE);
    PUBLISH("umicom.studio.message-outbox",
            services->outbox,
            UMI_SERVICE_SINGLETON | UMI_SERVICE_THREAD_SAFE);
    PUBLISH("umicom.studio.dead-letters",
            services->dead_letters,
            UMI_SERVICE_SINGLETON | UMI_SERVICE_THREAD_SAFE);
    PUBLISH("umicom.studio.message-journal",
            &services->journal,
            UMI_SERVICE_SINGLETON | UMI_SERVICE_THREAD_SAFE);
    PUBLISH("umicom.studio.security",
            umi_studio_operations_security(services->operations),
            UMI_SERVICE_SINGLETON | UMI_SERVICE_THREAD_SAFE);
    PUBLISH("umicom.studio.plugins",
            umi_studio_operations_plugins(services->operations),
            UMI_SERVICE_SINGLETON | UMI_SERVICE_THREAD_SAFE);
    PUBLISH("umicom.studio.metrics",
            umi_studio_operations_metrics(services->operations),
            UMI_SERVICE_SINGLETON | UMI_SERVICE_THREAD_SAFE);
    PUBLISH("umicom.studio.traces",
            umi_studio_operations_traces(services->operations),
            UMI_SERVICE_SINGLETON | UMI_SERVICE_THREAD_SAFE);
    PUBLISH("umicom.studio.audit",
            umi_studio_operations_audit(services->operations),
            UMI_SERVICE_SINGLETON | UMI_SERVICE_THREAD_SAFE);
    PUBLISH("umicom.studio.readiness",
            umi_studio_operations_readiness(services->operations),
            UMI_SERVICE_SINGLETON | UMI_SERVICE_THREAD_SAFE);
    PUBLISH("umicom.studio.operational-events",
            umi_studio_operations_events(services->operations),
            UMI_SERVICE_SINGLETON | UMI_SERVICE_THREAD_SAFE);
    PUBLISH("umicom.studio.resilience",
            umi_studio_operations_resilience(services->operations),
            UMI_SERVICE_SINGLETON | UMI_SERVICE_THREAD_SAFE);
    PUBLISH("umicom.studio.developer-platform",
            services->developer_platform,
            UMI_SERVICE_SINGLETON);
    PUBLISH("umicom.studio.build",
            umi_studio_developer_platform_build(services->developer_platform),
            UMI_SERVICE_SINGLETON);
    PUBLISH("umicom.studio.tests",
            umi_studio_developer_platform_tests(services->developer_platform),
            UMI_SERVICE_SINGLETON);
    PUBLISH("umicom.studio.terminal",
            umi_studio_developer_platform_terminal(services->developer_platform),
            UMI_SERVICE_SINGLETON);
    PUBLISH("umicom.studio.terminal-controller",
            services->terminal_controller,
            UMI_SERVICE_SINGLETON);
    PUBLISH("umicom.studio.language",
            umi_studio_developer_platform_language(services->developer_platform),
            UMI_SERVICE_SINGLETON);
    PUBLISH("umicom.studio.debugger",
            umi_studio_developer_platform_debugger(services->developer_platform),
            UMI_SERVICE_SINGLETON);
    PUBLISH("umicom.studio.source-control",
            umi_studio_developer_platform_source_control(
                services->developer_platform),
            UMI_SERVICE_SINGLETON);
    PUBLISH("umicom.studio.vcs-workspace",
            umi_studio_source_control_service_workspace(
                umi_studio_developer_platform_source_control(
                    services->developer_platform)),
            UMI_SERVICE_SINGLETON);
    PUBLISH("umicom.studio.trading",
            services->trading,
            UMI_SERVICE_SINGLETON);
    PUBLISH("umicom.studio.trading-workspace",
            umi_studio_trading_service_workspace(services->trading),
            UMI_SERVICE_SINGLETON);
    PUBLISH("umicom.studio.declarative",
            services->declarative,
            UMI_SERVICE_SINGLETON);
    PUBLISH("umicom.studio.designer",
            services->designer,
            UMI_SERVICE_SINGLETON);
    PUBLISH("umicom.studio.web-platform",
            services->web_platform,
            UMI_SERVICE_SINGLETON);
    PUBLISH("umicom.studio.delivery-platform",
            services->delivery_platform,
            UMI_SERVICE_SINGLETON);
    PUBLISH("umicom.studio.product-centre",
            services->product_centre,
            UMI_SERVICE_SINGLETON);
    PUBLISH("umicom.studio.distribution-service",
            umi_studio_product_centre_service(services->product_centre),
            UMI_SERVICE_SINGLETON);
    PUBLISH("umicom.studio.ai-platform",
            services->ai_platform,
            UMI_SERVICE_SINGLETON);
    PUBLISH("umicom.studio.ai-runtime",
            umi_studio_ai_platform_runtime(services->ai_platform),
            UMI_SERVICE_SINGLETON);
    PUBLISH("umicom.studio.authorengine-integration",
            umi_studio_ai_platform_authorengine(services->ai_platform),
            UMI_SERVICE_SINGLETON);
    PUBLISH("umicom.studio.ai-coding-assistant",
            umi_studio_ai_platform_coding_assistant(services->ai_platform),
            UMI_SERVICE_SINGLETON);
    PUBLISH("umicom.studio.knowledge-centre",
            umi_studio_ai_platform_knowledge(services->ai_platform),
            UMI_SERVICE_SINGLETON);
    PUBLISH("umicom.studio.helix",
            umi_studio_ai_platform_helix(services->ai_platform),
            UMI_SERVICE_SINGLETON);
    status = umi_studio_designer_bind_commands(
        services->designer,
        umi_master_controller_command_registry(master));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    PUBLISH("umicom.studio.clock",
            &services->clock,
            UMI_SERVICE_SINGLETON | UMI_SERVICE_THREAD_SAFE);
#undef PUBLISH

    status = umi_health_registry_update(health,
                                        "org.umicom.studio.services",
                                        UMI_HEALTH_READY,
                                        "Studio shared services are ready",
                                        services->clock.wall_nanoseconds(
                                            &services->clock));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) {
        return status;
    }

    services->published = 1;
    return UMI_STATUS_OK;
}

/*
 * Provide the studio services add diagnostic sink operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_services_add_diagnostic_sink(
    UmiStudioServices *services,
    UmiDiagnosticSink sink,
    void *user_data)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    return umi_diagnostic_hub_add(&services->diagnostic_hub,
                                  sink,
                                  user_data);
}

/*
 * Provide the studio services remove diagnostic sink operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_services_remove_diagnostic_sink(
    UmiStudioServices *services,
    UmiDiagnosticSink sink,
    void *user_data)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    return umi_diagnostic_hub_remove(&services->diagnostic_hub,
                                     sink,
                                     user_data);
}

/*
 * Provide the studio services diagnostic sink operation used by this module and its client
 * applications.
 */
UmiDiagnosticSink umi_studio_services_diagnostic_sink(void)
{
    return umi_diagnostic_hub_emit;
}

/*
 * Provide the studio services diagnostic user data operation used by this module and its
 * client applications.
 */
void *umi_studio_services_diagnostic_user_data(UmiStudioServices *services)
{
    return services != NULL ? (void *)&services->diagnostic_hub : NULL;
}

/*
 * Provide the studio services clock operation used by this module and its client
 * applications.
 */
UmiClock *umi_studio_services_clock(UmiStudioServices *services)
{
    return services != NULL ? &services->clock : NULL;
}

/*
 * Provide the studio services settings operation used by this module and its client
 * applications.
 */
UmiSettings *umi_studio_services_settings(UmiStudioServices *services)
{
    return services != NULL ? services->settings : NULL;
}

/*
 * Provide the studio services diagnostic store operation used by this module and its
 * client applications.
 */
UmiDiagnosticStore *umi_studio_services_diagnostic_store(
    UmiStudioServices *services)
{
    return services != NULL ? services->diagnostic_store : NULL;
}

/*
 * Provide the studio services diagnostic pipeline operation used by this module and its
 * client applications.
 */
UmiDiagnosticPipeline *umi_studio_services_diagnostic_pipeline(
    UmiStudioServices *services)
{
    return services != NULL ? services->diagnostic_pipeline : NULL;
}

/*
 * Provide the studio services task queue operation used by this module and its client
 * applications.
 */
UmiTaskQueue *umi_studio_services_task_queue(UmiStudioServices *services)
{
    return services != NULL ? services->task_queue : NULL;
}

/*
 * Provide the studio services documents operation used by this module and its client
 * applications.
 */
UmiDocumentStore *umi_studio_services_documents(UmiStudioServices *services)
{
    return services != NULL ? services->documents : NULL;
}

/*
 * Provide the studio services session operation used by this module and its client
 * applications.
 */
UmiSessionStore *umi_studio_services_session(UmiStudioServices *services)
{
    return services != NULL ? services->session : NULL;
}

/* Expose a borrowed registry to Studio surfaces without transferring ownership. */
UmiRecentItemRegistry *umi_studio_services_recent_items(
    UmiStudioServices *services)
{
    return services != NULL ? services->recent_items : NULL;
}

/*
 * Provide the studio services recovery operation used by this module and its client
 * applications.
 */
UmiRecoveryManager *umi_studio_services_recovery(UmiStudioServices *services)
{
    return services != NULL ? services->recovery : NULL;
}

/*
 * Provide the studio services workspace operation used by this module and its client
 * applications.
 */
UmiWorkspaceGraph *umi_studio_services_workspace(UmiStudioServices *services)
{
    return services != NULL ? services->workspace : NULL;
}

/*
 * Provide the studio services file index operation used by this module and its client
 * applications.
 */
UmiFileIndex *umi_studio_services_file_index(UmiStudioServices *services)
{
    return services != NULL ? services->file_index : NULL;
}

/*
 * Provide the studio services watcher operation used by this module and its client
 * applications.
 */
UmiWatcher *umi_studio_services_watcher(UmiStudioServices *services)
{
    return services != NULL ? services->watcher : NULL;
}

/*
 * Provide the studio services process supervisor operation used by this module and its
 * client applications.
 */
UmiProcessSupervisor *umi_studio_services_process_supervisor(
    UmiStudioServices *services)
{
    return services != NULL ? services->process_supervisor : NULL;
}

/*
 * Provide the studio services data server operation used by this module and its client
 * applications.
 */
UmiDataServer *umi_studio_services_data_server(UmiStudioServices *services)
{
    return services != NULL ? services->data_server : NULL;
}

/*
 * Provide the studio services store operation used by this module and its client
 * applications.
 */
UmiStore *umi_studio_services_store(UmiStudioServices *services)
{
    return services != NULL ? &services->store : NULL;
}

/*
 * Provide the studio services schema registry operation used by this module and its client
 * applications.
 */
UmiSchemaRegistry *umi_studio_services_schema_registry(UmiStudioServices *services)
{
    return services != NULL ? services->schemas : NULL;
}

/*
 * Provide the studio services dispatcher operation used by this module and its client
 * applications.
 */
UmiDispatcher *umi_studio_services_dispatcher(UmiStudioServices *services)
{
    return services != NULL ? services->dispatcher : NULL;
}

/*
 * Provide the studio services inbox operation used by this module and its client
 * applications.
 */
UmiInbox *umi_studio_services_inbox(UmiStudioServices *services)
{
    return services != NULL ? services->inbox : NULL;
}

/*
 * Provide the studio services outbox operation used by this module and its client
 * applications.
 */
UmiOutbox *umi_studio_services_outbox(UmiStudioServices *services)
{
    return services != NULL ? services->outbox : NULL;
}

/*
 * Provide the studio services dead letters operation used by this module and its client
 * applications.
 */
UmiDeadLetterStore *umi_studio_services_dead_letters(UmiStudioServices *services)
{
    return services != NULL ? services->dead_letters : NULL;
}

/*
 * Provide the studio services topics operation used by this module and its client
 * applications.
 */
UmiTopicRegistry *umi_studio_services_topics(UmiStudioServices *services)
{
    return services != NULL ? services->topics : NULL;
}

/*
 * Provide the studio services message store operation used by this module and its client
 * applications.
 */
UmiMessageStore *umi_studio_services_message_store(UmiStudioServices *services)
{
    return services != NULL ? services->message_store : NULL;
}

/*
 * Provide the studio services journal operation used by this module and its client
 * applications.
 */
UmiJournalStore *umi_studio_services_journal(UmiStudioServices *services)
{
    return services != NULL ? &services->journal : NULL;
}

/*
 * Provide the studio services message metrics operation used by this module and its client
 * applications.
 */
UmiMessageMetricsCounter *umi_studio_services_message_metrics(UmiStudioServices *services)
{
    return services != NULL ? services->message_metrics : NULL;
}

/*
 * Provide the studio services operations operation used by this module and its client
 * applications.
 */
UmiStudioOperations *umi_studio_services_operations(UmiStudioServices *services)
{
    return services != NULL ? services->operations : NULL;
}

/*
 * Provide the studio services ai platform operation used by this module and its client
 * applications.
 */
UmiStudioAiPlatform *umi_studio_services_ai_platform(
    UmiStudioServices *services)
{
    return services != NULL ? services->ai_platform : NULL;
}

/*
 * Provide the studio services knowledge operation used by this module and its client
 * applications.
 */
UmiKnowledgeService *umi_studio_services_knowledge(
    UmiStudioServices *services)
{
    return services != NULL
        ? umi_studio_ai_platform_knowledge(services->ai_platform) : NULL;
}

/*
 * Provide the studio services developer platform operation used by this module and its
 * client applications.
 */
UmiStudioDeveloperPlatform *umi_studio_services_developer_platform(
    UmiStudioServices *services)
{
    return services != NULL ? services->developer_platform : NULL;
}

/*
 * Provide the studio services build operation used by this module and its client
 * applications.
 */
UmiStudioBuildService *umi_studio_services_build(UmiStudioServices *services)
{
    return services != NULL && services->developer_platform != NULL
        ? umi_studio_developer_platform_build(services->developer_platform)
        : NULL;
}

/*
 * Provide the studio services tests operation used by this module and its client
 * applications.
 */
UmiStudioTestService *umi_studio_services_tests(UmiStudioServices *services)
{
    return services != NULL && services->developer_platform != NULL
        ? umi_studio_developer_platform_tests(services->developer_platform)
        : NULL;
}

/*
 * Provide the studio services terminal operation used by this module and its client
 * applications.
 */
UmiStudioTerminalService *umi_studio_services_terminal(
    UmiStudioServices *services)
{
    return services != NULL && services->developer_platform != NULL
        ? umi_studio_developer_platform_terminal(services->developer_platform)
        : NULL;
}

/*
 * Provide the studio services terminal controller operation used by this module and its
 * client applications.
 */
UmiTerminalController *umi_studio_services_terminal_controller(
    UmiStudioServices *services)
{
    return services != NULL ? services->terminal_controller : NULL;
}

/*
 * Provide the studio services language operation used by this module and its client
 * applications.
 */
UmiStudioLanguageService *umi_studio_services_language(
    UmiStudioServices *services)
{
    return services != NULL && services->developer_platform != NULL
        ? umi_studio_developer_platform_language(services->developer_platform)
        : NULL;
}

/*
 * Provide the studio services debugger operation used by this module and its client
 * applications.
 */
UmiStudioDebuggerService *umi_studio_services_debugger(
    UmiStudioServices *services)
{
    return services != NULL && services->developer_platform != NULL
        ? umi_studio_developer_platform_debugger(services->developer_platform)
        : NULL;
}

/*
 * Provide the studio services source control operation used by this module and its client
 * applications.
 */
UmiStudioSourceControlService *umi_studio_services_source_control(
    UmiStudioServices *services)
{
    return services != NULL && services->developer_platform != NULL
        ? umi_studio_developer_platform_source_control(
              services->developer_platform)
        : NULL;
}

/*
 * Provide the studio services trading operation used by this module and its client
 * applications.
 */
UmiStudioTradingService *umi_studio_services_trading(
    UmiStudioServices *services)
{
    return services != NULL ? services->trading : NULL;
}

/*
 * Provide the studio services open workspace operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_services_open_workspace(UmiStudioServices *services,
                                             const char *root,
                                             int trusted)
{
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL || root == NULL || root[0] == '\0') {
        return UMI_STATUS_INVALID_ARGUMENT;
    }
    status = umi_watcher_stop(services->watcher);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_file_index_set_root(services->file_index, root);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_watcher_set_root(services->watcher, root);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_workspace_graph_open(services->workspace, root, trusted);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_workspace_graph_discover(services->workspace);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_file_index_rebuild(services->file_index);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        UmiWorkspaceGraphSnapshot workspace_snapshot;
        UmiWorkspaceProjectSnapshot project_snapshot;
        const char *project_id = "workspace";
        /* Apply this branch only when its contract condition is satisfied. */
        if (umi_workspace_graph_snapshot(services->workspace,
                                         &workspace_snapshot) ==
                UMI_STATUS_OK) {
            /* Apply this branch only when its contract condition is satisfied. */
            if (workspace_snapshot.project_count > 0U &&
                umi_workspace_graph_project_at(services->workspace, 0U,
                                                &project_snapshot) ==
                    UMI_STATUS_OK) {
                project_id = project_snapshot.stable_id;
            }
            status = umi_studio_test_service_set_workspace(
                umi_studio_services_tests(services), root, project_id,
                workspace_snapshot.revision);
        }
    }
    if (status == UMI_STATUS_OK) {
        UmiStatus recent_status;
        /* Recent-work persistence is helpful but not required to open a
         * workspace. A history-file error must not turn a successful open into
         * a failed developer operation. */
        recent_status = studio_remember_workspace(services, root);
        if (recent_status == UMI_STATUS_OK) {
            services->recent_items_dirty = 1;
            if (services->recent_items_persistence_enabled != 0 &&
                umi_platform_recent_items_registry_save(
                    services->recent_items,
                    STUDIO_RECENT_ITEMS_PATH) == UMI_STATUS_OK) {
                services->recent_items_dirty = 0;
            }
        }
    }
    return status;
}

/*
 * Provide the studio services close workspace operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_services_close_workspace(UmiStudioServices *services)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (services == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    (void)umi_watcher_stop(services->watcher);
    status = umi_workspace_graph_close(services->workspace);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_file_index_clear(services->file_index);
    }
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) {
        status = umi_studio_test_service_set_workspace(
            umi_studio_services_tests(services), "", "", 0U);
    }
    return status;
}

/*
 * Return the number of records represented by studio services diagnostic sink without
 * changing their state.
 */
size_t umi_studio_services_diagnostic_sink_count(
    const UmiStudioServices *services)
{
    return services != NULL
        ? umi_diagnostic_hub_count(&services->diagnostic_hub)
        : 0U;
}

/*
 * Provide the studio services declarative operation used by this module and its client
 * applications.
 */
UmiStudioDeclarative *umi_studio_services_declarative(UmiStudioServices *services)
{
    return services != NULL ? services->declarative : NULL;
}

/*
 * Provide the studio services designer operation used by this module and its client
 * applications.
 */
UmiStudioDesigner *umi_studio_services_designer(UmiStudioServices *services)
{
    return services != NULL ? services->designer : NULL;
}

/*
 * Provide the studio services web platform operation used by this module and its client
 * applications.
 */
UmiStudioWebPlatform *umi_studio_services_web_platform(
    UmiStudioServices *services)
{
    return services != NULL ? services->web_platform : NULL;
}

/*
 * Provide the studio services delivery platform operation used by this module and its
 * client applications.
 */
UmiStudioDeliveryPlatform *umi_studio_services_delivery_platform(
    UmiStudioServices *services)
{
    return services != NULL ? services->delivery_platform : NULL;
}

/*
 * Provide the studio services compatibility platform operation used by this module and its
 * client applications.
 */
UmiStudioCompatibilityPlatform *umi_studio_services_compatibility_platform(
    UmiStudioServices *services)
{
    return services == NULL ? NULL : services->compatibility_platform;
}

/*
 * Provide the studio services extension platform operation used by this module and its
 * client applications.
 */
UmiStudioExtensionPlatform *umi_studio_services_extension_platform(
    UmiStudioServices *services)
{
    return services != NULL ? services->extension_platform : NULL;
}

/*
 * Provide the studio services product centre operation used by this module and its client
 * applications.
 */
UmiStudioProductCentre *umi_studio_services_product_centre(
    UmiStudioServices *services)
{
    return services != NULL ? services->product_centre : NULL;
}
