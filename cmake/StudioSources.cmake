#-----------------------------------------------------------------------------
# Umicom Studio IDE
# File: cmake/StudioSources.cmake
#
# PURPOSE:
#   Maintain the explicit product-source inventory and provide the Studio
#   workbench-context composition as one reusable target for every GTK workbench
#   executable that needs it.
#
# AUTHOR AND ORGANISATION:
#   Sammy Hegab
#   Umicom Foundation
#
# LICENCE:
#   MIT
#-----------------------------------------------------------------------------

set(UMICOM_STUDIO_PRODUCT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/build/build_queue.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/build/build_runner.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/build/build_system.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/build/build_tasks.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/build/diagnostic_parsers.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/build/task_runner.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/actions.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/diagnostics_router.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/json_store.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/options.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/problem_router.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/proj_templates.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/project_manager.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/recent_files.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/run_config.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/run_pipeline.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/session.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/settings_apply.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/settings_bus.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/telemetry.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/umi_log.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/workspace.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/editor/editor.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/editor/editor_actions.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/editor/keymap.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/app/app.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/app/app_actions.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/app/window.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/app/window_chat_integration.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/chrome/app_menu_llm.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/chrome/status_bar.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/dialogs/about.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/dialogs/open_folder.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/llm_lab/llm_lab.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/menu/command_palette.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/prefs/prefs.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/prefs/prefs_ui.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/theme/theme.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/context_link_centre.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/llm/llm_core.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/llm/llm_http.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/llm/privacy.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/llm/providers/llm_zai.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/llm/studio_codestral_fim.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/panes/chat/chat_pane.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/panes/output/output_console.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/panes/output/output_filters.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/panes/output/output_pane.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/panes/output/output_pane_adapter.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/panes/problems/problem_list.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/plugins/transpile/i18n_translate.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/plugins/transpile/transpile.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/search/rg_discovery.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/search/rg_runner.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/search/ripgrep_args.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/search/search_panel.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/icon.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/resources/ustudio_resources.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/splash.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/util/fs/file_index.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/util/fs/file_io.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/util/fs/file_tree.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/util/fs/fs_walk.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/util/fs/util_path.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/util/git/git_integration.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/util/log/console_logger.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/util/log/logging.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/util/log/profiler.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/util/log/status_util.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/util/sys/ansi_color.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/util/sys/argv.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/util/sys/crash_guard.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/util/sys/env.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/util/sys/msys_env.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/util/sys/timestamp.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/util/sys/utf8.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/util/watchers/path_watcher.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/util/watchers/watcher_integration.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/util/watchers/watcher_recursive.c"
)

# -----------------------------------------------------------------------------
# Shared Studio workbench-context composition
# -----------------------------------------------------------------------------
# context_link_centre.c was previously hidden inside StudioProduct. The small
# workbench demo uses the same workbench_window.c but does not link StudioProduct,
# so its context-link symbols were unresolved. Giving the composition its own
# target fixes the dependency at its architectural boundary without compiling
# the same C file twice into the full IDE.
add_library(umicom_studio_workbench_context STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench/context_link_centre.c"
)
add_library(
    Umicom::StudioWorkbenchContext
    ALIAS umicom_studio_workbench_context
)

target_include_directories(
    umicom_studio_workbench_context
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/workbench"
)

target_link_libraries(
    umicom_studio_workbench_context
    PUBLIC
        Umicom::Framework
)

umicom_apply_warnings(umicom_studio_workbench_context)
umicom_apply_sanitizers(umicom_studio_workbench_context)

# StudioCore is the common dependency of both the workbench demo and the full
# IDE. Publishing this tiny composition dependency from StudioCore therefore
# closes both final link lines without dragging the complete StudioProduct
# implementation into the lightweight workbench demo.
target_link_libraries(
    umicom_studio_core
    PUBLIC
        Umicom::StudioWorkbenchContext
)
