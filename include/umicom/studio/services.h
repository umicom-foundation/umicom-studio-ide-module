/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/services.h
 *
 * PURPOSE:
 *   Own the Umicom Framework services shared by Studio frontends and Slave
 *   Controllers.  The container creates one instance of each authoritative
 *   settings, diagnostics, task, document, session, recovery, workspace, file
 *   index, watcher, process-supervision, Data Server, Integration Fabric and
 *   clock service.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_SERVICES_H
#define UMICOM_STUDIO_SERVICES_H

#include <stddef.h>

#include "umicom/umicom.h"
#include "umicom/studio/ai_platform.h"
#include "umicom/studio/compatibility_platform.h"
#include "umicom/studio/developer_platform.h"
#include "umicom/studio/declarative.h"
#include "umicom/studio/delivery_platform.h"
#include "umicom/studio/designer.h"
#include "umicom/studio/extension_platform.h"
#include "umicom/studio/product_centre.h"
#include "umicom/studio/settings.h"
#include "umicom/studio/trading.h"
#include "umicom/studio/web_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represent the studio services data shared with callers of this public contract.
 */
typedef struct UmiStudioServices UmiStudioServices;
/**
 * Represent the studio operations data shared with callers of this public contract.
 */
typedef struct UmiStudioOperations UmiStudioOperations;

/**
 * Initialise studio services from caller-provided values so later operations receive a
 * known state.
 */
UmiStatus umi_studio_services_create(
    UmiDiagnosticSink initial_sink,
    void *initial_user_data,
    UmiStudioServices **out_services
);

/**
 * Release or reset state held by studio services so the same storage can be reused safely.
 */
void umi_studio_services_destroy(UmiStudioServices *services);

/**
 * Provide the studio services publish operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_services_publish(
    UmiStudioServices *services,
    UmiMasterController *master
);

/**
 * Provide the studio services add diagnostic sink operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_services_add_diagnostic_sink(
    UmiStudioServices *services,
    UmiDiagnosticSink sink,
    void *user_data
);

/**
 * Provide the studio services remove diagnostic sink operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_services_remove_diagnostic_sink(
    UmiStudioServices *services,
    UmiDiagnosticSink sink,
    void *user_data
);

/**
 * Provide the studio services diagnostic sink operation used by this module and its client
 * applications.
 */
UmiDiagnosticSink umi_studio_services_diagnostic_sink(void);
/**
 * Provide the studio services diagnostic user data operation used by this module and its
 * client applications.
 */
void *umi_studio_services_diagnostic_user_data(UmiStudioServices *services);
/**
 * Provide the studio services clock operation used by this module and its client
 * applications.
 */
UmiClock *umi_studio_services_clock(UmiStudioServices *services);
/**
 * Provide the studio services settings operation used by this module and its client
 * applications.
 */
UmiSettings *umi_studio_services_settings(UmiStudioServices *services);
/**
 * Provide the studio services diagnostic store operation used by this module and its
 * client applications.
 */
UmiDiagnosticStore *umi_studio_services_diagnostic_store(
    UmiStudioServices *services
);
/**
 * Provide the studio services diagnostic pipeline operation used by this module and its
 * client applications.
 */
UmiDiagnosticPipeline *umi_studio_services_diagnostic_pipeline(
    UmiStudioServices *services
);
/**
 * Provide the studio services task queue operation used by this module and its client
 * applications.
 */
UmiTaskQueue *umi_studio_services_task_queue(UmiStudioServices *services);
/**
 * Provide the studio services documents operation used by this module and its client
 * applications.
 */
UmiDocumentStore *umi_studio_services_documents(UmiStudioServices *services);
/**
 * Provide the studio services session operation used by this module and its client
 * applications.
 */
UmiSessionStore *umi_studio_services_session(UmiStudioServices *services);
/**
 * Provide the studio services recovery operation used by this module and its client
 * applications.
 */
UmiRecoveryManager *umi_studio_services_recovery(UmiStudioServices *services);
/**
 * Provide the studio services workspace operation used by this module and its client
 * applications.
 */
UmiWorkspaceGraph *umi_studio_services_workspace(UmiStudioServices *services);
/**
 * Provide the studio services file index operation used by this module and its client
 * applications.
 */
UmiFileIndex *umi_studio_services_file_index(UmiStudioServices *services);
/**
 * Provide the studio services watcher operation used by this module and its client
 * applications.
 */
UmiWatcher *umi_studio_services_watcher(UmiStudioServices *services);
/**
 * Provide the studio services process supervisor operation used by this module and its
 * client applications.
 */
UmiProcessSupervisor *umi_studio_services_process_supervisor(
    UmiStudioServices *services
);
/**
 * Provide the studio services data server operation used by this module and its client
 * applications.
 */
UmiDataServer *umi_studio_services_data_server(UmiStudioServices *services);
/**
 * Provide the studio services store operation used by this module and its client
 * applications.
 */
UmiStore *umi_studio_services_store(UmiStudioServices *services);
/**
 * Provide the studio services schema registry operation used by this module and its client
 * applications.
 */
UmiSchemaRegistry *umi_studio_services_schema_registry(UmiStudioServices *services);
/**
 * Provide the studio services dispatcher operation used by this module and its client
 * applications.
 */
