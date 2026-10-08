#-----------------------------------------------------------------------------
# Umicom Studio IDE application targets
# File: applications/studio/CMakeLists.txt
#
# PURPOSE:
#   Create explicit Framework-based Studio targets.  The current Studio source
#   is treated as the active product implementation, not as a disposable legacy
#   library.  Reusable capabilities will continue to migrate into Framework in
#   small, independently tested vertical slices.
#
# AUTHOR AND ORGANISATION:
#   Sammy Hegab
#   Umicom Foundation
#
# LICENCE:
#   MIT
#-----------------------------------------------------------------------------
set(UMICOM_STUDIO_VERSION "0.23.0")

# -----------------------------------------------------------------------------
# Studio composition root
# -----------------------------------------------------------------------------
add_library(umicom_studio_core STATIC
    src/app/bootstrap.c
    src/app/federated_content.c
    src/app/federated_interactions.c
    src/app/federated_workspace_session.c
    src/app/services.c
    src/app/trading.c
    src/app/version.c
    src/app/doctor.c
    src/app/diagnostics.c
    src/app/settings.c
    src/app/platform.c
    src/app/toolchain_centre.c
    src/app/session.c
    src/app/documents.c
    src/app/tasks.c
    src/app/commands.c
    src/app/state.c
    src/app/workspace.c
    src/app/files.c
    src/app/search.c
    src/app/processes.c
    src/app/watcher.c
    src/app/data.c
    src/app/database_explorer_centre.c
    src/app/database_explorer_connections.c
    src/app/database_explorer_queries.c
    src/app/database_explorer_migrations.c
    src/app/database_explorer_transfer.c
    src/app/database_explorer_views.c
    src/app/database_explorer_commands.c
    src/app/integration_designer_centre.c
    src/app/integration_designer_connections.c
    src/app/integration_designer_api.c
    src/app/integration_designer_mapping.c
    src/app/integration_designer_workflow.c
    src/app/integration_designer_execution.c
    src/app/integration_designer_monitoring.c
    src/app/integration_designer_views.c
    src/app/integration_designer_commands.c
    src/app/messages.c
    src/app/history.c
    src/app/replay.c
    src/app/fabric.c
    src/app/operations.c
    src/app/operations_centre.c
    src/app/operations_views.c
    src/app/operations_commands.c
    src/app/security.c
    src/app/security_centre.c
    src/app/security_identities.c
    src/app/security_roles.c
    src/app/security_secrets.c
    src/app/security_approvals.c
    src/app/security_packages.c
    src/app/security_audit_view.c
    src/app/security_views.c
    src/app/security_commands.c
    src/app/plugins.c
    src/app/observability.c
    src/app/resilience.c
    src/app/ui.c
    src/app/automation_audit.c
    src/app/application_surface.c
    src/app/application_surface_controllers.c
    src/app/application_surface_commands.c
    src/app/application_surface_policy.c
    src/app/workbench.c
    src/app/professional_workspace.c
    src/app/workspace_catalogue.c
    src/app/workspace_layouts.c
    src/app/workspace_profiles.c
    src/app/workspace_profile_session.c
    src/app/editor_layout_session.c
    src/app/editor_command_contribution.c
    src/app/source_navigation_contribution.c
    src/app/navigation_view_contribution.c
    src/app/editor_intelligence_contribution.c
    src/app/editor_intelligence_workbench_contribution.c
    src/app/vcs_workbench_contribution.c
    src/app/data_workbench_contribution.c
    src/app/web_api_workbench_contribution.c
    src/app/developer_ecosystem_contribution.c
    src/app/test_readiness_contribution.c
    src/app/editor_assistance_contribution.c
    src/app/editor_productivity_contribution.c
    src/app/editor_refactoring_contribution.c
    src/app/debug_orchestration_contribution.c
    src/app/workspace_groups.c
    src/app/workspace_themes.c
    src/app/appearance_centre.c
    src/app/workspace_persistence.c
    src/app/workspace_views.c
    src/app/workspace_commands.c
    src/app/compiler_settings.c
    src/app/compiler_catalogue.c
    src/app/compiler_project_binding.c
    src/app/compiler_templates.c
    src/app/compiler_views.c
    src/app/compiler_commands.c
    src/app/compiler_diagnostics.c
    src/app/compiler_interop.c
    src/app/umicc_integration.c
    src/app/polyglot_workspace.c
    src/app/compiler_export.c
    src/app/compiler_centre.c
    src/app/sdk_centre.c
    src/app/package_project.c
    src/app/dependency_centre.c
    src/app/bundle_designer.c
    src/app/installer_centre.c
    src/app/supply_chain.c
    src/app/release_channels.c
    src/app/upgrade_centre.c
    src/app/distribution_centre.c
    src/app/distribution_views.c
    src/app/distribution_commands.c
    src/app/perspectives.c
    src/app/contributions.c
    src/app/view_models.c
    src/app/workbench_commands.c
    src/app/platform_contract.c
    src/app/workbench_shell_catalogue.c
    src/app/workbench_views.c
    src/app/platform_shell.c
    src/app/command_centre.c
    src/app/resource_explorer.c
    src/app/global_search.c
    src/app/extension_centre.c
    src/app/extension_platform.c
    src/app/product_centre.c
    # Engine Centre is a thin view over Framework-owned engine descriptors.
    src/app/engine_centre.c
    src/app/application_centre.c
    src/app/settings_centre.c
    src/app/welcome_centre.c
    src/app/learning_centre.c
    src/app/developer_dashboard.c
    src/app/editor_workspace.c
    src/app/chart_workspace.c
    src/app/designer_workspace.c
    src/app/ai_workspace.c
    src/app/project_centre.c
    src/app/language_intelligence_centre.c
    src/app/debug_centre.c
    src/app/source_control_centre.c
    src/app/test_explorer_centre.c
    src/app/problems_centre.c
    src/app/output_centre.c
    src/app/progress_centre.c
    src/app/developer_workbench.c
    src/app/developer_session.c
    src/app/developer_pipeline_centre.c
    src/app/developer_task_centre.c
    src/app/developer_run_centre.c
    src/app/developer_workspace_state_centre.c
    src/app/ui_catalogue.c
    src/app/build.c
    src/app/build_workbench.c
    src/app/tests.c
    src/app/terminal.c
    src/app/language.c
    src/app/debugger.c
    src/app/debug_workspace_views.c
    src/app/source_control.c
    src/app/developer_platform.c
    src/app/declarative.c
    src/app/designer.c
    src/app/designer_catalogue.c
    src/app/designer_preview.c
    src/app/designer_project.c
    src/app/new_project.c
    src/app/designer_state.c
    src/app/web_platform.c
    src/app/web.c
    src/app/browser.c
    src/app/charting.c
    src/app/frontend.c
    src/app/frontend_studio_settings.c
    src/app/frontend_studio_preview.c
    src/app/frontend_studio_inspector.c
    src/app/frontend_studio_centre.c
    src/app/frontend_studio_views.c
    src/app/frontend_studio_commands.c
    src/app/ai_chat.c
    src/app/ai_evaluation.c
    src/app/ai_platform.c
    src/app/coding_assistant.c
    src/app/vibe_coding.c
    src/app/knowledge_centre.c
    src/app/knowledge_settings.c
    src/app/knowledge_views.c
    src/app/knowledge_commands.c
    src/app/ai_retrieval.c
    src/app/ai_sources.c
    src/app/ai_tools.c
    src/app/authorengine.c
    src/app/helix.c
    src/app/helix_agents.c
    src/app/helix_candidate.c
    src/app/helix_evidence.c
    src/app/helix_fitness.c
    src/app/helix_generation.c
    src/app/helix_agent_adapters.c
    src/app/helix_agent_centre.c
    src/app/helix_agent_commands.c
    src/app/helix_agent_operations.c
    src/app/helix_agent_views.c
    src/app/helix_agent_candidates.c
    src/app/helix_agent_journal.c
    src/app/helix_agent_settings.c
    src/app/delivery_platform.c
    src/app/compatibility_platform.c
    src/app/sdk.c
    src/app/products.c
    src/app/conformance.c
    src/app/architecture_guard.c
    src/app/architecture_quality.c
    src/app/quality_gate.c
    src/app/duplicate_review.c
    src/app/consolidation.c
    src/app/codeguard.c
    src/app/quality_centre.c
    src/app/quality_evidence.c
    src/app/quality_navigation.c
    src/app/quality_remediation.c
    src/app/quality_duplicates.c
    src/app/quality_compatibility.c
    src/app/quality_gates.c
    src/app/quality_views.c
    src/app/quality_commands.c
    src/app/delivery_release.c
    src/app/delivery_package.c
    src/app/delivery_update.c
    src/app/delivery_report.c
    src/app/delivery_distribution.c
    src/app/delivery_evidence.c
    src/app/visual_builder_centre.c
    src/app/visual_builder_tree.c
    src/app/visual_builder_properties.c
    src/app/visual_builder_bindings.c
    src/app/visual_builder_interactions.c
    src/app/visual_builder_drag_drop.c
    src/app/visual_builder_clipboard.c
    src/app/visual_builder_templates.c
    src/app/visual_builder_layout.c
    src/app/visual_builder_preview.c
    src/app/visual_builder_source.c
    src/app/visual_builder_views.c
    src/app/visual_builder_commands.c
    src/app/visual_builder_settings.c
    src/app/production_profile.c
    src/app/production_panels.c
    src/app/production_commands.c
    src/app/production_readiness.c
    src/experience_acceptance.c
)
add_library(Umicom::StudioCore ALIAS umicom_studio_core)