UmiDispatcher *umi_studio_services_dispatcher(UmiStudioServices *services);
/**
 * Provide the studio services inbox operation used by this module and its client
 * applications.
 */
UmiInbox *umi_studio_services_inbox(UmiStudioServices *services);
/**
 * Provide the studio services outbox operation used by this module and its client
 * applications.
 */
UmiOutbox *umi_studio_services_outbox(UmiStudioServices *services);
/**
 * Provide the studio services dead letters operation used by this module and its client
 * applications.
 */
UmiDeadLetterStore *umi_studio_services_dead_letters(UmiStudioServices *services);
/**
 * Provide the studio services topics operation used by this module and its client
 * applications.
 */
UmiTopicRegistry *umi_studio_services_topics(UmiStudioServices *services);
/**
 * Provide the studio services message store operation used by this module and its client
 * applications.
 */
UmiMessageStore *umi_studio_services_message_store(UmiStudioServices *services);
/**
 * Provide the studio services journal operation used by this module and its client
 * applications.
 */
UmiJournalStore *umi_studio_services_journal(UmiStudioServices *services);
/**
 * Provide the studio services message metrics operation used by this module and its client
 * applications.
 */
UmiMessageMetricsCounter *umi_studio_services_message_metrics(UmiStudioServices *services);
/**
 * Provide the studio services operations operation used by this module and its client
 * applications.
 */
UmiStudioOperations *umi_studio_services_operations(UmiStudioServices *services);
/**
 * Provide the studio services ai platform operation used by this module and its client
 * applications.
 */
UmiStudioAiPlatform *umi_studio_services_ai_platform(
    UmiStudioServices *services
);
/**
 * Provide the studio services knowledge operation used by this module and its client
 * applications.
 */
UmiKnowledgeService *umi_studio_services_knowledge(
    UmiStudioServices *services
);
/**
 * Provide the studio services developer platform operation used by this module and its
 * client applications.
 */
UmiStudioDeveloperPlatform *umi_studio_services_developer_platform(
    UmiStudioServices *services
);
/**
 * Provide the studio services declarative operation used by this module and its client
 * applications.
 */
UmiStudioDeclarative *umi_studio_services_declarative(UmiStudioServices *services);
/**
 * Provide the studio services designer operation used by this module and its client
 * applications.
 */
UmiStudioDesigner *umi_studio_services_designer(UmiStudioServices *services);
/**
 * Provide the studio services web platform operation used by this module and its client
 * applications.
 */
UmiStudioWebPlatform *umi_studio_services_web_platform(UmiStudioServices *services);
/**
 * Provide the studio services delivery platform operation used by this module and its
 * client applications.
 */
UmiStudioDeliveryPlatform *umi_studio_services_delivery_platform(
    UmiStudioServices *services);
/**
 * Provide the studio services compatibility platform operation used by this module and its
 * client applications.
 */
UmiStudioCompatibilityPlatform *umi_studio_services_compatibility_platform(
    UmiStudioServices *services
);
/**
 * Provide the studio services extension platform operation used by this module and its
 * client applications.
 */
UmiStudioExtensionPlatform *umi_studio_services_extension_platform(
    UmiStudioServices *services
);
/**
 * Provide the studio services product centre operation used by this module and its client
 * applications.
 */
UmiStudioProductCentre *umi_studio_services_product_centre(
    UmiStudioServices *services
);
/**
 * Provide the studio services build operation used by this module and its client
 * applications.
 */
UmiStudioBuildService *umi_studio_services_build(UmiStudioServices *services);
/**
 * Provide the studio services tests operation used by this module and its client
 * applications.
 */
UmiStudioTestService *umi_studio_services_tests(UmiStudioServices *services);
/**
 * Provide the studio services terminal operation used by this module and its client
 * applications.
 */
UmiStudioTerminalService *umi_studio_services_terminal(
    UmiStudioServices *services
);
/**
 * Provide the studio services terminal controller operation used by this module and its
 * client applications.
 */
UmiTerminalController *umi_studio_services_terminal_controller(
    UmiStudioServices *services
);
/**
 * Provide the studio services language operation used by this module and its client
 * applications.
 */
UmiStudioLanguageService *umi_studio_services_language(
    UmiStudioServices *services
);
/**
 * Provide the studio services debugger operation used by this module and its client
 * applications.
 */
UmiStudioDebuggerService *umi_studio_services_debugger(
    UmiStudioServices *services
);
/**
 * Provide the studio services source control operation used by this module and its client
 * applications.
 */
UmiStudioSourceControlService *umi_studio_services_source_control(
    UmiStudioServices *services
);
/**
 * Provide the studio services trading operation used by this module and its client
 * applications.
 */
UmiStudioTradingService *umi_studio_services_trading(
    UmiStudioServices *services
);
/**
 * Provide the studio services open workspace operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_services_open_workspace(UmiStudioServices *services,
                                             const char *root,
                                             int trusted);
/**
 * Provide the studio services close workspace operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_services_close_workspace(UmiStudioServices *services);

/**
 * Return the number of records represented by studio services diagnostic sink without
 * changing their state.
 */
size_t umi_studio_services_diagnostic_sink_count(
    const UmiStudioServices *services
);

#ifdef __cplusplus
}
#endif

#endif