target_include_directories(umicom_studio_core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(umicom_studio_core PUBLIC
    Umicom::Framework
    Umicom::ui
    Umicom::ui_headless
)

umicom_apply_warnings(umicom_studio_core)
umicom_apply_sanitizers(umicom_studio_core)

# A small console frontend verifies Framework lifecycle without GTK4.
add_executable(umicom-studio-console
    src/console/main.c
)
target_link_libraries(umicom-studio-console PRIVATE
    Umicom::StudioCore
)
umicom_apply_warnings(umicom-studio-console)
umicom_apply_sanitizers(umicom-studio-console)

# Native project-health command.  This is intentionally implemented in C23 so
# normal repository validation does not depend on Python.
add_executable(umicom-studio-doctor
    src/tools/doctor_main.c
)
target_link_libraries(umicom-studio-doctor PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-doctor)
umicom_apply_sanitizers(umicom-studio-doctor)

# Native retained-diagnostics inspection command.
add_executable(umicom-studio-diagnostics
    src/tools/diagnostics_main.c
)
target_link_libraries(umicom-studio-diagnostics PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-diagnostics)
umicom_apply_sanitizers(umicom-studio-diagnostics)


# Native typed-settings inspection and editing command.
add_executable(umicom-studio-settings
    src/tools/settings_main.c
)
target_link_libraries(umicom-studio-settings PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-settings)
umicom_apply_sanitizers(umicom-studio-settings)

# Native Framework platform and toolchain inspection command.
add_executable(umicom-studio-platform
    src/tools/platform_main.c
)
target_link_libraries(umicom-studio-platform PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-platform)
umicom_apply_sanitizers(umicom-studio-platform)

# Native multi-profile Toolchain & Environment Centre inspection command.
# The reusable discovery/catalogue/environment mechanisms remain Framework
# owned; this target is Studio's composition and operator-facing boundary.
add_executable(umicom-studio-toolchain
    src/tools/toolchain_main.c
)
target_link_libraries(umicom-studio-toolchain PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-toolchain)
umicom_apply_sanitizers(umicom-studio-toolchain)

# Native Framework and Studio operational-state command.
add_executable(umicom-studio-state
    src/tools/state_main.c
)
target_link_libraries(umicom-studio-state PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-state)
umicom_apply_sanitizers(umicom-studio-state)

# Native workspace, index and search inspection command.
add_executable(umicom-studio-workspace
    src/tools/workspace_main.c
)
target_link_libraries(umicom-studio-workspace PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-workspace)
umicom_apply_sanitizers(umicom-studio-workspace)

# Native supervised process execution command.
add_executable(umicom-studio-process
    src/tools/process_main.c
)
target_link_libraries(umicom-studio-process PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-process)
umicom_apply_sanitizers(umicom-studio-process)

# Native Data Server inspection command.
add_executable(umicom-studio-data src/tools/data_main.c)
target_link_libraries(umicom-studio-data PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-data)
umicom_apply_sanitizers(umicom-studio-data)

# Database Explorer stays a Studio composition over the authoritative
# Framework Data Server; it never exposes provider connection handles.
add_executable(umicom-studio-database-explorer
    src/tools/database_explorer_main.c)
target_link_libraries(umicom-studio-database-explorer
    PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-database-explorer)
umicom_apply_sanitizers(umicom-studio-database-explorer)

# Integration Designer composes provider-neutral Framework contracts. The
# executable performs only deterministic planning and simulation by default.
add_executable(umicom-studio-integration-designer
    src/tools/integration_designer_main.c)
target_link_libraries(umicom-studio-integration-designer
    PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-integration-designer)
umicom_apply_sanitizers(umicom-studio-integration-designer)

# Native Integration Fabric inspection and publication command.
add_executable(umicom-studio-messages src/tools/messages_main.c)
target_link_libraries(umicom-studio-messages PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-messages)
umicom_apply_sanitizers(umicom-studio-messages)

# Native durable replay command.
add_executable(umicom-studio-replay src/tools/replay_main.c)
target_link_libraries(umicom-studio-replay PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-replay)
umicom_apply_sanitizers(umicom-studio-replay)

# Native Studio operational-history command.
add_executable(umicom-studio-history src/tools/history_main.c)
target_link_libraries(umicom-studio-history PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-history)
umicom_apply_sanitizers(umicom-studio-history)

# Native security, plug-in, observability and resilience commands.
foreach(UMICOM_STUDIO_OPERATION_TOOL
        security
        plugins
        observability
        resilience
        operations)
    add_executable(umicom-studio-${UMICOM_STUDIO_OPERATION_TOOL}
        src/tools/${UMICOM_STUDIO_OPERATION_TOOL}_main.c)
    target_link_libraries(umicom-studio-${UMICOM_STUDIO_OPERATION_TOOL}
        PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-${UMICOM_STUDIO_OPERATION_TOOL})
    umicom_apply_sanitizers(umicom-studio-${UMICOM_STUDIO_OPERATION_TOOL})
endforeach()

# Native Security, Identity, Secrets and Policy Centre command.
add_executable(umicom-studio-security-centre src/tools/security_centre_main.c)
target_link_libraries(umicom-studio-security-centre PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-security-centre)
umicom_apply_sanitizers(umicom-studio-security-centre)

# Native Operations Centre and Professional Workspace Designer commands.
add_executable(umicom-studio-operations-centre src/tools/operations_centre_main.c)
target_link_libraries(umicom-studio-operations-centre PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-operations-centre)
umicom_apply_sanitizers(umicom-studio-operations-centre)

add_executable(umicom-studio-workspace-designer src/tools/workspace_designer_main.c)
target_link_libraries(umicom-studio-workspace-designer PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-workspace-designer)
umicom_apply_sanitizers(umicom-studio-workspace-designer)

# Batch 58 native Compiler Centre and polyglot planning commands.
add_executable(umicom-studio-compiler-centre src/tools/compiler_centre_main.c)
target_link_libraries(umicom-studio-compiler-centre PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-compiler-centre)
umicom_apply_sanitizers(umicom-studio-compiler-centre)

add_executable(umicom-studio-polyglot-compiler src/tools/polyglot_compiler_main.c)
target_link_libraries(umicom-studio-polyglot-compiler PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-polyglot-compiler)
umicom_apply_sanitizers(umicom-studio-polyglot-compiler)

# Batch 59 native SDK, package, installer and distribution commands.
add_executable(umicom-studio-distribution-centre
    src/tools/distribution_centre_main.c)
target_link_libraries(umicom-studio-distribution-centre
    PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-distribution-centre)
umicom_apply_sanitizers(umicom-studio-distribution-centre)

add_executable(umicom-studio-package-inspector
    src/tools/package_inspector_main.c)
target_link_libraries(umicom-studio-package-inspector
    PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-package-inspector)
umicom_apply_sanitizers(umicom-studio-package-inspector)

# Native toolkit-neutral workbench and UI catalogue commands.
add_executable(umicom-studio-workbench src/tools/workbench_main.c)
target_link_libraries(umicom-studio-workbench PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-workbench)
umicom_apply_sanitizers(umicom-studio-workbench)

add_executable(umicom-studio-ui-catalogue src/tools/ui_catalogue_main.c)
target_link_libraries(umicom-studio-ui-catalogue PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-ui-catalogue)
umicom_apply_sanitizers(umicom-studio-ui-catalogue)

# Native developer-platform commands for build, test, terminal, language,
# debugging, source control and the combined platform report.
foreach(UMICOM_STUDIO_DEVELOPER_TOOL
        build
        tests
        terminal
        language
        debugger
        source-control
        developer
        developer-pipeline
        developer-state)
    string(REPLACE "-" "_" UMICOM_STUDIO_DEVELOPER_SOURCE
        "${UMICOM_STUDIO_DEVELOPER_TOOL}")
    add_executable(umicom-studio-${UMICOM_STUDIO_DEVELOPER_TOOL}
        src/tools/${UMICOM_STUDIO_DEVELOPER_SOURCE}_main.c)
    target_link_libraries(umicom-studio-${UMICOM_STUDIO_DEVELOPER_TOOL}
        PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-${UMICOM_STUDIO_DEVELOPER_TOOL})
    umicom_apply_sanitizers(umicom-studio-${UMICOM_STUDIO_DEVELOPER_TOOL})
endforeach()

# Native declarative-template and visual-designer commands.
foreach(UMICOM_STUDIO_DESIGNER_TOOL
        template
        designer
        designer-catalogue
        visual-builder
        new-project)
    string(REPLACE "-" "_" UMICOM_STUDIO_DESIGNER_SOURCE
        "${UMICOM_STUDIO_DESIGNER_TOOL}")
    add_executable(umicom-studio-${UMICOM_STUDIO_DESIGNER_TOOL}
        src/tools/${UMICOM_STUDIO_DESIGNER_SOURCE}_main.c)
    target_link_libraries(umicom-studio-${UMICOM_STUDIO_DESIGNER_TOOL}
        PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-${UMICOM_STUDIO_DESIGNER_TOOL})
    umicom_apply_sanitizers(umicom-studio-${UMICOM_STUDIO_DESIGNER_TOOL})
endforeach()


# Native Web Server and multi-frontend platform commands.
foreach(UMICOM_STUDIO_WEB_TOOL
        web
        browser
        chart
        frontend
        frontend-studio
        web-platform)
    string(REPLACE "-" "_" UMICOM_STUDIO_WEB_SOURCE
        "${UMICOM_STUDIO_WEB_TOOL}")
    add_executable(umicom-studio-${UMICOM_STUDIO_WEB_TOOL}
        src/tools/${UMICOM_STUDIO_WEB_SOURCE}_main.c)
    target_link_libraries(umicom-studio-${UMICOM_STUDIO_WEB_TOOL}
        PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-${UMICOM_STUDIO_WEB_TOOL})
    umicom_apply_sanitizers(umicom-studio-${UMICOM_STUDIO_WEB_TOOL})
endforeach()


# Native AI, AuthorEngine and Helix commands.
foreach(UMICOM_STUDIO_AI_TOOL
        ai
        retrieval
        authorengine
        knowledge
        helix
        helix-evidence
        helix-generation
        helix-agent-runtime)
    string(REPLACE "-" "_" UMICOM_STUDIO_AI_SOURCE
        "${UMICOM_STUDIO_AI_TOOL}")
    add_executable(umicom-studio-${UMICOM_STUDIO_AI_TOOL}
        src/tools/${UMICOM_STUDIO_AI_SOURCE}_main.c)
    target_link_libraries(umicom-studio-${UMICOM_STUDIO_AI_TOOL}
        PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-${UMICOM_STUDIO_AI_TOOL})
    umicom_apply_sanitizers(umicom-studio-${UMICOM_STUDIO_AI_TOOL})
endforeach()

# -----------------------------------------------------------------------------
# GTK4 Studio product implementation
# -----------------------------------------------------------------------------

# Native application-delivery, packaging, update and release-engineering commands.
foreach(UMICOM_STUDIO_DELIVERY_TOOL
        delivery
        package
        release
        update
        distribution
        evidence
        rollback
        pipeline)
    add_executable(umicom-studio-${UMICOM_STUDIO_DELIVERY_TOOL}
        src/tools/${UMICOM_STUDIO_DELIVERY_TOOL}_main.c)
    target_link_libraries(umicom-studio-${UMICOM_STUDIO_DELIVERY_TOOL}
        PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-${UMICOM_STUDIO_DELIVERY_TOOL})
    umicom_apply_sanitizers(umicom-studio-${UMICOM_STUDIO_DELIVERY_TOOL})
endforeach()


# Native CodeGuard and consolidation commands.
add_executable(umicom-studio-codeguard src/tools/codeguard_main.c)
target_link_libraries(umicom-studio-codeguard PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-codeguard)
umicom_apply_sanitizers(umicom-studio-codeguard)
add_executable(umicom-studio-consolidation src/tools/consolidation_main.c)
target_link_libraries(umicom-studio-consolidation PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-consolidation)
umicom_apply_sanitizers(umicom-studio-consolidation)
add_executable(umicom-studio-duplicates src/tools/duplicates_main.c)
target_link_libraries(umicom-studio-duplicates PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-duplicates)
umicom_apply_sanitizers(umicom-studio-duplicates)
add_executable(umicom-studio-quality-centre src/tools/quality_centre_main.c)
target_link_libraries(umicom-studio-quality-centre PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-quality-centre)
umicom_apply_sanitizers(umicom-studio-quality-centre)
add_executable(umicom-studio-quality src/tools/quality_main.c)
target_link_libraries(umicom-studio-quality PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-quality)
umicom_apply_sanitizers(umicom-studio-quality)

# Create this optional product surface only when its build option is enabled.
if(UMICOM_STUDIO_BUILD_GTK)
    find_package(PkgConfig REQUIRED)

    pkg_check_modules(GLIB REQUIRED IMPORTED_TARGET glib-2.0)
    pkg_check_modules(GOBJECT REQUIRED IMPORTED_TARGET gobject-2.0)
    pkg_check_modules(GIO REQUIRED IMPORTED_TARGET gio-2.0)
    pkg_check_modules(JSON_GLIB REQUIRED IMPORTED_TARGET json-glib-1.0)
    pkg_check_modules(SOUP3 REQUIRED IMPORTED_TARGET libsoup-3.0)
    pkg_check_modules(CURL REQUIRED IMPORTED_TARGET libcurl)
    pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4>=4.10)
    pkg_check_modules(PIXBUF REQUIRED IMPORTED_TARGET gdk-pixbuf-2.0)
    pkg_check_modules(CAIRO REQUIRED IMPORTED_TARGET cairo)
    pkg_check_modules(PANGO REQUIRED IMPORTED_TARGET pango)
    pkg_check_modules(HARFBUZZ REQUIRED IMPORTED_TARGET harfbuzz)
    pkg_check_modules(GRAPHENE REQUIRED IMPORTED_TARGET graphene-1.0)
    pkg_check_modules(SOURCEVIEW5 QUIET IMPORTED_TARGET gtksourceview-5)

    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/StudioSources.cmake")

    add_library(umicom_studio_product STATIC
        ${UMICOM_STUDIO_PRODUCT_SOURCES}
    )
    add_library(Umicom::StudioProduct ALIAS umicom_studio_product)

    # These paths preserve the current product's include relationships.  Later
    # extraction commits will reduce this list as each bounded module receives
    # a public contract and its own CMake target.
    target_include_directories(umicom_studio_product PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/build/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/core/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/editor/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/app/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/chrome/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/dialogs/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/llm_lab/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/menu/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/prefs/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/theme"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/llm"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/llm/includes"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/llm/providers"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/panes/chat/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/panes/output/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/panes/problems/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/plugins/transpile"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/plugins/transpile/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/search"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/search/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/resources"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/util/fs/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/util/git/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/util/log/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/util/sys/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/util/watchers/include"
    )

    target_compile_definitions(umicom_studio_product PRIVATE
        NOMINMAX
        _CRT_SECURE_NO_WARNINGS
        UMICOM_VERSION_MAJOR=0
        UMICOM_VERSION_MINOR=17
        UMICOM_VERSION_PATCH=0
    )

    target_link_libraries(umicom_studio_product
        PUBLIC
            Umicom::Framework
        PRIVATE
            PkgConfig::GLIB
            PkgConfig::GOBJECT
            PkgConfig::GIO
            PkgConfig::JSON_GLIB
            PkgConfig::SOUP3
            PkgConfig::CURL
            PkgConfig::GTK4
            PkgConfig::PIXBUF
            PkgConfig::CAIRO
            PkgConfig::PANGO
            PkgConfig::HARFBUZZ
            PkgConfig::GRAPHENE
    )

    # Configure the optional target only when its feature has created it.
    if(TARGET PkgConfig::SOURCEVIEW5)
        target_link_libraries(umicom_studio_product PRIVATE
            PkgConfig::SOURCEVIEW5
        )
    endif()

    # The product implementation is not warning-clean yet.  Keep a practical
    # warning set here while new Framework and StudioCore code remains strict.
    if(MSVC)
        target_compile_options(umicom_studio_product PRIVATE /W4)
    # Use this fallback path when the earlier condition does not apply.
    else()
        target_compile_options(umicom_studio_product PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wno-unused-parameter
        )
    endif()

    # This focused non-visual test exercises StudioProduct's project lifecycle
    # while also compiling its headers beside Framework's UmiStatus contract.
    if(BUILD_TESTING)
        add_executable(umicom-studio-project-manager-indexing-test
            tests/test_project_manager_indexing.c)
        target_include_directories(
            umicom-studio-project-manager-indexing-test PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/src/core/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/chrome/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/util/fs/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/util/log/include")
        target_link_libraries(
            umicom-studio-project-manager-indexing-test PRIVATE
            Umicom::StudioProduct
            PkgConfig::GTK4
            PkgConfig::GLIB)
        umicom_apply_warnings(umicom-studio-project-manager-indexing-test)
        umicom_apply_sanitizers(umicom-studio-project-manager-indexing-test)
        add_test(NAME studio.project_manager.indexing
            COMMAND umicom-studio-project-manager-indexing-test)
    endif()

    add_executable(umicom-studio-workbench-demo
        src/gtk/workbench_main.c
        src/gui/workbench/workbench_window.c
        src/gui/workbench/perspective_switcher.c
        src/gui/workbench/status_strip.c
        src/gui/explorer/project_explorer_view.c
        src/gui/editor/editor_host_view.c
    )
    target_include_directories(umicom-studio-workbench-demo PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/explorer/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/editor/include"
    )
    target_link_libraries(umicom-studio-workbench-demo PRIVATE
        Umicom::StudioCore
        Umicom::ui_gtk4
        PkgConfig::GTK4
        PkgConfig::GLIB
        PkgConfig::GIO
    )
    umicom_apply_warnings(umicom-studio-workbench-demo)
    umicom_apply_sanitizers(umicom-studio-workbench-demo)

    # Compile the actual Studio GTK adapter into an isolated acceptance host.
    # The native C fixture disables presentation, tool discovery and personal
    # session storage; it checks real editor bodies rather than a suite preview.
    if(BUILD_TESTING)
        # Drive the native shortcut route and retain its controller at teardown.
        add_executable(umicom-studio-navigation-history-test
            tests/test_navigation_history_gtk4.c src/gui/workbench/workbench_window.c)
        target_include_directories(umicom-studio-navigation-history-test PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
        target_link_libraries(umicom-studio-navigation-history-test PRIVATE
            Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
        umicom_apply_warnings(umicom-studio-navigation-history-test)
        umicom_apply_sanitizers(umicom-studio-navigation-history-test)
        add_test(NAME studio.navigation.history.gtk4 COMMAND umicom-studio-navigation-history-test)
        set_tests_properties(studio.navigation.history.gtk4 PROPERTIES
            SKIP_RETURN_CODE 77 TIMEOUT 60 RUN_SERIAL TRUE LABELS "studio;gtk4;navigation;acceptance")
        if(COMMAND umicom_register_validation_target)
            umicom_register_validation_target(umicom-studio-navigation-history-test)
        endif()
        # Search scope is exercised through real entries, buttons and services.
        add_executable(umicom-studio-search-filter-test
            tests/test_search_filter_gtk4.c src/gui/workbench/workbench_window.c)
        target_include_directories(umicom-studio-search-filter-test PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
        target_link_libraries(umicom-studio-search-filter-test PRIVATE
            Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
        umicom_apply_warnings(umicom-studio-search-filter-test)
        umicom_apply_sanitizers(umicom-studio-search-filter-test)
        add_test(NAME studio.file_search.filter.gtk4 COMMAND umicom-studio-search-filter-test)
        set_tests_properties(studio.file_search.filter.gtk4 PROPERTIES
            SKIP_RETURN_CODE 77 TIMEOUT 60 RUN_SERIAL TRUE LABELS "studio;gtk4;search;acceptance")
        if(COMMAND umicom_register_validation_target)
            umicom_register_validation_target(umicom-studio-search-filter-test)
        endif()
        # Exercise the real modal replacement review in isolated product graphs.
        add_executable(umicom-studio-replacement-review-test
            tests/test_replace_review_gtk4.c src/gui/workbench/workbench_window.c)
        target_include_directories(umicom-studio-replacement-review-test PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
        target_link_libraries(umicom-studio-replacement-review-test PRIVATE
            Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
        umicom_apply_warnings(umicom-studio-replacement-review-test)
        umicom_apply_sanitizers(umicom-studio-replacement-review-test)
        foreach(case preview cancel apply stale inactive closed teardown no-match)
            add_test(NAME studio.replacement_review.gtk4.${case}
                COMMAND umicom-studio-replacement-review-test ${case})
            set_tests_properties(studio.replacement_review.gtk4.${case} PROPERTIES
                SKIP_RETURN_CODE 77 TIMEOUT 60 LABELS "studio;gtk4;document;replacement;acceptance")
        endforeach()
        if(COMMAND umicom_register_validation_target)
            umicom_register_validation_target(umicom-studio-replacement-review-test)
        endif()
        add_executable(umicom-studio-workspace-canvas-test
            tests/test_workspace_canvas_gtk4.c
            src/gui/workbench/workbench_window.c)
        target_include_directories(umicom-studio-workspace-canvas-test PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
        target_link_libraries(umicom-studio-workspace-canvas-test PRIVATE
            Umicom::StudioCore Umicom::ui_gtk4
            PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
        umicom_apply_warnings(umicom-studio-workspace-canvas-test)
        umicom_apply_sanitizers(umicom-studio-workspace-canvas-test)
        add_test(NAME studio.workspace.canvas.gtk4
            COMMAND umicom-studio-workspace-canvas-test)
        set_tests_properties(studio.workspace.canvas.gtk4 PROPERTIES
            SKIP_RETURN_CODE 77
            TIMEOUT 60
            LABELS "studio;gtk4;workspace;canvas;acceptance")
        if(COMMAND umicom_register_validation_target)
            umicom_register_validation_target(umicom-studio-workspace-canvas-test)
        endif()
    endif()

    add_executable(umicom-studio-designer-demo
        src/gtk/designer_main.c
        src/gui/designer/designer_window.c
        src/gui/designer/palette_view.c
        src/gui/designer/tree_view.c
        src/gui/designer/inspector_view.c
        src/gui/designer/surface_view.c
        src/gui/designer/preview_view.c
    )
    target_include_directories(umicom-studio-designer-demo PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/designer/include"
    )
    target_link_libraries(umicom-studio-designer-demo PRIVATE
        Umicom::StudioCore
        Umicom::ui_gtk4
        PkgConfig::GTK4
        PkgConfig::GLIB
    )
    umicom_apply_warnings(umicom-studio-designer-demo)
    umicom_apply_sanitizers(umicom-studio-designer-demo)


    add_executable(umicom-studio-web-platform-demo
        src/gui/web/web_platform_demo.c
        src/gui/web/web_preview.c
        src/gui/web/chart_preview.c
        src/gui/web/frontend_preview.c
    )
    target_include_directories(umicom-studio-web-platform-demo PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/web/include"
    )
    target_link_libraries(umicom-studio-web-platform-demo PRIVATE
        Umicom::StudioCore
        PkgConfig::GTK4
        PkgConfig::GLIB
        PkgConfig::GIO
    )
    umicom_apply_warnings(umicom-studio-web-platform-demo)
    umicom_apply_sanitizers(umicom-studio-web-platform-demo)

    add_executable(umicom-studio-ide
        src/gtk/main.c
        src/gui/workbench/workbench_window.c
        src/ui/icon.c
        src/ui/splash.c
    )

    # Define the local fallback only when the parent build did not provide the shared helper.
    if(NOT COMMAND umicom_apply_application_branding)
        include(
            "${CMAKE_CURRENT_SOURCE_DIR}/../../framework/cmake/UmicomApplicationBranding.cmake")
    endif()
    # Studio receives its executable icon, taskbar identity and runtime vectors
    # from Framework rather than maintaining a second brand resource file.
    if(UMICOM_STUDIO_GUI)
        umicom_apply_application_branding(
            TARGET umicom-studio-ide
            PRODUCT_NAME "Umicom Studio IDE"
            INTERNAL_NAME "umicom-studio-ide"
            APPLICATION_ID "org.umicom.studio.framework-workbench"
            VERSION_MAJOR ${PROJECT_VERSION_MAJOR}
            VERSION_MINOR ${PROJECT_VERSION_MINOR}
            VERSION_PATCH ${PROJECT_VERSION_PATCH}
            RESOURCE_ROOT
                "${CMAKE_CURRENT_SOURCE_DIR}/../../framework/resources"
            DESKTOP_ENTRY
            WINDOWS_GUI)
    # Use this fallback path when the earlier condition does not apply.
    else()
        umicom_apply_application_branding(
            TARGET umicom-studio-ide
            PRODUCT_NAME "Umicom Studio IDE"
            INTERNAL_NAME "umicom-studio-ide"
            APPLICATION_ID "org.umicom.studio.framework-workbench"
            VERSION_MAJOR ${PROJECT_VERSION_MAJOR}
            VERSION_MINOR ${PROJECT_VERSION_MINOR}
            VERSION_PATCH ${PROJECT_VERSION_PATCH}
            RESOURCE_ROOT
                "${CMAKE_CURRENT_SOURCE_DIR}/../../framework/resources"
            DESKTOP_ENTRY)
    endif()

    target_include_directories(umicom-studio-ide PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/app/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/include"
        "${CMAKE_SOURCE_DIR}/win"
    )

    target_link_libraries(umicom-studio-ide PRIVATE
        Umicom::StudioCore
        Umicom::StudioProduct
        Umicom::ui_gtk4
        PkgConfig::GTK4
        PkgConfig::GLIB
        PkgConfig::GIO
    )

    target_compile_definitions(umicom-studio-ide PRIVATE
        USIDE_GUI=$<IF:$<BOOL:${UMICOM_STUDIO_GUI}>,1,0>
        NOMINMAX
        _CRT_SECURE_NO_WARNINGS
        UMICOM_VERSION_MAJOR=0
        UMICOM_VERSION_MINOR=17
        UMICOM_VERSION_PATCH=0
    )

    install(TARGETS
        umicom-studio-ide
        umicom-studio-workbench-demo
        RUNTIME DESTINATION bin)
endif()

# The Engine Centre test verifies Studio does not duplicate Framework maturity.
if(BUILD_TESTING)
    add_executable(umicom-studio-engine-centre-test
        tests/test_engine_centre.c)
    target_link_libraries(umicom-studio-engine-centre-test
        PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-engine-centre-test)
    umicom_apply_sanitizers(umicom-studio-engine-centre-test)
    add_test(NAME studio.engine_centre
        COMMAND umicom-studio-engine-centre-test)
endif()

# Studio adopts the Framework application-surface runtime as an additive
# product layer. These focused tests do not require a graphical display.
if(BUILD_TESTING)
    # Use the shared build helper when it is available from the parent composition.
    if(COMMAND umicom_register_header_governance_test)
        umicom_register_header_governance_test(
            studio.sdk.public_headers
            "${CMAKE_CURRENT_SOURCE_DIR}/include"
            "Umicom Studio IDE")
    endif()
    # Visit each bounded item once so every record receives the same rule.
    foreach(UMICOM_STUDIO_APPLICATION_SURFACE_TEST
            application_surface
            application_surface_commands
            application_surface_controllers
            application_surface_policy)
        string(REPLACE "_" "-" UMICOM_STUDIO_APPLICATION_SURFACE_TARGET
            "${UMICOM_STUDIO_APPLICATION_SURFACE_TEST}")
        add_executable(
            "umicom-studio-${UMICOM_STUDIO_APPLICATION_SURFACE_TARGET}-test"
            "tests/test_${UMICOM_STUDIO_APPLICATION_SURFACE_TEST}.c")
        target_link_libraries(
            "umicom-studio-${UMICOM_STUDIO_APPLICATION_SURFACE_TARGET}-test"
            PRIVATE Umicom::StudioCore)
        umicom_apply_warnings(
            "umicom-studio-${UMICOM_STUDIO_APPLICATION_SURFACE_TARGET}-test")
        umicom_apply_sanitizers(
            "umicom-studio-${UMICOM_STUDIO_APPLICATION_SURFACE_TARGET}-test")
        add_test(
            NAME "studio.${UMICOM_STUDIO_APPLICATION_SURFACE_TEST}"
            COMMAND
                "umicom-studio-${UMICOM_STUDIO_APPLICATION_SURFACE_TARGET}-test")
    endforeach()
endif()

# -----------------------------------------------------------------------------
# Framework integration tests
# -----------------------------------------------------------------------------

# Native ABI, SDK, reference-product and architecture-conformance commands.
foreach(UMICOM_STUDIO_CONFORMANCE_TOOL
        abi
        sdk
        products
        compatibility
        conformance
        architecture)
    add_executable(umicom-studio-${UMICOM_STUDIO_CONFORMANCE_TOOL}
        src/tools/${UMICOM_STUDIO_CONFORMANCE_TOOL}_main.c)
    target_link_libraries(umicom-studio-${UMICOM_STUDIO_CONFORMANCE_TOOL}
        PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-${UMICOM_STUDIO_CONFORMANCE_TOOL})
    umicom_apply_sanitizers(umicom-studio-${UMICOM_STUDIO_CONFORMANCE_TOOL})
endforeach()

# -----------------------------------------------------------------------------
# Compatibility fallback for simple native tool sources
# -----------------------------------------------------------------------------
# Batch 26 introduced this generic tool-discovery path.  It is intentionally
# retained rather than deleted.  The explicit target definitions above remain
# canonical because they preserve the existing grouped architecture and
# documentation.  The NOT TARGET guard makes this block additive: it only
# creates a command if a future source exists and no canonical target has
# already claimed that executable name.
set(UMICOM_STUDIO_INSTALL_TARGETS umicom-studio-console)
set(UMICOM_STUDIO_SIMPLE_TOOLS
    doctor diagnostics settings platform state workspace process data messages replay history
    security plugins observability resilience operations workbench ui_catalogue
    build tests terminal language debugger source_control developer template designer designer_catalogue new_project
    web browser chart frontend web_platform ai retrieval authorengine knowledge helix helix_evidence helix_generation
    delivery package release update distribution evidence rollback pipeline abi sdk products compatibility conformance architecture
    codeguard consolidation duplicates quality)
# Visit each bounded item once so every record receives the same rule.
foreach(tool IN LISTS UMICOM_STUDIO_SIMPLE_TOOLS)
    string(REPLACE "_" "-" exe "${tool}")
    # Use the optional file only when it is present in this checkout.
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/tools/${tool}_main.c"
       AND NOT TARGET umicom-studio-${exe})
        add_executable(umicom-studio-${exe} src/tools/${tool}_main.c)
        target_link_libraries(umicom-studio-${exe} PRIVATE Umicom::StudioCore)
        umicom_apply_warnings(umicom-studio-${exe})
        umicom_apply_sanitizers(umicom-studio-${exe})
        list(APPEND UMICOM_STUDIO_INSTALL_TARGETS umicom-studio-${exe})
    endif()
endforeach()

# Register verification targets only when the developer has enabled testing.
if(BUILD_TESTING)
    add_executable(umicom-studio-framework-bootstrap-test
        tests/test_framework_bootstrap.c
    )
    target_link_libraries(umicom-studio-framework-bootstrap-test PRIVATE
        Umicom::StudioCore
    )
    umicom_apply_warnings(umicom-studio-framework-bootstrap-test)
    umicom_apply_sanitizers(umicom-studio-framework-bootstrap-test)

    add_test(
        NAME studio.framework_bootstrap
        COMMAND umicom-studio-framework-bootstrap-test
    )

    # Unified workbench and reusable platform shell coverage.
    foreach(UMICOM_STUDIO_PLATFORM_TEST
            workbench_shell_catalogue
            workbench_activity
            workbench_keybinding
            workbench_session_state
            workbench_views
            source_control_workbench
            platform_shell_expansion
            editor_workspace
            editor_workbench_commands
            developer_project_centre
            developer_workspace_model
            developer_workspace_discovery
            developer_workspace_inheritance
            developer_language_intelligence_centre
            developer_debug_centre
            developer_source_control_centre
            developer_test_explorer_centre
            developer_developer_workbench
            developer_developer_session
            developer_pipeline_centre
            developer_runtime_binding
            developer_runtime_execution
            developer_task_centre
            developer_run_centre
            developer_workspace_state_centre
            developer_workflow_centre
            developer_workbench_automation)
        add_executable(umicom-studio-${UMICOM_STUDIO_PLATFORM_TEST}-test
            tests/test_${UMICOM_STUDIO_PLATFORM_TEST}.c)
        target_link_libraries(umicom-studio-${UMICOM_STUDIO_PLATFORM_TEST}-test PRIVATE Umicom::StudioCore)
        umicom_apply_warnings(umicom-studio-${UMICOM_STUDIO_PLATFORM_TEST}-test)
        umicom_apply_sanitizers(umicom-studio-${UMICOM_STUDIO_PLATFORM_TEST}-test)
        add_test(NAME studio.platform.${UMICOM_STUDIO_PLATFORM_TEST}
            COMMAND umicom-studio-${UMICOM_STUDIO_PLATFORM_TEST}-test)
    endforeach()

    # Visit each bounded item once so every record receives the same rule.
    foreach(UMICOM_STUDIO_INTEGRATION_DESIGNER_TEST
            integration_designer_centre
            integration_designer_connections_api
            integration_designer_mapping
            integration_designer_workflow
            integration_designer_execution
            integration_designer_monitoring
            integration_designer_views_commands)
        string(REPLACE "_" "-" UMICOM_STUDIO_INTEGRATION_DESIGNER_TEST_TARGET
            "${UMICOM_STUDIO_INTEGRATION_DESIGNER_TEST}")
        add_executable(umicom-studio-${UMICOM_STUDIO_INTEGRATION_DESIGNER_TEST_TARGET}-test
            tests/test_${UMICOM_STUDIO_INTEGRATION_DESIGNER_TEST}.c)
        target_link_libraries(umicom-studio-${UMICOM_STUDIO_INTEGRATION_DESIGNER_TEST_TARGET}-test
            PRIVATE Umicom::StudioCore)
        umicom_apply_warnings(umicom-studio-${UMICOM_STUDIO_INTEGRATION_DESIGNER_TEST_TARGET}-test)
        umicom_apply_sanitizers(umicom-studio-${UMICOM_STUDIO_INTEGRATION_DESIGNER_TEST_TARGET}-test)
        add_test(NAME studio.${UMICOM_STUDIO_INTEGRATION_DESIGNER_TEST}
            COMMAND umicom-studio-${UMICOM_STUDIO_INTEGRATION_DESIGNER_TEST_TARGET}-test)
    endforeach()

    # Filesystem project-import coverage extends the existing platform tests.
    add_executable(umicom-studio-developer-project-import-test
        tests/test_developer_project_import.c)
    target_link_libraries(umicom-studio-developer-project-import-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-developer-project-import-test)
    umicom_apply_sanitizers(umicom-studio-developer-project-import-test)
    add_test(NAME studio.platform.developer_project_import
        COMMAND umicom-studio-developer-project-import-test)

    # Batch 35 Studio composition coverage: selection/environment snapshots,
    # deterministic per-project binding and compile_commands.json import.
    foreach(UMICOM_STUDIO_TOOLCHAIN_TEST
            centre
            project_binding
            compilation_database)
        add_executable(umicom-studio-toolchain-${UMICOM_STUDIO_TOOLCHAIN_TEST}-test
            tests/test_toolchain_${UMICOM_STUDIO_TOOLCHAIN_TEST}.c)
        target_link_libraries(
            umicom-studio-toolchain-${UMICOM_STUDIO_TOOLCHAIN_TEST}-test
            PRIVATE Umicom::StudioCore)
        umicom_apply_warnings(
            umicom-studio-toolchain-${UMICOM_STUDIO_TOOLCHAIN_TEST}-test)
        umicom_apply_sanitizers(
            umicom-studio-toolchain-${UMICOM_STUDIO_TOOLCHAIN_TEST}-test)
        add_test(NAME studio.toolchain.${UMICOM_STUDIO_TOOLCHAIN_TEST}
            COMMAND umicom-studio-toolchain-${UMICOM_STUDIO_TOOLCHAIN_TEST}-test)
    endforeach()

    add_executable(umicom-studio-version-test
        tests/test_version.c
    )
    target_link_libraries(umicom-studio-version-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-version-test)
    add_test(NAME studio.version COMMAND umicom-studio-version-test)

    add_executable(umicom-studio-services-test
        tests/test_services.c
    )
    target_link_libraries(umicom-studio-services-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-services-test)
    add_test(NAME studio.services COMMAND umicom-studio-services-test)

    add_executable(umicom-studio-doctor-test
        tests/test_doctor.c
    )
    target_link_libraries(umicom-studio-doctor-test PRIVATE Umicom::StudioCore)
    target_compile_definitions(umicom-studio-doctor-test PRIVATE
        UMICOM_STUDIO_SOURCE_ROOT="${CMAKE_SOURCE_DIR}"
    )
    umicom_apply_warnings(umicom-studio-doctor-test)
    add_test(NAME studio.doctor COMMAND umicom-studio-doctor-test)

    add_executable(umicom-studio-diagnostics-test
        tests/test_diagnostics.c
    )
    target_link_libraries(umicom-studio-diagnostics-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-diagnostics-test)
    umicom_apply_sanitizers(umicom-studio-diagnostics-test)
    add_test(NAME studio.diagnostics COMMAND umicom-studio-diagnostics-test)


    add_executable(umicom-studio-platform-test
        tests/test_platform.c
    )
    target_link_libraries(umicom-studio-platform-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-platform-test)
    umicom_apply_sanitizers(umicom-studio-platform-test)
    add_test(NAME studio.platform COMMAND umicom-studio-platform-test)

    add_executable(umicom-studio-settings-test
        tests/test_settings.c
    )
    target_link_libraries(umicom-studio-settings-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-settings-test)
    umicom_apply_sanitizers(umicom-studio-settings-test)
    add_test(NAME studio.settings COMMAND umicom-studio-settings-test)

    add_executable(umicom-studio-documents-test
        tests/test_documents.c
    )
    target_link_libraries(umicom-studio-documents-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-documents-test)
    umicom_apply_sanitizers(umicom-studio-documents-test)
    add_test(NAME studio.documents COMMAND umicom-studio-documents-test)

    add_executable(umicom-studio-document-workbench-test
        tests/test_document_workbench.c
    )
    target_link_libraries(umicom-studio-document-workbench-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-document-workbench-test)
    umicom_apply_sanitizers(umicom-studio-document-workbench-test)
    add_test(NAME studio.document_workbench COMMAND umicom-studio-document-workbench-test)

    add_executable(umicom-studio-tasks-test
        tests/test_tasks.c
    )
    target_link_libraries(umicom-studio-tasks-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-tasks-test)
    umicom_apply_sanitizers(umicom-studio-tasks-test)
    add_test(NAME studio.tasks COMMAND umicom-studio-tasks-test)

    add_executable(umicom-studio-commands-test
        tests/test_commands.c
    )
    target_link_libraries(umicom-studio-commands-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-commands-test)
    umicom_apply_sanitizers(umicom-studio-commands-test)
    add_test(NAME studio.commands COMMAND umicom-studio-commands-test)

    add_executable(umicom-studio-state-test
        tests/test_state.c
    )
    target_link_libraries(umicom-studio-state-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-state-test)
    umicom_apply_sanitizers(umicom-studio-state-test)
    add_test(NAME studio.state COMMAND umicom-studio-state-test)

    # Runtime composition contract and regression shield.  These tests validate
    # identities and authoritative registry inventories while preserving the
    # extensibility needed by designer, plug-in and future Studio commands.
    add_executable(umicom-studio-platform-contract-test tests/test_platform_contract.c)
    target_link_libraries(umicom-studio-platform-contract-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-platform-contract-test)
    umicom_apply_sanitizers(umicom-studio-platform-contract-test)
    add_test(NAME studio.platform_contract COMMAND umicom-studio-platform-contract-test)

    add_executable(umicom-studio-command-inventory-test tests/test_command_inventory.c)
    target_link_libraries(umicom-studio-command-inventory-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-command-inventory-test)
    umicom_apply_sanitizers(umicom-studio-command-inventory-test)
    add_test(NAME studio.command_inventory COMMAND umicom-studio-command-inventory-test)

    add_executable(umicom-studio-automation-audit-test
        tests/test_automation_audit.c)
    target_link_libraries(umicom-studio-automation-audit-test PRIVATE
        Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-automation-audit-test)
    umicom_apply_sanitizers(umicom-studio-automation-audit-test)
    add_test(NAME studio.automation.wiring
        COMMAND umicom-studio-automation-audit-test)

    add_executable(umicom-studio-command-inventory-partition-test tests/test_command_inventory_partition.c)
    target_link_libraries(umicom-studio-command-inventory-partition-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-command-inventory-partition-test)
    umicom_apply_sanitizers(umicom-studio-command-inventory-partition-test)
    add_test(NAME studio.command_inventory_partition COMMAND umicom-studio-command-inventory-partition-test)

    add_executable(umicom-studio-service-inventory-test tests/test_service_inventory.c)
    target_link_libraries(umicom-studio-service-inventory-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-service-inventory-test)
    umicom_apply_sanitizers(umicom-studio-service-inventory-test)
    add_test(NAME studio.service_inventory COMMAND umicom-studio-service-inventory-test)

    add_executable(umicom-studio-platform-contract-missing-test tests/test_platform_contract_missing.c)
    target_link_libraries(umicom-studio-platform-contract-missing-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-platform-contract-missing-test)
    umicom_apply_sanitizers(umicom-studio-platform-contract-missing-test)
    add_test(NAME studio.platform_contract_missing COMMAND umicom-studio-platform-contract-missing-test)

    add_executable(umicom-studio-version-contract-test tests/test_version_contract.c)
    target_link_libraries(umicom-studio-version-contract-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-version-contract-test)
    umicom_apply_sanitizers(umicom-studio-version-contract-test)
    add_test(NAME studio.version_contract COMMAND umicom-studio-version-contract-test)

    add_executable(umicom-studio-framework-runtime-contract-test tests/test_framework_runtime_contract.c)
    target_link_libraries(umicom-studio-framework-runtime-contract-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-framework-runtime-contract-test)
    umicom_apply_sanitizers(umicom-studio-framework-runtime-contract-test)
    add_test(NAME studio.framework_runtime_contract COMMAND umicom-studio-framework-runtime-contract-test)

    add_executable(umicom-studio-bootstrap-inventory-test tests/test_bootstrap_inventory.c)
    target_link_libraries(umicom-studio-bootstrap-inventory-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-bootstrap-inventory-test)
    umicom_apply_sanitizers(umicom-studio-bootstrap-inventory-test)
    add_test(NAME studio.bootstrap_inventory COMMAND umicom-studio-bootstrap-inventory-test)

    add_executable(umicom-studio-composition-lifecycle-test tests/test_composition_lifecycle.c)
    target_link_libraries(umicom-studio-composition-lifecycle-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-composition-lifecycle-test)
    umicom_apply_sanitizers(umicom-studio-composition-lifecycle-test)
    add_test(NAME studio.composition_lifecycle COMMAND umicom-studio-composition-lifecycle-test)

    add_executable(umicom-studio-registry-extension-compatibility-test tests/test_registry_extension_compatibility.c)
    target_link_libraries(umicom-studio-registry-extension-compatibility-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-registry-extension-compatibility-test)
    umicom_apply_sanitizers(umicom-studio-registry-extension-compatibility-test)
    add_test(NAME studio.registry_extension_compatibility COMMAND umicom-studio-registry-extension-compatibility-test)

    add_executable(umicom-studio-state-inventory-consistency-test tests/test_state_inventory_consistency.c)
    target_link_libraries(umicom-studio-state-inventory-consistency-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-state-inventory-consistency-test)
    umicom_apply_sanitizers(umicom-studio-state-inventory-consistency-test)
    add_test(NAME studio.state_inventory_consistency COMMAND umicom-studio-state-inventory-consistency-test)

    add_executable(umicom-studio-designer-command-contribution-test tests/test_designer_command_contribution.c)
    target_link_libraries(umicom-studio-designer-command-contribution-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-designer-command-contribution-test)
    umicom_apply_sanitizers(umicom-studio-designer-command-contribution-test)
    add_test(NAME studio.designer_command_contribution COMMAND umicom-studio-designer-command-contribution-test)

    add_executable(umicom-studio-editor-command-contribution-test tests/test_editor_command_contribution.c)
    target_link_libraries(umicom-studio-editor-command-contribution-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-editor-command-contribution-test)
    umicom_apply_sanitizers(umicom-studio-editor-command-contribution-test)
    add_test(NAME studio.editor_command_contribution COMMAND umicom-studio-editor-command-contribution-test)

    add_executable(umicom-studio-debug-selection-test tests/test_debug_selection.c)
    target_link_libraries(umicom-studio-debug-selection-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-debug-selection-test)
    umicom_apply_sanitizers(umicom-studio-debug-selection-test)
    add_test(NAME studio.debug_selection COMMAND umicom-studio-debug-selection-test)

    add_executable(umicom-studio-source-navigation-contribution-test tests/test_source_navigation_contribution.c)
    target_link_libraries(umicom-studio-source-navigation-contribution-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-source-navigation-contribution-test)
    umicom_apply_sanitizers(umicom-studio-source-navigation-contribution-test)
    add_test(NAME studio.source_navigation_contribution COMMAND umicom-studio-source-navigation-contribution-test)

    add_executable(umicom-studio-navigation-view-contribution-test tests/test_navigation_view_contribution.c)
    target_link_libraries(umicom-studio-navigation-view-contribution-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-navigation-view-contribution-test)
    umicom_apply_sanitizers(umicom-studio-navigation-view-contribution-test)
    add_test(NAME studio.navigation_view_contribution COMMAND umicom-studio-navigation-view-contribution-test)

    add_executable(umicom-studio-editor-intelligence-contribution-test tests/test_editor_intelligence_contribution.c)
    target_link_libraries(umicom-studio-editor-intelligence-contribution-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-editor-intelligence-contribution-test)
    umicom_apply_sanitizers(umicom-studio-editor-intelligence-contribution-test)
    add_test(NAME studio.editor_intelligence_contribution COMMAND umicom-studio-editor-intelligence-contribution-test)

    add_executable(umicom-studio-editor-intelligence-workbench-contribution-test
        tests/test_editor_intelligence_workbench_contribution.c)
    target_link_libraries(umicom-studio-editor-intelligence-workbench-contribution-test
        PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-editor-intelligence-workbench-contribution-test)
    umicom_apply_sanitizers(umicom-studio-editor-intelligence-workbench-contribution-test)
    add_test(NAME studio.editor_intelligence_workbench_contribution
        COMMAND umicom-studio-editor-intelligence-workbench-contribution-test)
    # Use the shared build helper when it is available from the parent composition.
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(
            umicom-studio-editor-intelligence-workbench-contribution-test)
    endif()

    add_executable(umicom-studio-vcs-workbench-contribution-test
        tests/test_vcs_workbench_contribution.c)
    target_link_libraries(umicom-studio-vcs-workbench-contribution-test
        PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-vcs-workbench-contribution-test)
    umicom_apply_sanitizers(umicom-studio-vcs-workbench-contribution-test)
    add_test(NAME studio.vcs_workbench_contribution
        COMMAND umicom-studio-vcs-workbench-contribution-test)
    # Use the shared build helper when it is available from the parent composition.
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(
            umicom-studio-vcs-workbench-contribution-test)
    endif()

    add_executable(umicom-studio-data-workbench-contribution-test
        tests/test_data_workbench_contribution.c)
    target_link_libraries(umicom-studio-data-workbench-contribution-test
        PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-data-workbench-contribution-test)
    umicom_apply_sanitizers(umicom-studio-data-workbench-contribution-test)
    add_test(NAME studio.data_workbench_contribution
        COMMAND umicom-studio-data-workbench-contribution-test)
    # Use the shared build helper when it is available from the parent composition.
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(
            umicom-studio-data-workbench-contribution-test)
    endif()

    add_executable(umicom-studio-web-api-workbench-contribution-test
        tests/test_web_api_workbench_contribution.c)
    target_link_libraries(umicom-studio-web-api-workbench-contribution-test
        PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-web-api-workbench-contribution-test)
    umicom_apply_sanitizers(umicom-studio-web-api-workbench-contribution-test)
    add_test(NAME studio.web_api_workbench_contribution
        COMMAND umicom-studio-web-api-workbench-contribution-test)
    # Use the shared build helper when it is available from the parent composition.
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(
            umicom-studio-web-api-workbench-contribution-test)
    endif()

    add_executable(umicom-studio-test-readiness-contribution-test
        tests/test_test_readiness_contribution.c)
    target_link_libraries(umicom-studio-test-readiness-contribution-test
        PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-test-readiness-contribution-test)
    umicom_apply_sanitizers(umicom-studio-test-readiness-contribution-test)
    add_test(NAME studio.test_readiness_contribution
        COMMAND umicom-studio-test-readiness-contribution-test)
    # Use the shared build helper when it is available from the parent composition.
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(
            umicom-studio-test-readiness-contribution-test)
    endif()

    add_executable(umicom-studio-developer-ecosystem-contribution-test
        tests/test_developer_ecosystem_contribution.c)
    target_link_libraries(umicom-studio-developer-ecosystem-contribution-test
        PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-developer-ecosystem-contribution-test)
    umicom_apply_sanitizers(umicom-studio-developer-ecosystem-contribution-test)
    add_test(NAME studio.developer_ecosystem_contribution
        COMMAND umicom-studio-developer-ecosystem-contribution-test)
    # Use the shared build helper when it is available from the parent composition.
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(
            umicom-studio-developer-ecosystem-contribution-test)
    endif()

    add_executable(umicom-studio-editor-assistance-contribution-test tests/test_editor_assistance_contribution.c)
    target_link_libraries(umicom-studio-editor-assistance-contribution-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-editor-assistance-contribution-test)
    umicom_apply_sanitizers(umicom-studio-editor-assistance-contribution-test)
    add_test(NAME studio.editor_assistance_contribution COMMAND umicom-studio-editor-assistance-contribution-test)

    add_executable(umicom-studio-editor-productivity-contribution-test tests/test_editor_productivity_contribution.c)
    target_link_libraries(umicom-studio-editor-productivity-contribution-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-editor-productivity-contribution-test)
    umicom_apply_sanitizers(umicom-studio-editor-productivity-contribution-test)
    add_test(NAME studio.editor_productivity_contribution COMMAND umicom-studio-editor-productivity-contribution-test)

    add_executable(umicom-studio-editor-refactoring-contribution-test tests/test_editor_refactoring_contribution.c)
    target_link_libraries(umicom-studio-editor-refactoring-contribution-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-editor-refactoring-contribution-test)
    umicom_apply_sanitizers(umicom-studio-editor-refactoring-contribution-test)
    add_test(NAME studio.editor_refactoring_contribution COMMAND umicom-studio-editor-refactoring-contribution-test)

    add_executable(umicom-studio-debug-orchestration-contribution-test tests/test_debug_orchestration_contribution.c)
    target_link_libraries(umicom-studio-debug-orchestration-contribution-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-debug-orchestration-contribution-test)
    umicom_apply_sanitizers(umicom-studio-debug-orchestration-contribution-test)
    add_test(NAME studio.debug_orchestration_contribution COMMAND umicom-studio-debug-orchestration-contribution-test)

    add_executable(umicom-studio-platform-contract-api-bounds-test tests/test_platform_contract_api_bounds.c)
    target_link_libraries(umicom-studio-platform-contract-api-bounds-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-platform-contract-api-bounds-test)
    umicom_apply_sanitizers(umicom-studio-platform-contract-api-bounds-test)
    add_test(NAME studio.platform_contract_api_bounds COMMAND umicom-studio-platform-contract-api-bounds-test)

    add_executable(umicom-studio-service-namespace-inventory-test tests/test_service_namespace_inventory.c)
    target_link_libraries(umicom-studio-service-namespace-inventory-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-service-namespace-inventory-test)
    umicom_apply_sanitizers(umicom-studio-service-namespace-inventory-test)
    add_test(NAME studio.service_namespace_inventory COMMAND umicom-studio-service-namespace-inventory-test)

    add_executable(umicom-studio-workspace-test
        tests/test_workspace.c
    )
    target_link_libraries(umicom-studio-workspace-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-workspace-test)
    umicom_apply_sanitizers(umicom-studio-workspace-test)
    add_test(NAME studio.workspace COMMAND umicom-studio-workspace-test)

    add_executable(umicom-studio-files-test
        tests/test_files.c
    )
    target_link_libraries(umicom-studio-files-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-files-test)
    umicom_apply_sanitizers(umicom-studio-files-test)
    add_test(NAME studio.files COMMAND umicom-studio-files-test)

    add_executable(umicom-studio-search-test
        tests/test_search.c
    )
    target_link_libraries(umicom-studio-search-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-search-test)
    umicom_apply_sanitizers(umicom-studio-search-test)
    add_test(NAME studio.search COMMAND umicom-studio-search-test)

    add_executable(umicom-studio-processes-test
        tests/test_processes.c
    )
    target_link_libraries(umicom-studio-processes-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-processes-test)
    umicom_apply_sanitizers(umicom-studio-processes-test)
    add_test(NAME studio.processes COMMAND umicom-studio-processes-test)

    add_executable(umicom-studio-watcher-test
        tests/test_watcher.c
    )
    target_link_libraries(umicom-studio-watcher-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-watcher-test)
    umicom_apply_sanitizers(umicom-studio-watcher-test)
    add_test(NAME studio.watcher COMMAND umicom-studio-watcher-test)

    add_executable(umicom-studio-data-test tests/test_data.c)
    target_link_libraries(umicom-studio-data-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-data-test)
    add_test(NAME studio.data COMMAND umicom-studio-data-test)

    # Visit each bounded item once so every record receives the same rule.
    foreach(UMICOM_STUDIO_DATABASE_TEST
            database_explorer_centre
            database_explorer_connections
            database_explorer_queries
            database_explorer_migrations
            database_explorer_transfer
            database_explorer_views
            database_explorer_commands)
        string(REPLACE "_" "-" UMICOM_STUDIO_DATABASE_TEST_TARGET
            "${UMICOM_STUDIO_DATABASE_TEST}")
        add_executable(umicom-studio-${UMICOM_STUDIO_DATABASE_TEST_TARGET}-test
            tests/test_${UMICOM_STUDIO_DATABASE_TEST}.c)
        target_link_libraries(umicom-studio-${UMICOM_STUDIO_DATABASE_TEST_TARGET}-test
            PRIVATE Umicom::StudioCore)
        umicom_apply_warnings(umicom-studio-${UMICOM_STUDIO_DATABASE_TEST_TARGET}-test)
        umicom_apply_sanitizers(umicom-studio-${UMICOM_STUDIO_DATABASE_TEST_TARGET}-test)
        add_test(NAME studio.${UMICOM_STUDIO_DATABASE_TEST}
            COMMAND umicom-studio-${UMICOM_STUDIO_DATABASE_TEST_TARGET}-test)
    endforeach()

    add_executable(umicom-studio-messages-test tests/test_messages.c)
    target_link_libraries(umicom-studio-messages-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-messages-test)
    add_test(NAME studio.messages COMMAND umicom-studio-messages-test)

    add_executable(umicom-studio-history-test tests/test_history.c)
    target_link_libraries(umicom-studio-history-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-history-test)
    add_test(NAME studio.history COMMAND umicom-studio-history-test)

    add_executable(umicom-studio-replay-test tests/test_replay.c)
    target_link_libraries(umicom-studio-replay-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-replay-test)
    add_test(NAME studio.replay COMMAND umicom-studio-replay-test)

    add_executable(umicom-studio-fabric-test tests/test_fabric.c)
    target_link_libraries(umicom-studio-fabric-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-fabric-test)
    add_test(NAME studio.fabric COMMAND umicom-studio-fabric-test)

    add_executable(umicom-studio-outbox-test tests/test_outbox.c)
    target_link_libraries(umicom-studio-outbox-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-outbox-test)
    add_test(NAME studio.outbox COMMAND umicom-studio-outbox-test)

    add_executable(umicom-studio-persistence-test tests/test_persistence.c)
    target_link_libraries(umicom-studio-persistence-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-persistence-test)
    add_test(NAME studio.persistence COMMAND umicom-studio-persistence-test)

    add_executable(umicom-studio-dead-letters-test tests/test_dead_letters.c)
    target_link_libraries(umicom-studio-dead-letters-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-dead-letters-test)
    add_test(NAME studio.dead_letters COMMAND umicom-studio-dead-letters-test)

    # Visit each bounded item once so every record receives the same rule.
    foreach(UMICOM_STUDIO_OPERATION_TEST
            operations
            security
            plugins
            extension_platform
            product_centre
            observability
            resilience)
        add_executable(umicom-studio-${UMICOM_STUDIO_OPERATION_TEST}-test
            tests/test_${UMICOM_STUDIO_OPERATION_TEST}.c)
        target_link_libraries(umicom-studio-${UMICOM_STUDIO_OPERATION_TEST}-test
            PRIVATE Umicom::StudioCore)
        umicom_apply_warnings(umicom-studio-${UMICOM_STUDIO_OPERATION_TEST}-test)
        umicom_apply_sanitizers(umicom-studio-${UMICOM_STUDIO_OPERATION_TEST}-test)
        add_test(NAME studio.${UMICOM_STUDIO_OPERATION_TEST}
            COMMAND umicom-studio-${UMICOM_STUDIO_OPERATION_TEST}-test)
    endforeach()

    # Visit each bounded item once so every record receives the same rule.
    foreach(UMICOM_STUDIO_UI_TEST
            ui
            learning_centre
            workbench
            perspectives
            contributions
            view_models
            ui_catalogue
            ui_headless
            ui_services
            ui_session
            ui_actions
            application_authority
            desktop_shell
            federated_content
            federated_interactions
            federated_workspace_session
            extension_centre
            product_centre_views)
        add_executable(umicom-studio-${UMICOM_STUDIO_UI_TEST}-test
            tests/test_${UMICOM_STUDIO_UI_TEST}.c)
        target_link_libraries(umicom-studio-${UMICOM_STUDIO_UI_TEST}-test
            PRIVATE Umicom::StudioCore)
        umicom_apply_warnings(umicom-studio-${UMICOM_STUDIO_UI_TEST}-test)
        umicom_apply_sanitizers(umicom-studio-${UMICOM_STUDIO_UI_TEST}-test)
        add_test(NAME studio.${UMICOM_STUDIO_UI_TEST}
            COMMAND umicom-studio-${UMICOM_STUDIO_UI_TEST}-test)
    endforeach()

    # Visit each bounded item once so every record receives the same rule.
    foreach(UMICOM_STUDIO_DEVELOPER_TEST
            build_service
            build_workbench
            build_workspace_views
            diagnostics_pipeline
            test_service
            test_explorer_state
            test_explorer_operations
            test_explorer_hierarchy
            test_workspace_views
            test_execution_bridge
            terminal_service
            terminal_workbench
            language_service
            debugger_service
            debug_workspace_views
            source_control_service
            source_control_workspace
            trading_service
            trading_workspace_views
            developer_platform
            universal_development)
        string(REPLACE "_" "-" UMICOM_STUDIO_DEVELOPER_TEST_TARGET
            "${UMICOM_STUDIO_DEVELOPER_TEST}")
        add_executable(umicom-studio-${UMICOM_STUDIO_DEVELOPER_TEST_TARGET}-test
            tests/test_${UMICOM_STUDIO_DEVELOPER_TEST}.c)
        target_link_libraries(umicom-studio-${UMICOM_STUDIO_DEVELOPER_TEST_TARGET}-test
            PRIVATE Umicom::StudioCore)
        umicom_apply_warnings(umicom-studio-${UMICOM_STUDIO_DEVELOPER_TEST_TARGET}-test)
        umicom_apply_sanitizers(umicom-studio-${UMICOM_STUDIO_DEVELOPER_TEST_TARGET}-test)
        add_test(NAME studio.${UMICOM_STUDIO_DEVELOPER_TEST}
            COMMAND umicom-studio-${UMICOM_STUDIO_DEVELOPER_TEST_TARGET}-test)
    endforeach()

    # Visit each bounded item once so every record receives the same rule.
    foreach(UMICOM_STUDIO_DESIGNER_TEST
            declarative
            designer
            designer_workspace_model
            designer_surface_interaction
            designer_history
            designer_catalogue
            designer_preview
            new_project
            designer_state
            designer_project
            visual_builder_centre
            visual_builder_tree
            visual_builder_properties
            visual_builder_bindings
            visual_builder_interactions
            visual_builder_drag_drop
            visual_builder_clipboard
            visual_builder_templates
            visual_builder_preview_source
            visual_builder_settings_views)
        string(REPLACE "_" "-" UMICOM_STUDIO_DESIGNER_TEST_TARGET
            "${UMICOM_STUDIO_DESIGNER_TEST}")
        add_executable(umicom-studio-${UMICOM_STUDIO_DESIGNER_TEST_TARGET}-test
            tests/test_${UMICOM_STUDIO_DESIGNER_TEST}.c)
        target_link_libraries(umicom-studio-${UMICOM_STUDIO_DESIGNER_TEST_TARGET}-test
            PRIVATE Umicom::StudioCore)
        umicom_apply_warnings(umicom-studio-${UMICOM_STUDIO_DESIGNER_TEST_TARGET}-test)
        umicom_apply_sanitizers(umicom-studio-${UMICOM_STUDIO_DESIGNER_TEST_TARGET}-test)
        add_test(NAME studio.${UMICOM_STUDIO_DESIGNER_TEST}
            COMMAND umicom-studio-${UMICOM_STUDIO_DESIGNER_TEST_TARGET}-test)
    endforeach()


    # Visit each bounded item once so every record receives the same rule.
    foreach(UMICOM_STUDIO_WEB_TEST
            web_platform
            web
            browser
            charting
            charting_indicator
            frontend
            frontend_manifest
            web_metrics
            web_not_found
            web_service_accessor
            frontend_studio_centre
            frontend_studio_settings
            frontend_studio_preview
            frontend_studio_inspector
            frontend_studio_views
            frontend_studio_commands
            frontend_studio_quality)
        string(REPLACE "_" "-" UMICOM_STUDIO_WEB_TEST_TARGET
            "${UMICOM_STUDIO_WEB_TEST}")
        add_executable(umicom-studio-${UMICOM_STUDIO_WEB_TEST_TARGET}-test
            tests/test_${UMICOM_STUDIO_WEB_TEST}.c)
        target_link_libraries(umicom-studio-${UMICOM_STUDIO_WEB_TEST_TARGET}-test
            PRIVATE Umicom::StudioCore)
        umicom_apply_warnings(umicom-studio-${UMICOM_STUDIO_WEB_TEST_TARGET}-test)
        umicom_apply_sanitizers(umicom-studio-${UMICOM_STUDIO_WEB_TEST_TARGET}-test)
        add_test(NAME studio.${UMICOM_STUDIO_WEB_TEST}
            COMMAND umicom-studio-${UMICOM_STUDIO_WEB_TEST_TARGET}-test)
    endforeach()


    # Visit each bounded item once so every record receives the same rule.
    foreach(UMICOM_STUDIO_AI_TEST
            ai_platform
            ai_chat
            ai_model_comparison
            ai_retrieval
            ai_tools
            authorengine
            authorengine_integration
            authorengine_views
            ai_workspace
            coding_assistant
            vibe_coding
            ai_coding_views
            knowledge_centre
            knowledge_settings
            knowledge_views
            knowledge_archive
            knowledge_boundary
            helix
            helix_evidence
            helix_fitness
            helix_generation
            helix_agents
            helix_agent_centre
            helix_agent_commands
            helix_agent_adapters
            helix_agent_views
            helix_agent_candidates
            helix_agent_journal
            helix_agent_settings)
        string(REPLACE "_" "-" UMICOM_STUDIO_AI_TEST_TARGET
            "${UMICOM_STUDIO_AI_TEST}")
        add_executable(umicom-studio-${UMICOM_STUDIO_AI_TEST_TARGET}-test
            tests/test_${UMICOM_STUDIO_AI_TEST}.c)
        target_link_libraries(umicom-studio-${UMICOM_STUDIO_AI_TEST_TARGET}-test
            PRIVATE Umicom::StudioCore)
        umicom_apply_warnings(umicom-studio-${UMICOM_STUDIO_AI_TEST_TARGET}-test)
        umicom_apply_sanitizers(umicom-studio-${UMICOM_STUDIO_AI_TEST_TARGET}-test)
        add_test(NAME studio.${UMICOM_STUDIO_AI_TEST}
            COMMAND umicom-studio-${UMICOM_STUDIO_AI_TEST_TARGET}-test)
    endforeach()

    # Studio integration tests share the default .umicom session/recovery
    # directory.  A named CTest resource lock preserves parallel Framework
    # testing while preventing Studio tests from deleting one another's state.
    set_tests_properties(
        studio.framework_bootstrap
        studio.version
        studio.services
        studio.doctor
        studio.diagnostics
        studio.platform
        studio.settings
        studio.documents
        studio.document_workbench
        studio.tasks
        studio.commands
        studio.state
        studio.workspace
        studio.files
        studio.search
        studio.processes
        studio.watcher
        studio.data
        studio.messages
        studio.history
        studio.replay
        studio.fabric
        studio.outbox
        studio.persistence
        studio.dead_letters
        studio.operations
        studio.security
        studio.plugins
        studio.extension_platform
        studio.product_centre
        studio.observability
        studio.resilience
        studio.ui
        studio.learning_centre
        studio.workbench
        studio.perspectives
        studio.contributions
        studio.view_models
        studio.ui_catalogue
        studio.ui_headless
        studio.ui_services
        studio.ui_session
        studio.ui_actions
        studio.application_authority
        studio.desktop_shell
        studio.federated_content
        studio.federated_interactions
        studio.federated_workspace_session
        studio.extension_centre
        studio.product_centre_views
        studio.trading_service
        studio.trading_workspace_views
        studio.build_service
        studio.build_workbench
        studio.build_workspace_views
        studio.test_service
        studio.test_workspace_views
        studio.terminal_service
        studio.language_service
        studio.debugger_service
        studio.debug_workspace_views
        studio.source_control_service
        studio.developer_platform
        studio.ai_platform
        studio.ai_chat
        studio.ai_model_comparison
        studio.ai_retrieval
        studio.ai_tools
        studio.authorengine
        studio.authorengine_integration
        studio.authorengine_views
        studio.ai_workspace
        studio.coding_assistant
        studio.vibe_coding
        studio.ai_coding_views
        studio.knowledge_centre
        studio.knowledge_settings
        studio.knowledge_views
        studio.knowledge_archive
        studio.knowledge_boundary
        studio.helix
        studio.helix_evidence
        studio.helix_fitness
        studio.helix_generation
        studio.helix_agents
        studio.helix_agent_centre
        studio.helix_agent_commands
        studio.helix_agent_adapters
        studio.helix_agent_views
        studio.helix_agent_candidates
        studio.helix_agent_journal
        studio.helix_agent_settings
        PROPERTIES RESOURCE_LOCK umicom_studio_runtime_state
    )

    # Application delivery and release-engineering integration tests.
    add_executable(umicom-studio-delivery-platform-test tests/test_delivery_platform.c)
    target_link_libraries(umicom-studio-delivery-platform-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-delivery-platform-test)
    umicom_apply_sanitizers(umicom-studio-delivery-platform-test)
    add_test(NAME studio.delivery_platform COMMAND umicom-studio-delivery-platform-test)

    add_executable(umicom-studio-delivery-release-test tests/test_delivery_release.c)
    target_link_libraries(umicom-studio-delivery-release-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-delivery-release-test)
    umicom_apply_sanitizers(umicom-studio-delivery-release-test)
    add_test(NAME studio.delivery_release COMMAND umicom-studio-delivery-release-test)

    add_executable(umicom-studio-delivery-package-test tests/test_delivery_package.c)
    target_link_libraries(umicom-studio-delivery-package-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-delivery-package-test)
    umicom_apply_sanitizers(umicom-studio-delivery-package-test)
    add_test(NAME studio.delivery_package COMMAND umicom-studio-delivery-package-test)

    add_executable(umicom-studio-delivery-update-test tests/test_delivery_update.c)
    target_link_libraries(umicom-studio-delivery-update-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-delivery-update-test)
    umicom_apply_sanitizers(umicom-studio-delivery-update-test)
    add_test(NAME studio.delivery_update COMMAND umicom-studio-delivery-update-test)

    add_executable(umicom-studio-delivery-report-test tests/test_delivery_report.c)
    target_link_libraries(umicom-studio-delivery-report-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-delivery-report-test)
    umicom_apply_sanitizers(umicom-studio-delivery-report-test)
    add_test(NAME studio.delivery_report COMMAND umicom-studio-delivery-report-test)

    add_executable(umicom-studio-delivery-distribution-test tests/test_delivery_distribution.c)
    target_link_libraries(umicom-studio-delivery-distribution-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-delivery-distribution-test)
    umicom_apply_sanitizers(umicom-studio-delivery-distribution-test)
    add_test(NAME studio.delivery_distribution COMMAND umicom-studio-delivery-distribution-test)

    add_executable(umicom-studio-delivery-evidence-test tests/test_delivery_evidence.c)
    target_link_libraries(umicom-studio-delivery-evidence-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-delivery-evidence-test)
    umicom_apply_sanitizers(umicom-studio-delivery-evidence-test)
    add_test(NAME studio.delivery_evidence COMMAND umicom-studio-delivery-evidence-test)

    add_executable(umicom-studio-delivery-services-test tests/test_delivery_services.c)
    target_link_libraries(umicom-studio-delivery-services-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-delivery-services-test)
    umicom_apply_sanitizers(umicom-studio-delivery-services-test)
    add_test(NAME studio.delivery_services COMMAND umicom-studio-delivery-services-test)


    # ABI, SDK and reference-product conformance tests.
    add_executable(umicom-studio-compatibility-platform-test tests/test_compatibility_platform.c)
    target_link_libraries(umicom-studio-compatibility-platform-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-compatibility-platform-test)
    add_test(NAME studio.compatibility_platform COMMAND umicom-studio-compatibility-platform-test)

    add_executable(umicom-studio-sdk-platform-test tests/test_sdk_platform.c)
    target_link_libraries(umicom-studio-sdk-platform-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-sdk-platform-test)
    add_test(NAME studio.sdk_platform COMMAND umicom-studio-sdk-platform-test)

    add_executable(umicom-studio-products-platform-test tests/test_products_platform.c)
    target_link_libraries(umicom-studio-products-platform-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-products-platform-test)
    add_test(NAME studio.products_platform COMMAND umicom-studio-products-platform-test)

    add_executable(umicom-studio-conformance-platform-test tests/test_conformance_platform.c)
    target_link_libraries(umicom-studio-conformance-platform-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-conformance-platform-test)
    add_test(NAME studio.conformance_platform COMMAND umicom-studio-conformance-platform-test)

    add_executable(umicom-studio-architecture-guard-test tests/test_architecture_guard.c)
    target_link_libraries(umicom-studio-architecture-guard-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-architecture-guard-test)
    add_test(NAME studio.architecture_guard COMMAND umicom-studio-architecture-guard-test)

    add_executable(umicom-studio-reference-products-test tests/test_reference_products.c)
    target_link_libraries(umicom-studio-reference-products-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-reference-products-test)
    add_test(NAME studio.reference_products COMMAND umicom-studio-reference-products-test)

endif()

install(TARGETS
    umicom-studio-console
    umicom-studio-doctor
    umicom-studio-diagnostics
    umicom-studio-settings
    umicom-studio-platform
    umicom-studio-toolchain
    umicom-studio-state
    umicom-studio-workspace
    umicom-studio-process
    umicom-studio-data
    umicom-studio-database-explorer
    umicom-studio-integration-designer
    umicom-studio-quality-centre
    umicom-studio-messages
    umicom-studio-replay
    umicom-studio-history
    umicom-studio-security
    umicom-studio-security-centre
    umicom-studio-operations-centre
    umicom-studio-workspace-designer
    umicom-studio-compiler-centre
    umicom-studio-polyglot-compiler
    umicom-studio-distribution-centre
    umicom-studio-package-inspector
    umicom-studio-plugins
    umicom-studio-observability
    umicom-studio-resilience
    umicom-studio-operations
    umicom-studio-workbench
    umicom-studio-ui-catalogue
    umicom-studio-build
    umicom-studio-tests
    umicom-studio-terminal
    umicom-studio-language
    umicom-studio-debugger
    umicom-studio-source-control
    umicom-studio-developer
    umicom-studio-developer-pipeline
    umicom-studio-developer-state
    umicom-studio-visual-builder
    umicom-studio-ai
    umicom-studio-retrieval
    umicom-studio-authorengine
    umicom-studio-knowledge
    umicom-studio-helix
    umicom-studio-helix-evidence
    umicom-studio-helix-generation
    umicom-studio-delivery
    umicom-studio-package
    umicom-studio-release
    umicom-studio-update
    umicom-studio-distribution
    umicom-studio-evidence
    umicom-studio-rollback
    umicom-studio-pipeline
    umicom-studio-abi
    umicom-studio-sdk
    umicom-studio-products
    umicom-studio-compatibility
    umicom-studio-conformance
    umicom-studio-architecture
    RUNTIME DESTINATION bin
)

# Register verification targets only when the developer has enabled testing.
if(BUILD_TESTING)
add_executable(umicom-studio-codeguard-test tests/test_codeguard.c)
target_link_libraries(umicom-studio-codeguard-test PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-codeguard-test)
add_test(NAME studio.codeguard_platform COMMAND umicom-studio-codeguard-test)
add_executable(umicom-studio-architecture-quality-test tests/test_architecture_quality.c)
target_link_libraries(umicom-studio-architecture-quality-test PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-architecture-quality-test)
add_test(NAME studio.architecture_quality COMMAND umicom-studio-architecture-quality-test)
add_executable(umicom-studio-duplicate-review-test tests/test_duplicate_review.c)
target_link_libraries(umicom-studio-duplicate-review-test PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-duplicate-review-test)
add_test(NAME studio.duplicate_review COMMAND umicom-studio-duplicate-review-test)
add_executable(umicom-studio-quality-gate-test tests/test_quality_gate.c)
target_link_libraries(umicom-studio-quality-gate-test PRIVATE Umicom::StudioCore)
umicom_apply_warnings(umicom-studio-quality-gate-test)
add_test(NAME studio.quality_gate COMMAND umicom-studio-quality-gate-test)
# Visit each bounded item once so every record receives the same rule.
foreach(UMICOM_STUDIO_QUALITY_TEST
        quality_centre
        quality_evidence_navigation
        quality_remediation
        quality_duplicates
        quality_compatibility
        quality_gates
        quality_views
        quality_documentation
        quality_commands)
    string(REPLACE "_" "-" UMICOM_STUDIO_QUALITY_TEST_TARGET
        "${UMICOM_STUDIO_QUALITY_TEST}")
    add_executable(umicom-studio-${UMICOM_STUDIO_QUALITY_TEST_TARGET}-test
        tests/test_${UMICOM_STUDIO_QUALITY_TEST}.c)
    target_link_libraries(umicom-studio-${UMICOM_STUDIO_QUALITY_TEST_TARGET}-test
        PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-${UMICOM_STUDIO_QUALITY_TEST_TARGET}-test)
    umicom_apply_sanitizers(umicom-studio-${UMICOM_STUDIO_QUALITY_TEST_TARGET}-test)
    add_test(NAME studio.${UMICOM_STUDIO_QUALITY_TEST}
        COMMAND umicom-studio-${UMICOM_STUDIO_QUALITY_TEST_TARGET}-test)
endforeach()
endif()

include(
    "${CMAKE_CURRENT_SOURCE_DIR}/../../framework/cmake/UmicomApplicationProductisationContribution.cmake")
umicom_add_application_productisation_contribution(
    NAME "studio"
    TARGET "umicom_studio_productisation"
    SOURCE "src/productisation_contribution.c"
    TEST_SOURCE "tests/test_productisation_contribution.c")
target_link_libraries(umicom_studio_core PUBLIC
    umicom_studio_productisation)

# Register verification targets only when the developer has enabled testing.
if(BUILD_TESTING)
    # Visit each bounded item once so every record receives the same rule.
    foreach(UMICOM_STUDIO_PRODUCTION_TEST
            production_profile
            production_panels
            production_commands
            production_readiness)
        # Experience acceptance is registered separately because it consumes
        # executable journey evidence rather than the production profile.
        string(REPLACE "_" "-"
            UMICOM_STUDIO_PRODUCTION_TEST_SUFFIX
            "${UMICOM_STUDIO_PRODUCTION_TEST}")
        add_executable(
            "umicom-studio-${UMICOM_STUDIO_PRODUCTION_TEST_SUFFIX}-test"
            "tests/test_${UMICOM_STUDIO_PRODUCTION_TEST}.c")
        target_link_libraries(
            "umicom-studio-${UMICOM_STUDIO_PRODUCTION_TEST_SUFFIX}-test"
            PRIVATE Umicom::StudioCore)
        umicom_apply_warnings(
            "umicom-studio-${UMICOM_STUDIO_PRODUCTION_TEST_SUFFIX}-test")
        umicom_apply_sanitizers(
            "umicom-studio-${UMICOM_STUDIO_PRODUCTION_TEST_SUFFIX}-test")
        add_test(
            NAME "studio.${UMICOM_STUDIO_PRODUCTION_TEST}"
            COMMAND
                "umicom-studio-${UMICOM_STUDIO_PRODUCTION_TEST_SUFFIX}-test")
        # Use the shared build helper when it is available from the parent composition.
        if(COMMAND umicom_register_validation_target)
            umicom_register_validation_target(
                "umicom-studio-${UMICOM_STUDIO_PRODUCTION_TEST_SUFFIX}-test")
        endif()
    endforeach()

    add_executable(umicom-studio-experience-acceptance-test
        tests/test_experience_acceptance.c)
    target_link_libraries(umicom-studio-experience-acceptance-test
        PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-experience-acceptance-test)
    umicom_apply_sanitizers(umicom-studio-experience-acceptance-test)
    add_test(NAME studio.experience_acceptance
        COMMAND umicom-studio-experience-acceptance-test)
    # Use the shared build helper when it is available from the parent composition.
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(
            umicom-studio-experience-acceptance-test)
    endif()
endif()

# Register verification targets only when the developer has enabled testing.
if(BUILD_TESTING)
# Visit each bounded item once so every record receives the same rule.
foreach(UMICOM_STUDIO_SECURITY_TEST
        security_centre
        security_identities_roles
        security_secrets
        security_approvals
        security_packages
        security_audit_view
        security_views
        security_commands)
    string(REPLACE "_" "-" UMICOM_STUDIO_SECURITY_TEST_TARGET
        "${UMICOM_STUDIO_SECURITY_TEST}")
    add_executable(umicom-studio-${UMICOM_STUDIO_SECURITY_TEST_TARGET}-test
        tests/test_${UMICOM_STUDIO_SECURITY_TEST}.c)
    target_link_libraries(umicom-studio-${UMICOM_STUDIO_SECURITY_TEST_TARGET}-test
        PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-${UMICOM_STUDIO_SECURITY_TEST_TARGET}-test)
    umicom_apply_sanitizers(umicom-studio-${UMICOM_STUDIO_SECURITY_TEST_TARGET}-test)
    add_test(NAME studio.${UMICOM_STUDIO_SECURITY_TEST}
        COMMAND umicom-studio-${UMICOM_STUDIO_SECURITY_TEST_TARGET}-test)
endforeach()
endif()

# Register verification targets only when the developer has enabled testing.
if(BUILD_TESTING)
# Visit each bounded item once so every record receives the same rule.
foreach(UMICOM_STUDIO_WORKSPACE_TEST
        source_control_offline
        operations_centre
        operations_views_commands
        workspace_catalogue
        workspace_layouts
        workspace_profiles
        workspace_profile_session
        editor_layout_session
        workspace_groups
        workspace_themes
        appearance_centre
        appearance_session
        editor_theme_integration
        workspace_persistence
        workspace_views_commands)
    string(REPLACE "_" "-" UMICOM_STUDIO_WORKSPACE_TEST_TARGET
        "${UMICOM_STUDIO_WORKSPACE_TEST}")
    add_executable(umicom-studio-${UMICOM_STUDIO_WORKSPACE_TEST_TARGET}-test
        tests/test_${UMICOM_STUDIO_WORKSPACE_TEST}.c)
    target_include_directories(umicom-studio-${UMICOM_STUDIO_WORKSPACE_TEST_TARGET}-test
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    target_link_libraries(umicom-studio-${UMICOM_STUDIO_WORKSPACE_TEST_TARGET}-test
        PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-${UMICOM_STUDIO_WORKSPACE_TEST_TARGET}-test)
    umicom_apply_sanitizers(umicom-studio-${UMICOM_STUDIO_WORKSPACE_TEST_TARGET}-test)
    add_test(NAME studio.${UMICOM_STUDIO_WORKSPACE_TEST}
        COMMAND umicom-studio-${UMICOM_STUDIO_WORKSPACE_TEST_TARGET}-test)
endforeach()
    # Keep explicit discovery-off verification in the shared validation build.
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-source-control-offline-test)
    endif()
    set_tests_properties(studio.source_control_offline PROPERTIES
        LABELS "studio;source-control;offline;acceptance")
endif()

# Register verification targets only when the developer has enabled testing.
if(BUILD_TESTING)
# Visit each bounded item once so every record receives the same rule.
foreach(UMICOM_STUDIO_COMPILER_TEST
        compiler_catalogue
        compiler_views_commands
        compiler_templates
        compiler_settings_binding
        compiler_diagnostics
        compiler_interop
        umicc_polyglot
        compiler_centre)
    string(REPLACE "_" "-" UMICOM_STUDIO_COMPILER_TEST_TARGET
        "${UMICOM_STUDIO_COMPILER_TEST}")
    add_executable(umicom-studio-${UMICOM_STUDIO_COMPILER_TEST_TARGET}-test
        tests/test_${UMICOM_STUDIO_COMPILER_TEST}.c)
    target_link_libraries(umicom-studio-${UMICOM_STUDIO_COMPILER_TEST_TARGET}-test
        PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-${UMICOM_STUDIO_COMPILER_TEST_TARGET}-test)
    umicom_apply_sanitizers(umicom-studio-${UMICOM_STUDIO_COMPILER_TEST_TARGET}-test)
    add_test(NAME studio.${UMICOM_STUDIO_COMPILER_TEST}
        COMMAND umicom-studio-${UMICOM_STUDIO_COMPILER_TEST_TARGET}-test)
endforeach()
endif()

# Register verification targets only when the developer has enabled testing.
if(BUILD_TESTING)
# Visit each bounded item once so every record receives the same rule.
foreach(UMICOM_STUDIO_DISTRIBUTION_TEST
        sdk_centre
        package_project
        dependency_bundle
        installer_centre
        supply_chain
        release_upgrade
        distribution_centre
        distribution_views_commands)
    string(REPLACE "_" "-" UMICOM_STUDIO_DISTRIBUTION_TEST_TARGET
        "${UMICOM_STUDIO_DISTRIBUTION_TEST}")
    add_executable(umicom-studio-${UMICOM_STUDIO_DISTRIBUTION_TEST_TARGET}-test
        tests/test_${UMICOM_STUDIO_DISTRIBUTION_TEST}.c)
    target_link_libraries(umicom-studio-${UMICOM_STUDIO_DISTRIBUTION_TEST_TARGET}-test
        PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-${UMICOM_STUDIO_DISTRIBUTION_TEST_TARGET}-test)
    umicom_apply_sanitizers(umicom-studio-${UMICOM_STUDIO_DISTRIBUTION_TEST_TARGET}-test)
    add_test(NAME studio.${UMICOM_STUDIO_DISTRIBUTION_TEST}
        COMMAND umicom-studio-${UMICOM_STUDIO_DISTRIBUTION_TEST_TARGET}-test)
endforeach()
endif()

# Execute real CMake/CTest/toolchain work in a disposable, test-owned folder.
if(BUILD_TESTING)
    add_executable(umicom-studio-project-workflow-test tests/test_project_workflow.c)
    target_link_libraries(umicom-studio-project-workflow-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-project-workflow-test)
    umicom_apply_sanitizers(umicom-studio-project-workflow-test)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/project-workflow")
    add_test(NAME studio.project_workflow COMMAND umicom-studio-project-workflow-test)
    set_tests_properties(studio.project_workflow PROPERTIES TIMEOUT 180
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/project-workflow"
        LABELS "studio;ide-workflow;integration;toolchain")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/project-workflow-large-source")
    add_test(NAME studio.project_workflow.large_source
        COMMAND umicom-studio-project-workflow-test large-source)
    set_tests_properties(studio.project_workflow.large_source PROPERTIES TIMEOUT 180
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/project-workflow-large-source"
        LABELS "studio;document-content;ide-workflow;integration;toolchain")
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-project-workflow-test)
    endif()
endif()

# Older Studio tests create and remove the same relative .umicom directory.
# Keep their established working directory, but do not let CTest execute these
# owners concurrently. Tests with an explicit isolated directory stay parallel.
if(BUILD_TESTING)
    get_property(studioDeclaredTests DIRECTORY PROPERTY TESTS)
    foreach(studioTest IN LISTS studioDeclaredTests)
        get_property(studioWorkingDirectory TEST "${studioTest}" PROPERTY WORKING_DIRECTORY)
        if(NOT studioWorkingDirectory OR studioWorkingDirectory STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
            set_property(TEST "${studioTest}" APPEND PROPERTY RESOURCE_LOCK umicom_studio_shared_test_files)
        endif()
    endforeach()
endif()

# Persist and reopen reviewed project settings with the real service container.
if(BUILD_TESTING)
    add_executable(umicom-studio-project-settings-test tests/test_project_settings.c)
    target_link_libraries(umicom-studio-project-settings-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-project-settings-test)
    umicom_apply_sanitizers(umicom-studio-project-settings-test)
    foreach(settings_backend IN ITEMS memory durable)
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/project-settings-${settings_backend}")
        add_test(NAME "studio.project_settings.${settings_backend}"
            COMMAND umicom-studio-project-settings-test "${settings_backend}")
        set_tests_properties("studio.project_settings.${settings_backend}" PROPERTIES
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/project-settings-${settings_backend}"
            TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;operational-state;regression")
    endforeach()
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-project-settings-test)
    endif()
endif()

# Verify editing through Studio's real service composition, without a display.
if(BUILD_TESTING)
    add_executable(umicom-studio-editor-workflow-test tests/test_editor_workflow.c)
    target_link_libraries(umicom-studio-editor-workflow-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-editor-workflow-test)
    umicom_apply_sanitizers(umicom-studio-editor-workflow-test)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/editor-workflow")
    add_test(NAME studio.editor_workflow COMMAND umicom-studio-editor-workflow-test)
    set_tests_properties(studio.editor_workflow PROPERTIES TIMEOUT 60
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/editor-workflow"
        LABELS "studio;editor;integration")
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-editor-workflow-test)
    endif()
endif()

# Ship the same beginner editing guide used by source-tree readers.
install(FILES docs/EDITING_FILES.html
    DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom/studio/docs
    COMPONENT Studio)

# Reuse the real project workflow, adding compiler-to-source navigation and
# preserving a second unsaved draft while the broken project is corrected.
if(TARGET umicom-studio-project-workflow-test)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/project-diagnostics-workflow")
    add_test(NAME studio.project_workflow.diagnostics
        COMMAND umicom-studio-project-workflow-test diagnostics)
    set_tests_properties(studio.project_workflow.diagnostics PROPERTIES TIMEOUT 180
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/project-diagnostics-workflow")
endif()

install(FILES docs/READING_BUILD_ERRORS.html
    DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom/studio/docs)

# Exercise creation, editing, Save All and compilation using a real new header.
if(BUILD_TESTING)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/project-workflow-files")
    add_test(NAME studio.project_workflow.project_files
        COMMAND umicom-studio-project-workflow-test project-files)
    set_tests_properties(studio.project_workflow.project_files PROPERTIES TIMEOUT 180
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/project-workflow-files"
        LABELS "studio;project-files;integration;toolchain")
endif()

# Keep the ordinary, large-source, diagnostic and project-file journeys intact.
if(TARGET umicom-studio-project-workflow-test)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/project-workflow-refresh")
    add_test(NAME studio.project_workflow.background_refresh
        COMMAND umicom-studio-project-workflow-test background-refresh)
    set_tests_properties(studio.project_workflow.background_refresh PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/project-workflow-refresh"
        TIMEOUT 180 LABELS "studio;background-refresh;integration;toolchain")
endif()

# Exercise Find in Files through the real Studio facade, documents and compiler.
if(TARGET umicom-studio-project-workflow-test)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/project-workflow-search")
    add_test(NAME studio.project_workflow.find_in_files
        COMMAND umicom-studio-project-workflow-test find-in-files)
    set_tests_properties(studio.project_workflow.find_in_files PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/project-workflow-search"
        TIMEOUT 180 LABELS "studio;search;integration;toolchain")
endif()

# Install the public search walkthrough beside the existing editing guides.
install(FILES docs/FIND_IN_FILES.html
    DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom/studio/docs)

# Reload the intended document while another unsaved tab stays active, then
# exercise the same real configure/build/run/test/install pipeline.
if(TARGET umicom-studio-project-workflow-test)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/project-workflow-reload")
    add_test(NAME studio.project_workflow.reload
        COMMAND umicom-studio-project-workflow-test reload)
    set_tests_properties(studio.project_workflow.reload PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/project-workflow-reload"
        TIMEOUT 180 LABELS "studio;document;reload;integration;toolchain")
endif()

# Install the saved-file workflow alongside the existing editor lessons.
install(FILES docs/RELOADING_FILES.html
    DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom/studio/docs)

# Compare captured source versions, retain an unrelated draft, then use the
# existing real build/run/test/install workflow after explicit reload approval.
if(TARGET umicom-studio-project-workflow-test)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/project-workflow-comparison")
    add_test(NAME studio.project_workflow.compare_saved
        COMMAND umicom-studio-project-workflow-test compare-saved)
    set_tests_properties(studio.project_workflow.compare_saved PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/project-workflow-comparison"
        TIMEOUT 180 LABELS "studio;document;comparison;integration;toolchain")
endif()
install(FILES docs/COMPARING_SAVED_FILES.html
    DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom/studio/docs)


# Exercise the actual Studio document owner, then the generated project's
# existing save/configure/build/run/test/install and failed-build checks.
if(TARGET umicom-studio-project-workflow-test)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/project-workflow-editing")
    add_test(NAME studio.project_workflow.edit_commands
        COMMAND umicom-studio-project-workflow-test edit-commands)
    set_tests_properties(studio.project_workflow.edit_commands PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/project-workflow-editing"
        TIMEOUT 180 LABELS "studio;document;editing;integration;toolchain")
endif()

install(FILES docs/EDIT_COMMANDS.html
    DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom/studio/docs)

# Check the native editing boundary before publishing a Studio build.
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/StudioBuildReadiness.cmake")

# Install the public workflow lesson beside this product's other guides.
install(FILES docs/WORKFLOW_TOOLS.html
    DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom/studio/docs)

# Make the document-closing guide available in the installed application.
install(FILES docs/CLOSING_DOCUMENTS.html
    DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom/studio/docs)

# Exercise the production build-before-debug command with a controlled child
# protocol peer; this is not a replacement for GDB/LLDB native qualification.
if(BUILD_TESTING AND TARGET umicom-dap-protocol-fixture)
    add_executable(umicom-studio-development-workflow-test tests/test_development_workflow.c)
    target_link_libraries(umicom-studio-development-workflow-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-development-workflow-test)
    umicom_apply_sanitizers(umicom-studio-development-workflow-test)
    add_dependencies(umicom-studio-development-workflow-test umicom-dap-protocol-fixture)
    foreach(case complete compile-failure cancel-building profile-changed untrusted adapter-missing no-build-submitted)
        add_test(NAME studio.development_workflow.${case}
            COMMAND umicom-studio-development-workflow-test "$<TARGET_FILE:umicom-dap-protocol-fixture>" "${case}")
        set_tests_properties(studio.development_workflow.${case} PROPERTIES TIMEOUT 180 RUN_SERIAL TRUE
            LABELS "studio;build;debug;protocol-fixture;integration")
    endforeach()
endif()

if(TARGET umicom-studio-build-readiness AND TARGET umicom-studio-development-workflow-test)
    add_custom_target(umicom-studio-development-readiness)
    add_dependencies(umicom-studio-development-readiness umicom-studio-build-readiness
        umicom-studio-development-workflow-test umicom-development-debug-test
        umicom-development-debug-session-test umicom-development-delivery-contract-test
        umicom-development-workflow-example)
endif()

# Install the public project-development lesson beside the existing guides.
install(FILES docs/DEVELOPMENT_WORKFLOW.html
    DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom/studio/docs)

# Keep the beginner ordering workflow available with the installed product.
install(FILES docs/ORDERING_WORKSPACE_LAYOUTS.md
    DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom/studio/docs
    COMPONENT Studio)

# Keep the beginner report workflow with the installed documentation.
install(FILES docs/COPYING_TEST_EVIDENCE.md
    DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom/studio/docs)

# Keep the beginner test navigation lesson available in an installed product.
install(FILES docs/OPENING_TEST_FAILURES.md
    DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom/studio/docs)

# Ship the beginner debugger navigation workflow with Studio.
install(FILES docs/DEBUGGER_SOURCE_SELECTION.md
    DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)

# Property editing uses the real product service and the actual workbench.
if(BUILD_TESTING)
    add_executable(umicom-studio-breakpoint-properties-test tests/test_breakpoint_properties.c)
    target_link_libraries(umicom-studio-breakpoint-properties-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-breakpoint-properties-test)
    umicom_apply_sanitizers(umicom-studio-breakpoint-properties-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-breakpoint-properties-test)
    endif()
    foreach(case idle memory stale remove)
        add_test(NAME studio.breakpoint_properties.service.${case} COMMAND umicom-studio-breakpoint-properties-test ${case})
        set_tests_properties(studio.breakpoint_properties.service.${case} PROPERTIES
            TIMEOUT 30 LABELS "studio;debugger;breakpoint;regression")
    endforeach()
    if(TARGET umicom-studio-workspace-canvas-test)
        add_executable(umicom-studio-breakpoint-properties-native-test tests/test_breakpoint_properties_gtk4.c src/gui/workbench/workbench_window.c)
        target_include_directories(umicom-studio-breakpoint-properties-native-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
        target_link_libraries(umicom-studio-breakpoint-properties-native-test PRIVATE
            Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
        umicom_apply_warnings(umicom-studio-breakpoint-properties-native-test)
        umicom_apply_sanitizers(umicom-studio-breakpoint-properties-native-test)
        if(COMMAND umicom_register_validation_target)
            umicom_register_validation_target(umicom-studio-breakpoint-properties-native-test)
        endif()
        foreach(case apply disable remove stale reset draft-refresh long-text detached close)
            add_test(NAME studio.breakpoint_properties.gtk4.${case} COMMAND umicom-studio-breakpoint-properties-native-test ${case})
            set_tests_properties(studio.breakpoint_properties.gtk4.${case} PROPERTIES
                TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;gtk4;debugger;breakpoint;acceptance")
        endforeach()
    endif()
endif()
install(FILES docs/EDITING_BREAKPOINTS.md DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)


# The deterministic child speaks DAP but never debugs or launches a program.
if(BUILD_TESTING AND TARGET umicom-dap-protocol-fixture)
    add_executable(umicom-studio-breakpoint-properties-live-test tests/test_breakpoint_properties_live.c)
    target_link_libraries(umicom-studio-breakpoint-properties-live-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-breakpoint-properties-live-test)
    umicom_apply_sanitizers(umicom-studio-breakpoint-properties-live-test)
    add_dependencies(umicom-studio-breakpoint-properties-live-test umicom-dap-protocol-fixture)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-breakpoint-properties-live-test)
    endif()
    foreach(case success rejected unsupported)
        add_test(NAME studio.breakpoint_properties.live.${case} COMMAND umicom-studio-breakpoint-properties-live-test
            "$<TARGET_FILE:umicom-dap-protocol-fixture>" ${case})
        set_tests_properties(studio.breakpoint_properties.live.${case} PROPERTIES
            TIMEOUT 30 LABELS "studio;debugger;breakpoint;protocol-fixture;regression")
    endforeach()
endif()


# Property editing uses the real product service and the actual workbench.
if(BUILD_TESTING)
    add_executable(umicom-studio-watch-properties-test tests/test_watch_properties.c)
    target_link_libraries(umicom-studio-watch-properties-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-watch-properties-test)
    umicom_apply_sanitizers(umicom-studio-watch-properties-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-watch-properties-test)
    endif()
    foreach(case edit remove stale evaluate-idle evaluate-memory)
        add_test(NAME studio.watch_properties.service.${case} COMMAND umicom-studio-watch-properties-test ${case})
        set_tests_properties(studio.watch_properties.service.${case} PROPERTIES
            TIMEOUT 30 LABELS "studio;debugger;watch;regression")
    endforeach()
    if(TARGET umicom-studio-workspace-canvas-test)
        add_executable(umicom-studio-watch-properties-native-test tests/test_watch_properties_gtk4.c src/gui/workbench/workbench_window.c)
        target_include_directories(umicom-studio-watch-properties-native-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
        target_link_libraries(umicom-studio-watch-properties-native-test PRIVATE
            Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
        umicom_apply_warnings(umicom-studio-watch-properties-native-test)
        umicom_apply_sanitizers(umicom-studio-watch-properties-native-test)
        if(COMMAND umicom_register_validation_target)
            umicom_register_validation_target(umicom-studio-watch-properties-native-test)
        endif()
        foreach(case apply disable remove stale reset draft-refresh long-text empty detached close evaluate-draft evaluate-idle)
            add_test(NAME studio.watch_properties.gtk4.${case} COMMAND umicom-studio-watch-properties-native-test ${case})
            set_tests_properties(studio.watch_properties.gtk4.${case} PROPERTIES
                TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;gtk4;debugger;watch;acceptance")
        endforeach()
    endif()
endif()
install(FILES docs/EDITING_WATCHES.md DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)


# The deterministic child speaks DAP but never debugs or launches a program.
if(BUILD_TESTING AND TARGET umicom-dap-protocol-fixture)
    add_executable(umicom-studio-watch-properties-live-test tests/test_watch_properties_live.c)
    target_link_libraries(umicom-studio-watch-properties-live-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-watch-properties-live-test)
    umicom_apply_sanitizers(umicom-studio-watch-properties-live-test)
    add_dependencies(umicom-studio-watch-properties-live-test umicom-dap-protocol-fixture)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-watch-properties-live-test)
    endif()
    foreach(case success rejected)
        add_test(NAME studio.watch_properties.live.${case} COMMAND umicom-studio-watch-properties-live-test
            "$<TARGET_FILE:umicom-dap-protocol-fixture>" ${case})
        set_tests_properties(studio.watch_properties.live.${case} PROPERTIES
            TIMEOUT 30 LABELS "studio;debugger;watch;protocol-fixture;regression")
    endforeach()
endif()


# Variable inspection runs through the real service and native controls. The
# deterministic adapter never attaches to a user process or runs a compiler.
if(BUILD_TESTING AND TARGET umicom-dap-protocol-fixture)
    add_executable(umicom-studio-variable-inspection-test tests/test_variable_inspection.c)
    target_link_libraries(umicom-studio-variable-inspection-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-variable-inspection-test)
    umicom_apply_sanitizers(umicom-studio-variable-inspection-test)
    add_dependencies(umicom-studio-variable-inspection-test umicom-dap-protocol-fixture)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-variable-inspection-test)
    endif()
    foreach(case idle memory success rejected)
        add_test(NAME studio.variable_inspection.service.${case} COMMAND umicom-studio-variable-inspection-test
            "$<TARGET_FILE:umicom-dap-protocol-fixture>" ${case})
        set_tests_properties(studio.variable_inspection.service.${case} PROPERTIES
            TIMEOUT 30 LABELS "studio;debugger;variables;protocol-fixture;regression")
    endforeach()
    if(TARGET umicom-studio-workspace-canvas-test)
        add_executable(umicom-studio-variable-inspection-native-test tests/test_variable_inspection_gtk4.c src/gui/workbench/workbench_window.c)
        target_include_directories(umicom-studio-variable-inspection-native-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
        target_link_libraries(umicom-studio-variable-inspection-native-test PRIVATE
            Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
        umicom_apply_warnings(umicom-studio-variable-inspection-native-test)
        umicom_apply_sanitizers(umicom-studio-variable-inspection-native-test)
        add_dependencies(umicom-studio-variable-inspection-native-test umicom-dap-protocol-fixture)
        if(COMMAND umicom_register_validation_target)
            umicom_register_validation_target(umicom-studio-variable-inspection-native-test)
        endif()
        foreach(case display expand nested collapse refresh stale detached close empty cycle rejected failed-refresh budget)
            add_test(NAME studio.variable_inspection.gtk4.${case} COMMAND umicom-studio-variable-inspection-native-test
                "$<TARGET_FILE:umicom-dap-protocol-fixture>" ${case})
            set_tests_properties(studio.variable_inspection.gtk4.${case} PROPERTIES
                TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;gtk4;debugger;variables;acceptance")
        endforeach()
    endif()
endif()
install(FILES docs/INSPECTING_VARIABLES.md DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)

# Scope controls share the variable inspector, but expensive scopes must remain
# unloaded until an explicit gesture. Exercise the actual native workbench.
if(BUILD_TESTING AND TARGET umicom-dap-protocol-fixture)
    if(TARGET umicom-studio-workspace-canvas-test)
        add_executable(umicom-studio-scope-inspection-native-test tests/test_scope_inspection_gtk4.c src/gui/workbench/workbench_window.c)
        target_include_directories(umicom-studio-scope-inspection-native-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
        target_link_libraries(umicom-studio-scope-inspection-native-test PRIVATE
            Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
        umicom_apply_warnings(umicom-studio-scope-inspection-native-test)
        umicom_apply_sanitizers(umicom-studio-scope-inspection-native-test)
        add_dependencies(umicom-studio-scope-inspection-native-test umicom-dap-protocol-fixture)
        if(COMMAND umicom_register_validation_target)
            umicom_register_validation_target(umicom-studio-scope-inspection-native-test)
        endif()
        foreach(case display expand only-expensive same-name nested collapse refresh selection stale detached close empty cycle rejected failed-refresh budget)
            add_test(NAME studio.scope_inspection.gtk4.${case} COMMAND umicom-studio-scope-inspection-native-test
                "$<TARGET_FILE:umicom-dap-protocol-fixture>" ${case})
            set_tests_properties(studio.scope_inspection.gtk4.${case} PROPERTIES
                TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;gtk4;debugger;scopes;acceptance")
        endforeach()
    endif()
endif()
install(FILES docs/INSPECTING_SCOPES.md DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)

# The thin product must consume the real Framework session codec and preserve
# its active value when imported data is damaged or its review is stale.
if(BUILD_TESTING)
    add_executable(umicom-studio-session-transfer-test tests/test_session_transfer.c)
    target_link_libraries(umicom-studio-session-transfer-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-session-transfer-test)
    umicom_apply_sanitizers(umicom-studio-session-transfer-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-session-transfer-test)
    endif()
    add_test(NAME studio.workspace.session_transfer COMMAND umicom-studio-session-transfer-test)
    set_tests_properties(studio.workspace.session_transfer PROPERTIES
        TIMEOUT 60 LABELS "studio;workspace;value-archive;regression")
endif()

install(FILES docs/WORKSPACE_SESSION_EXCHANGE.html DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)

# Exercise the product identity boundary against the shared transaction owner.
if(BUILD_TESTING)
    add_executable(umicom_studio_core-context-review-test tests/test_context_review.c)
    target_link_libraries(umicom_studio_core-context-review-test PRIVATE umicom_studio_core)
    umicom_apply_warnings(umicom_studio_core-context-review-test)
    umicom_apply_sanitizers(umicom_studio_core-context-review-test)
    foreach(context_case publication stale foreign clear divergent_ui)
        add_test(NAME studio.context_review.${context_case}
            COMMAND umicom_studio_core-context-review-test ${context_case})
        set_tests_properties(studio.context_review.${context_case} PROPERTIES
            LABELS "application;context-review;studio")
    endforeach()
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom_studio_core-context-review-test)
    endif()
endif()

# Native acceptance uses the product toolbar and Framework-owned review window.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-open-replacements-test
        tests/test_open_replacements_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-open-replacements-test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-open-replacements-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-open-replacements-test)
    umicom_apply_sanitizers(umicom-studio-open-replacements-test)
    foreach(case apply-both skip-first stop-partial stale teardown pending-close owned-fields creation-teardown)
        add_test(NAME studio.open_replacements.gtk4.${case} COMMAND umicom-studio-open-replacements-test ${case})
        set_tests_properties(studio.open_replacements.gtk4.${case} PROPERTIES
            TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;gtk4;document;replacement;acceptance")
    endforeach()
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-open-replacements-test)
    endif()
endif()

install(FILES docs/REVIEW_OPEN_DOCUMENT_REPLACEMENTS.html DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)

# Exercise real toolbar actions with the same isolated startup used by editor acceptance.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-bookmarks-test tests/test_bookmarks_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-bookmarks-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-bookmarks-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-bookmarks-test)
    umicom_apply_sanitizers(umicom-studio-bookmarks-test)
    foreach(case toggle next previous cross-document retained)
        add_test(NAME studio.bookmarks.gtk4.${case} COMMAND umicom-studio-bookmarks-test ${case})
        set_tests_properties(studio.bookmarks.gtk4.${case} PROPERTIES
            TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;document;bookmarks;gtk4;acceptance")
    endforeach()
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-bookmarks-test)
    endif()
endif()
install(FILES docs/SOURCE_BOOKMARKS.html DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)

# Assignments require a deliberate native gesture and a fresh captured row.
# This fixture uses protocol samples; it never modifies a user's debuggee.
if(BUILD_TESTING AND TARGET umicom-dap-protocol-fixture AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-variable-assignment-native-test tests/test_variable_assignment_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-variable-assignment-native-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-variable-assignment-native-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-variable-assignment-native-test)
    umicom_apply_sanitizers(umicom-studio-variable-assignment-native-test)
    add_dependencies(umicom-studio-variable-assignment-native-test umicom-dap-protocol-fixture)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-variable-assignment-native-test)
    endif()
    foreach(case success nested draft stale detached close unsupported malformed)
        add_test(NAME studio.variable_assignment.gtk4.${case} COMMAND umicom-studio-variable-assignment-native-test
            "$<TARGET_FILE:umicom-dap-protocol-fixture>" ${case})
        set_tests_properties(studio.variable_assignment.gtk4.${case} PROPERTIES
            TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;gtk4;debugger;variables;acceptance")
    endforeach()
endif()
install(FILES docs/ASSIGNING_DEBUGGER_VALUES.html DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)

# Keep live transcript integration separate from completed build-history tests.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-live-output-native-test tests/test_live_output_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-live-output-native-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-live-output-native-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-live-output-native-test)
    umicom_apply_sanitizers(umicom-studio-live-output-native-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-live-output-native-test)
    endif()
    foreach(case surface history retained)
        add_test(NAME studio.live_build_output.gtk4.${case} COMMAND umicom-studio-live-output-native-test ${case})
        set_tests_properties(studio.live_build_output.gtk4.${case} PROPERTIES
            SKIP_RETURN_CODE 77 TIMEOUT 30 LABELS "studio;build;output;gtk4;regression")
    endforeach()
endif()
install(FILES docs/LIVE_BUILD_OUTPUT.html DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)

# Log selection has no execution side effects; the native fixture never submits a job.
if(BUILD_TESTING)
    add_executable(umicom-studio-build-log-test tests/test_build_log.c)
    target_link_libraries(umicom-studio-build-log-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-build-log-test)
    umicom_apply_sanitizers(umicom-studio-build-log-test)
    add_test(NAME studio.build_log.selection COMMAND umicom-studio-build-log-test)
    set_tests_properties(studio.build_log.selection PROPERTIES TIMEOUT 30 LABELS "studio;build;log;regression")
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-build-log-test)
    endif()
endif()
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-build-log-native-test tests/test_build_log_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-build-log-native-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-build-log-native-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-build-log-native-test)
    umicom_apply_sanitizers(umicom-studio-build-log-native-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-build-log-native-test)
    endif()
    foreach(case select invalid clear retained)
        add_test(NAME studio.build_log.gtk4.${case} COMMAND umicom-studio-build-log-native-test ${case})
        set_tests_properties(studio.build_log.gtk4.${case} PROPERTIES
            SKIP_RETURN_CODE 77 TIMEOUT 30 LABELS "studio;build;log;gtk4;regression")
    endforeach()
endif()
install(FILES docs/SAVING_BUILD_LOGS.html DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)


# Exercise the thin connection-settings entry with a substituted presenter;
# the fixture never opens the user's settings database or a provider session.
if(BUILD_TESTING AND TARGET Umicom::ui_gtk4)
    add_executable(umicom-studio-provider-connections-entry-test tests/test_provider_connections_entry_gtk4.c)
    target_link_libraries(umicom-studio-provider-connections-entry-test PRIVATE Umicom::ui_gtk4)
    umicom_apply_warnings(umicom-studio-provider-connections-entry-test)
    umicom_apply_sanitizers(umicom-studio-provider-connections-entry-test)
    add_test(NAME studio.provider_connections.native_entry COMMAND umicom-studio-provider-connections-entry-test)
    set_tests_properties(studio.provider_connections.native_entry PROPERTIES TIMEOUT 30 SKIP_RETURN_CODE 77
        LABELS "studio;provider-connections;gtk4;ownership;regression")
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-provider-connections-entry-test)
    endif()
endif()

# Keep native selected-code capture on the same lifetime boundary as editor actions.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-selected-code-chat-test tests/test_selected_code_chat_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-selected-code-chat-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_compile_definitions(umicom-studio-selected-code-chat-test PRIVATE UmiProviderConnectionsGtkPresentSelection=FixturePresentSelection)
    target_link_libraries(umicom-studio-selected-code-chat-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-selected-code-chat-test)
    umicom_apply_sanitizers(umicom-studio-selected-code-chat-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-selected-code-chat-test)
    endif()
    foreach(case selection empty large-selection new-selection retained close-success close-failure)
        add_test(NAME studio.provider_chat.selection.gtk4.${case} COMMAND umicom-studio-selected-code-chat-test ${case})
        set_tests_properties(studio.provider_chat.selection.gtk4.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;provider-chat;document;gtk4;privacy;regression")
    endforeach()
endif()

# Exercise Studio's real action and Framework review window as one editing workflow.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-selected-code-proposal-test tests/test_selected_code_proposal_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-selected-code-proposal-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-selected-code-proposal-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-selected-code-proposal-test)
    umicom_apply_sanitizers(umicom-studio-selected-code-proposal-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-selected-code-proposal-test)
    endif()
    foreach(case apply stale cancel retained empty parent-close)
        add_test(NAME studio.document.proposal.gtk4.${case} COMMAND umicom-studio-selected-code-proposal-test ${case})
        set_tests_properties(studio.document.proposal.gtk4.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;document;proposal;gtk4;regression")
    endforeach()
endif()

# This owner-run scenario uses the real CMake/CTest processes and generated
# program to verify that distinct stages and the launch folder compose end to end.
if(TARGET umicom-studio-project-workflow-test)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/project-preset-launch")
    add_test(NAME studio.project_workflow.preset_launch
        COMMAND umicom-studio-project-workflow-test preset-launch)
    set_tests_properties(studio.project_workflow.preset_launch PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/project-preset-launch"
        TIMEOUT 180 LABELS "studio;project-workflow;presets;launch;integration;toolchain")
endif()

# Native preference actions keep the fixture's user-data path isolated while
# exercising real serialization, worker completion and retained GTK controls.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-adapter-preferences-test tests/test_adapter_preferences_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-adapter-preferences-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_compile_definitions(umicom-studio-adapter-preferences-test PRIVATE UmiDebugAdapterPreferencesPath=FixtureAdapterPreferencesPath)
    target_link_libraries(umicom-studio-adapter-preferences-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-adapter-preferences-test)
    umicom_apply_sanitizers(umicom-studio-adapter-preferences-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-adapter-preferences-test)
    endif()
    foreach(case current save load use changed missing malformed save-close load-close)
        add_test(NAME studio.debug_adapter_preferences.gtk4.${case} COMMAND umicom-studio-adapter-preferences-test ${case})
        set_tests_properties(studio.debug_adapter_preferences.gtk4.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77
            LABELS "studio;debugger;preferences;gtk4;ownership;regression")
    endforeach()
endif()


# Real native patch workflows keep Studio's adapter aligned with Framework.
# Each case owns a fresh directory and never needs a provider or credentials.
if(BUILD_TESTING)
    add_executable(umicom-studio-coding-local-files-test tests/test_coding_local_files.c)
    target_link_libraries(umicom-studio-coding-local-files-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-coding-local-files-test)
    umicom_apply_sanitizers(umicom-studio-coding-local-files-test)
    foreach(case create modify conflict delete binary capture)
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/coding-local-files/${case}")
        add_test(NAME studio.coding.local_files.${case} COMMAND umicom-studio-coding-local-files-test ${case})
        set_tests_properties(studio.coding.local_files.${case} PROPERTIES TIMEOUT 60
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qualification/coding-local-files/${case}"
            LABELS "studio;coding;filesystem;regression")
    endforeach()
endif()

# The editor action opens a captured Framework review without launching a server.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-source-completion-test tests/test_source_completion_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-source-completion-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-source-completion-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-source-completion-test)
    umicom_apply_sanitizers(umicom-studio-source-completion-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-source-completion-test)
    endif()
    foreach(case open cancel retained empty parent-close)
        add_test(NAME studio.language.completion_review.gtk4.${case} COMMAND umicom-studio-source-completion-test ${case})
        set_tests_properties(studio.language.completion_review.gtk4.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;language;completion;gtk4;ownership;regression")
    endforeach()
endif()

# The editor action opens a captured Framework review without launching a server.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-source-formatting-test tests/test_source_formatting_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-source-formatting-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-source-formatting-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-source-formatting-test)
    umicom_apply_sanitizers(umicom-studio-source-formatting-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-source-formatting-test)
    endif()
    foreach(case open cancel retained empty parent-close)
        add_test(NAME studio.language.formatting_review.gtk4.${case} COMMAND umicom-studio-source-formatting-test ${case})
        set_tests_properties(studio.language.formatting_review.gtk4.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;language;formatting;gtk4;ownership;regression")
    endforeach()
endif()

# The editor action opens a captured Framework review without launching a server.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-source-information-test tests/test_source_information_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-source-information-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-source-information-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-source-information-test)
    umicom_apply_sanitizers(umicom-studio-source-information-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-source-information-test)
    endif()
    foreach(case open cancel retained empty parent-close)
        add_test(NAME studio.language.source_information.gtk4.${case} COMMAND umicom-studio-source-information-test ${case})
        set_tests_properties(studio.language.source_information.gtk4.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;language;source-information;gtk4;ownership;regression")
    endforeach()
endif()

# The editor action opens a captured Framework review without launching a server.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-source-definition-test tests/test_source_definition_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-source-definition-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-source-definition-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-source-definition-test)
    umicom_apply_sanitizers(umicom-studio-source-definition-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-source-definition-test)
    endif()
    foreach(case open cancel retained empty parent-close)
        add_test(NAME studio.language.source_definition.gtk4.${case} COMMAND umicom-studio-source-definition-test ${case})
        set_tests_properties(studio.language.source_definition.gtk4.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;language;source-definition;gtk4;ownership;regression")
    endforeach()
endif()

# The editor action opens a captured Framework review without launching a server.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-source-references-test tests/test_source_references_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-source-references-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-source-references-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-source-references-test)
    umicom_apply_sanitizers(umicom-studio-source-references-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-source-references-test)
    endif()
    foreach(case open cancel retained empty parent-close)
        add_test(NAME studio.language.source_references.gtk4.${case} COMMAND umicom-studio-source-references-test ${case})
        set_tests_properties(studio.language.source_references.gtk4.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;language;source-references;gtk4;ownership;regression")
    endforeach()
endif()

# The editor action opens a captured Framework review without launching a server.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-document-symbols-test tests/test_document_symbols_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-document-symbols-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-document-symbols-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-document-symbols-test)
    umicom_apply_sanitizers(umicom-studio-document-symbols-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-document-symbols-test)
    endif()
    foreach(case open cancel retained empty parent-close outline-open outline-retained outline-parent-close)
        add_test(NAME studio.language.document_symbols.gtk4.${case} COMMAND umicom-studio-document-symbols-test ${case})
        set_tests_properties(studio.language.document_symbols.gtk4.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;language;document-symbols;gtk4;ownership;regression")
    endforeach()
endif()

# Parameter help opens a captured inspection; requesting is a separate action.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-parameter-help-test tests/test_parameter_help_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-parameter-help-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-parameter-help-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-parameter-help-test)
    umicom_apply_sanitizers(umicom-studio-parameter-help-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-parameter-help-test)
    endif()
    foreach(case open cancel retained empty parent-close)
        add_test(NAME studio.language.parameter_help.gtk4.${case} COMMAND umicom-studio-parameter-help-test ${case})
        set_tests_properties(studio.language.parameter_help.gtk4.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;language;parameter-help;gtk4;ownership;regression")
    endforeach()
endif()

# Symbol rename opens review first; a server starts only after Request.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-symbol-rename-test tests/test_symbol_rename_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-symbol-rename-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-symbol-rename-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-symbol-rename-test)
    umicom_apply_sanitizers(umicom-studio-symbol-rename-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-symbol-rename-test)
    endif()
    foreach(case open cancel retained empty parent-close)
        add_test(NAME studio.language.symbol_rename.gtk4.${case} COMMAND umicom-studio-symbol-rename-test ${case})
        set_tests_properties(studio.language.symbol_rename.gtk4.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;language;symbol-rename;gtk4;ownership;regression")
    endforeach()
endif()

# Code actions use the shared document review; opening never launches a server.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-code-actions-test tests/test_code_actions_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-code-actions-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-code-actions-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-code-actions-test)
    umicom_apply_sanitizers(umicom-studio-code-actions-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-code-actions-test)
    endif()
    foreach(case open cancel retained empty parent-close)
        add_test(NAME studio.language.code_actions.gtk4.${case} COMMAND umicom-studio-code-actions-test ${case})
        set_tests_properties(studio.language.code_actions.gtk4.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;language;code-actions;gtk4;ownership;regression")
    endforeach()
    add_executable(umicom-studio-source-diagnostics-test tests/test_source_diagnostics_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-source-diagnostics-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-source-diagnostics-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-source-diagnostics-test)
    umicom_apply_sanitizers(umicom-studio-source-diagnostics-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-source-diagnostics-test)
    endif()
    foreach(case open cancel retained empty parent-close)
        add_test(NAME studio.language.source_diagnostics.gtk4.${case} COMMAND umicom-studio-source-diagnostics-test ${case})
        set_tests_properties(studio.language.source_diagnostics.gtk4.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;language;source-diagnostics;gtk4;ownership;regression")
    endforeach()
endif()


# Complete-set review uses Framework ownership and the existing Studio host.
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-replacement-set-test
        tests/test_replacement_set_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-replacement-set-test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-replacement-set-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-replacement-set-test)
    umicom_apply_sanitizers(umicom-studio-replacement-set-test)
    foreach(case apply-both no-review partial-review stale teardown pending-close owned-fields creation-teardown undo-first)
        add_test(NAME studio.replacement_set.gtk4.${case} COMMAND umicom-studio-replacement-set-test ${case})
        set_tests_properties(studio.replacement_set.gtk4.${case} PROPERTIES
            TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;gtk4;document;replacement;acceptance")
    endforeach()
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-replacement-set-test)
    endif()
endif()

install(FILES docs/COMPLETE_DOCUMENT_REPLACEMENTS.html DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)

# Educational guide for the shared complete source-review workflow.
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/docs/WORKSPACE_RENAME.html" DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)

# Install the user workflow beside the existing single-draft code-action guide.
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/docs/WORKSPACE_CODE_ACTIONS.html" DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)

# Ship the explicit diagnostic workflow alongside notification-mode guidance.
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/docs/PULL_DIAGNOSTICS.html" DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)

# Keep project discovery guidance available beside the document-outline guide.
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/docs/PROJECT_SYMBOLS.html" DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/UmicomStudioTypeNavigation.cmake")

# Explain the selection scope alongside the existing whole-document workflow.
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/docs/SELECTION_FORMATTING.html" DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)

# Teach explicit action-family requests and complete edit review.
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/docs/ACTION_FAMILIES.html" DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)

# Explain explicit source destinations in the shared Problems workflow.
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/docs/RELATED_DIAGNOSTICS.html" DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/UmicomStudioSelectionRanges.cmake")

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/UmicomStudioActionResolution.cmake")

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/UmicomStudioCallHierarchy.cmake")

include(cmake/UmicomStudioSnippets.cmake)

include(cmake/UmicomStudioFolding.cmake)

include(cmake/UmicomStudioDelimiters.cmake)

include(cmake/UmicomStudioActionGroups.cmake)
include(cmake/UmicomStudioQuickOpen.cmake)
include(cmake/UmicomStudioSearchOptions.cmake)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/UmicomStudioSyntaxFolding.cmake")

include(cmake/UmicomStudioLineEdits.cmake)

include(cmake/UmicomStudioTypingHistory.cmake)

include(cmake/UmicomStudioRecovery.cmake)


# Saved setups exercise the real shared owner and Studio's native-process gate.
if(BUILD_TESTING)
    add_executable(umicom-studio-debug-setup-test tests/test_debug_setup.c)
    target_link_libraries(umicom-studio-debug-setup-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-debug-setup-test)
    umicom_apply_sanitizers(umicom-studio-debug-setup-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-debug-setup-test)
    endif()
    foreach(case apply stale active legacy-collision)
        add_test(NAME studio.debug_setup.service.${case} COMMAND umicom-studio-debug-setup-test ${case})
        set_tests_properties(studio.debug_setup.service.${case} PROPERTIES TIMEOUT 30 LABELS "studio;debugger;setup;regression")
    endforeach()
endif()
if(BUILD_TESTING AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-debug-setup-gtk4-test tests/test_debug_setup_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-debug-setup-gtk4-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-debug-setup-gtk4-test PRIVATE Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-debug-setup-gtk4-test)
    umicom_apply_sanitizers(umicom-studio-debug-setup-gtk4-test)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-debug-setup-gtk4-test)
    endif()
    foreach(case save save-close load-close apply previous stale no-approval missing malformed changed)
        add_test(NAME studio.debug_setup.gtk4.${case} COMMAND umicom-studio-debug-setup-gtk4-test ${case})
        set_tests_properties(studio.debug_setup.gtk4.${case} PROPERTIES TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;debugger;setup;gtk4;ownership;regression")
    endforeach()
endif()

# Memory inspection uses captured references and explicit bounded read requests.
# This fixture uses protocol samples; it never modifies a user's debuggee.
if(BUILD_TESTING AND TARGET umicom-dap-protocol-fixture AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-memory-inspection-native-test tests/test_memory_inspection_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-memory-inspection-native-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-memory-inspection-native-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-memory-inspection-native-test)
    umicom_apply_sanitizers(umicom-studio-memory-inspection-native-test)
    add_dependencies(umicom-studio-memory-inspection-native-test umicom-dap-protocol-fixture)
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-memory-inspection-native-test)
    endif()
    foreach(case success offset invalid-offset invalid-count unsupported malformed failed-refresh stale detached close reentrant)
        add_test(NAME studio.memory_inspection.gtk4.${case} COMMAND umicom-studio-memory-inspection-native-test
            "$<TARGET_FILE:umicom-dap-protocol-fixture>" ${case})
        set_tests_properties(studio.memory_inspection.gtk4.${case} PROPERTIES
            TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;gtk4;debugger;memory;acceptance")
    endforeach()
endif()
install(FILES docs/READING_DEBUGGER_MEMORY.html DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)

# Native and service restart use a controlled DAP peer, not an installed debugger.
if(BUILD_TESTING AND TARGET umicom-dap-protocol-fixture)
    add_executable(umicom-studio-debug-restart-test tests/test_debug_restart.c)
    target_link_libraries(umicom-studio-debug-restart-test PRIVATE Umicom::StudioCore)
    umicom_apply_warnings(umicom-studio-debug-restart-test)
    umicom_apply_sanitizers(umicom-studio-debug-restart-test)
    add_dependencies(umicom-studio-debug-restart-test umicom-dap-protocol-fixture)
    foreach(case supported unsupported rejected event-first)
        add_test(NAME studio.debug_restart.service.${case} COMMAND umicom-studio-debug-restart-test
            "$<TARGET_FILE:umicom-dap-protocol-fixture>" "${case}")
        set_tests_properties(studio.debug_restart.service.${case} PROPERTIES
            TIMEOUT 30 LABELS "studio;debugger;restart;protocol-fixture;regression")
    endforeach()
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-debug-restart-test)
    endif()
endif()
if(BUILD_TESTING AND TARGET umicom-dap-protocol-fixture AND TARGET umicom-studio-workspace-canvas-test)
    add_executable(umicom-studio-debug-restart-native-test tests/test_debug_restart_gtk4.c src/gui/workbench/workbench_window.c)
    target_include_directories(umicom-studio-debug-restart-native-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/include")
    target_link_libraries(umicom-studio-debug-restart-native-test PRIVATE
        Umicom::StudioCore Umicom::ui_gtk4 PkgConfig::GTK4 PkgConfig::GLIB PkgConfig::GIO)
    umicom_apply_warnings(umicom-studio-debug-restart-native-test)
    umicom_apply_sanitizers(umicom-studio-debug-restart-native-test)
    add_dependencies(umicom-studio-debug-restart-native-test umicom-dap-protocol-fixture)
    foreach(case supported unsupported close)
        add_test(NAME studio.debug_restart.gtk4.${case} COMMAND umicom-studio-debug-restart-native-test
            "$<TARGET_FILE:umicom-dap-protocol-fixture>" "${case}")
        set_tests_properties(studio.debug_restart.gtk4.${case} PROPERTIES
            TIMEOUT 60 SKIP_RETURN_CODE 77 LABELS "studio;gtk4;debugger;restart;ownership;acceptance")
    endforeach()
    if(COMMAND umicom_register_validation_target)
        umicom_register_validation_target(umicom-studio-debug-restart-native-test)
    endif()
endif()
install(FILES docs/RESTARTING_DEBUG_SESSIONS.html DESTINATION ${CMAKE_INSTALL_DATADIR}/umicom-studio/docs)

include(cmake/UmicomStudioJobHistory.cmake)
include(cmake/UmicomStudioTestArchive.cmake)
