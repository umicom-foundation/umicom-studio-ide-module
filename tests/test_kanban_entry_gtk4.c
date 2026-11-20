/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_kanban_entry_gtk4.c
 *
 * PURPOSE:
 *   Verify project-board entry, native closing and independent Framework ownership.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/kanban/publication_plan_storage.h"
#include "umicom/kanban/document.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include "umicom/studio/workspace.h"
#include "umicom/studio_runtime/workspace_canvas.h"
#include "umicom/ui/gtk4/automation.h"
#include "umicom/ui/gtk4/kanban_dock.h"
#include "umicom/ui/gtk4/kanban_project.h"
#include "umicom/ui/gtk4/kanban_publication_store.h"
#include "workbench_window.h"
#include <glib/gstdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define REQUIRE(c)                                                                                 \
  do {                                                                                             \
    if (!(c)) {                                                                                    \
      fprintf(stderr, "%d: %s\n", __LINE__, #c);                                                   \
      exit(1);                                                                                     \
    }                                                                                              \
  } while (0)
static GtkWidget *Find(GtkWidget *root) {
  if (strcmp(gtk_widget_get_name(root), "umicom-kanban-launcher") == 0)
    return root;
  for (GtkWidget *child = gtk_widget_get_first_child(root); child != NULL;
       child = gtk_widget_get_next_sibling(child)) {
    GtkWidget *found = Find(child);
    if (found != NULL)
      return found;
  }
  return NULL;
}
static GtkWidget *FindNamed(GtkWidget *root, const char *name) {
  if (!strcmp(gtk_widget_get_name(root), name))
    return root;
  for (GtkWidget *child = gtk_widget_get_first_child(root); child;
       child = gtk_widget_get_next_sibling(child)) {
    GtkWidget *found = FindNamed(child, name);
    if (found)
      return found;
  }
  return NULL;
}
static GtkWidget *FindButton(GtkWidget *root, const char *label) {
  if (GTK_IS_BUTTON(root) && g_strcmp0(gtk_button_get_label(GTK_BUTTON(root)), label) == 0)
    return root;
  for (GtkWidget *c = gtk_widget_get_first_child(root); c; c = gtk_widget_get_next_sibling(c)) {
    GtkWidget *found = FindButton(c, label);
    if (found)
      return found;
  }
  return NULL;
}
static GtkWindow *FindBoard(GtkApplication *application) {
  for (GList *item = gtk_application_get_windows(application); item != NULL; item = item->next)
    if (strcmp(gtk_widget_get_name(GTK_WIDGET(item->data)), "umicom-kanban-window") == 0)
      return GTK_WINDOW(item->data);
  return NULL;
}

/* Match the coordinator's native source view. Board fields and other text
 * controls must never be mistaken for the editor under qualification. */
static GtkTextView *FindSourceView(GtkWidget *root, const char *view_id) {
  const char *id = g_object_get_data(G_OBJECT(root), "umicom-view-id");
  if (g_strcmp0(id, view_id) == 0) {
    GtkWidget *editor = g_object_get_data(G_OBJECT(root), "umicom-editor-view");
    if (GTK_IS_TEXT_VIEW(editor))
      return GTK_TEXT_VIEW(editor);
  }
  for (GtkWidget *child = gtk_widget_get_first_child(root); child != NULL;
       child = gtk_widget_get_next_sibling(child)) {
    GtkTextView *editor = FindSourceView(child, view_id);
    if (editor != NULL)
      return editor;
  }
  return NULL;
}

/* Enter text through the real buffer signal. The coordinator must observe the
 * edit and continue owning the same unsaved working copy after board actions. */
static void EditActiveSource(GtkWidget *root, UmiDocumentCoordinator *documents,
                             const char *draft, UmiDocumentWorkingCopySnapshot *snapshot) {
  REQUIRE(umi_document_coordinator_active_snapshot(documents, snapshot) == UMI_STATUS_OK);
  GtkTextView *editor = FindSourceView(root, snapshot->view_id);
  REQUIRE(editor != NULL);
  gtk_text_buffer_set_text(gtk_text_view_get_buffer(editor), draft, -1);
  REQUIRE(umi_document_coordinator_active_snapshot(documents, snapshot) == UMI_STATUS_OK);
  REQUIRE(snapshot->dirty && snapshot->text_length == strlen(draft));
}

/* Read the same native view by identity, including when another tab is active. */
static void RequireSourceDraft(GtkWidget *root, const char *view_id, const char *draft) {
  GtkTextView *editor = FindSourceView(root, view_id);
  REQUIRE(editor != NULL);
  GtkTextBuffer *buffer = gtk_text_view_get_buffer(editor);
  GtkTextIter first, last;
  gtk_text_buffer_get_bounds(buffer, &first, &last);
  char *actual = gtk_text_buffer_get_text(buffer, &first, &last, TRUE);
  REQUIRE(g_strcmp0(actual, draft) == 0);
  g_free(actual);
}

/* Drain each worker action until its controls are available on the GTK owner
 * context. A bounded wait makes a stuck persistence or closure path visible. */
static void RunBoardFileAction(GtkWidget *root, const char *label) {
  GtkWidget *controls = FindNamed(root, "kanban-document-controls");
  GtkWidget *button = FindButton(root, label);
  REQUIRE(controls != NULL && GTK_IS_BUTTON(button) && gtk_widget_get_sensitive(controls));
  g_signal_emit_by_name(button, "clicked");
  REQUIRE(!gtk_widget_get_sensitive(controls));
  gint64 deadline = g_get_monotonic_time() + 10 * G_TIME_SPAN_SECOND;
  while (!gtk_widget_get_sensitive(controls) && g_get_monotonic_time() < deadline) {
    for (unsigned event = 0U; event < 32U && g_main_context_pending(NULL); ++event)
      (void)g_main_context_iteration(NULL, FALSE);
    g_usleep(1000U);
  }
  REQUIRE(gtk_widget_get_sensitive(controls));
}

/* A close request can release a pending file on the shared worker. Observe
 * actual removal from the application's native window list before reopening. */
static void WaitForBoardRemoval(GtkApplication *application) {
  gint64 deadline = g_get_monotonic_time() + 3 * G_TIME_SPAN_SECOND;
  while (FindBoard(application) != NULL && g_get_monotonic_time() < deadline) {
    for (unsigned event = 0U; event < 32U && g_main_context_pending(NULL); ++event)
      (void)g_main_context_iteration(NULL, FALSE);
    g_usleep(1000U);
  }
  REQUIRE(FindBoard(application) == NULL);
}

int main(int argc, char **argv) {
  REQUIRE(argc == 3);
  if (!gtk_init_check()) {
    fprintf(stderr, "GTK display unavailable; Studio board entry coverage skipped.\n");
    return 77;
  }
  char *directory = g_build_filename(argv[2], "studio-kanban-XXXXXX", NULL);
  REQUIRE(g_mkdtemp(directory) != NULL && g_chdir(directory) == 0);
  g_unsetenv("UMICOM_STUDIO_DATA_PATH");
  REQUIRE(g_mkdir_with_parents("config", 0700) == 0);
  UmiSettings *settings = NULL;
  REQUIRE(umi_studio_settings_create(&settings) == UMI_STATUS_OK);
  REQUIRE(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_ALLOW_REMOTE, 0) ==
          UMI_STATUS_OK);
  REQUIRE(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS, 0) ==
          UMI_STATUS_OK);
  REQUIRE(umi_settings_set_boolean(settings, UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION, 0) ==
          UMI_STATUS_OK);
  REQUIRE(umi_studio_settings_save(settings, umi_studio_settings_default_path()) == UMI_STATUS_OK);
  UmiStudioBootstrap *bootstrap = NULL;
  UmiStudioServicesOptions services = {0};
  REQUIRE(umi_studio_bootstrap_create_with_options(&services, &bootstrap) == UMI_STATUS_OK);
  GtkApplication *application =
      gtk_application_new("org.umicom.StudioKanbanTest", G_APPLICATION_NON_UNIQUE);
  GError *error = NULL;
  REQUIRE(g_application_register(G_APPLICATION(application), NULL, &error));
  UmiStudioGtkWorkbench *workbench = NULL;
  UmiStudioGtkWorkbenchOptions options = {0};
  REQUIRE(
      umi_studio_gtk_workbench_create_with_options(application, umi_studio_bootstrap_ui(bootstrap),
                                                   umi_studio_bootstrap_desktop_shell(bootstrap),
                                                   &options, &workbench) == UMI_STATUS_OK);
  GtkWidget *button = Find(GTK_WIDGET(umi_studio_gtk_workbench_window(workbench)));
  REQUIRE(button != NULL);
  g_object_ref(button);
  GtkWindow *board = NULL;
  GtkWidget *project_control = NULL;

  if (strncmp(argv[1], "project", 7U) == 0) {
    UmiStudioServices *project_services =
        umi_studio_ui_services(umi_studio_bootstrap_ui(bootstrap));
    char *project = g_build_filename(directory, "project", NULL);
    char *other = g_build_filename(directory, "other", NULL);
    REQUIRE(g_mkdir_with_parents(project, 0700) == 0 && g_mkdir_with_parents(other, 0700) == 0);
    if (!strcmp(argv[1], "project-native-close") || !strcmp(argv[1], "project-native-close-with-editor")) {
      /* Create the persistent Explorer before opening a project. The same
       * control must follow the workspace rather than keep its initial denial. */
      REQUIRE(umi_studio_gtk_workbench_workspace_open_surface(workbench,
          UMI_STUDIO_SURFACE_EXPLORER, NULL) == UMI_STATUS_OK);
      project_control = umi_gtk4_automation_find_tagged_widget(
          GTK_WIDGET(umi_studio_gtk_workbench_window(workbench)), "studio.explorer.project-board");
      REQUIRE(GTK_IS_BUTTON(project_control) && !gtk_widget_get_sensitive(project_control));
      g_object_ref(project_control);
    }
    if (strcmp(argv[1], "project-none") != 0)
      REQUIRE(umi_studio_workspace_open(project_services, project,
                                        strcmp(argv[1], "project-untrusted") != 0,
                                        0) == UMI_STATUS_OK);
    /* Native synchronization includes project identity and retained command
         * sensitivity. The legacy model/adapter refresh alone does not advance
         * the timer-owned Studio chrome. Retain that earlier call for review. */
#if 0
        REQUIRE(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
#endif
        REQUIRE(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
    UmiStatus status;
    int native_close = !strcmp(argv[1], "project-native-close") ||
        !strcmp(argv[1], "project-native-close-with-editor");
    if (native_close) {
      /* Use the real project button, including its weak owner binding. */
      /* The Explorer entry remains present after a project opens and Welcome
       * disappears. Preserve the Welcome-only lookup for review; closing the
       * board is now exercised through the persistent product command. */
#if 0
      GtkWidget *project_button = umi_gtk4_automation_find_tagged_widget(
          GTK_WIDGET(umi_studio_gtk_workbench_window(workbench)), "studio.project-board");
#endif
      GtkWidget *project_button = umi_gtk4_automation_find_tagged_widget(
          GTK_WIDGET(umi_studio_gtk_workbench_window(workbench)), "studio.explorer.project-board");
      REQUIRE(GTK_IS_BUTTON(project_button) && gtk_widget_get_sensitive(project_button));
      REQUIRE(project_button == project_control);
      g_signal_emit_by_name(project_button, "clicked");
      status = FindBoard(application) != NULL ? UMI_STATUS_OK : UMI_STATUS_NOT_FOUND;
    } else {
      status = umi_studio_gtk_workbench_execute_command(
          workbench, "project.board", strcmp(argv[1], "project-argument") == 0 ? "unexpected" : NULL);
    }
    if (!strcmp(argv[1], "project-none") || !strcmp(argv[1], "project-argument")) {
      REQUIRE(status == (!strcmp(argv[1], "project-none") ? UMI_STATUS_PERMISSION_DENIED
                                                          : UMI_STATUS_INVALID_ARGUMENT));
      REQUIRE(FindBoard(application) == NULL);
    } else {
      REQUIRE(status == UMI_STATUS_OK);
      board = FindBoard(application);
      REQUIRE(board != NULL);
      g_object_ref(board);
      /* Project entry must expose the shared explicit persistence controls.
       * This acceptance path never reads or writes a real user binding. */
      REQUIRE(GTK_IS_BUTTON(FindNamed(GTK_WIDGET(board), "kanban-project-remember")));
      REQUIRE(GTK_IS_BUTTON(FindNamed(GTK_WIDGET(board), "kanban-project-recall")));
      UmiKanbanProjectLocation captured;
      REQUIRE(UmiKanbanGtk4ProjectLocation(board, &captured) == UMI_STATUS_OK &&
              umi_path_equal(captured.project_root, project));

      if (!strcmp(argv[1], "project-public-plan") ||
          !strcmp(argv[1], "project-public-plan-independent")) {
        /* Exercise the shared private-plan control through the real Studio
         * project-board entry. The empty public selection is deliberate. */
        GtkWidget *root = GTK_WIDGET(board);
        GtkWidget *entry = FindNamed(root, "kanban-document-directory");
        REQUIRE(GTK_IS_EDITABLE(entry));
        gtk_editable_set_text(GTK_EDITABLE(entry), directory);
        GtkWidget *create = FindButton(root, "Create board");
        REQUIRE(GTK_IS_BUTTON(create));
        g_signal_emit_by_name(create, "clicked");
        GtkWidget *controls = FindNamed(root, "kanban-document-controls");
        gint64 deadline = g_get_monotonic_time() + 10 * G_TIME_SPAN_SECOND;
        while (!gtk_widget_get_sensitive(controls) && g_get_monotonic_time() < deadline) {
          while (g_main_context_iteration(NULL, FALSE)) {
          }
          g_usleep(1000U);
        }
        REQUIRE(gtk_widget_get_sensitive(controls));
        REQUIRE(umi_gtk4_automation_reveal_tagged_widget(root, "kanban.plan.save") ==
                UMI_STATUS_OK);
        for (unsigned i = 0U; i < 40U; ++i) {
          while (g_main_context_iteration(NULL, FALSE)) {
          }
          g_usleep(1000U);
        }
        GtkWidget *plan_panel = umi_gtk4_automation_find_tagged_widget(root, "kanban.plan.panel");
        GtkWidget *plan_directory =
            umi_gtk4_automation_find_tagged_widget(root, "kanban.plan.directory");
        GtkWidget *save = umi_gtk4_automation_find_tagged_widget(root, "kanban.plan.save");
        REQUIRE(plan_panel && GTK_IS_EDITABLE(plan_directory) && GTK_IS_BUTTON(save));
        gtk_editable_set_text(GTK_EDITABLE(plan_directory), directory);
        if (!strcmp(argv[1], "project-public-plan-independent")) {
          umi_studio_gtk_workbench_destroy(workbench);
          workbench = NULL;
        }
        g_signal_emit_by_name(save, "clicked");
        deadline = g_get_monotonic_time() + 10 * G_TIME_SPAN_SECOND;
        while (UmiKanbanPublicPlanGtk4Pending(plan_panel) && g_get_monotonic_time() < deadline) {
          while (g_main_context_iteration(NULL, FALSE)) {
          }
          g_usleep(1000U);
        }
        REQUIRE(!UmiKanbanPublicPlanGtk4Pending(plan_panel));
        UmiKanbanPublicationPlan *saved = NULL;
        REQUIRE(UmiKanbanPublicationPlanLoad(directory, "reviewed.umipublic", &saved) ==
                UMI_STATUS_OK);
        UmiKanbanPublicationPlanInfo info;
        REQUIRE(UmiKanbanPublicationPlanRead(saved, &info) == UMI_STATUS_OK &&
                info.card_count == 0U);
        UmiKanbanPublicationPlanDestroy(saved);
      }

      if (strncmp(argv[1], "project-dock", 12U) == 0) {
        REQUIRE(umi_studio_gtk_workbench_execute_command(workbench, "view.kanban", NULL) ==
                UMI_STATUS_OK);
        GtkWindow *studio_window = umi_studio_gtk_workbench_window(workbench);
        GtkWidget *dock_button = FindNamed(GTK_WIDGET(studio_window), "studio-board-pane-open");
        REQUIRE(GTK_IS_BUTTON(dock_button));
        g_object_ref(dock_button);
        g_signal_emit_by_name(dock_button, "clicked");
        REQUIRE(UmiKanbanGtk4WindowDocked(board) && gtk_window_get_child(board) == NULL);
        REQUIRE(FindNamed(GTK_WIDGET(studio_window), "kanban-document-controls") != NULL);
        if (!strcmp(argv[1], "project-dock-reuse")) {
          g_signal_emit_by_name(dock_button, "clicked");
          REQUIRE(UmiKanbanGtk4WindowDocked(board));
          size_t count = 0U;
          for (GList *i = gtk_application_get_windows(application); i; i = i->next)
            if (!strcmp(gtk_widget_get_name(GTK_WIDGET(i->data)), "umicom-kanban-window"))
              ++count;
          REQUIRE(count == 1U);
        } else if (!strcmp(argv[1], "project-dock-return")) {
          REQUIRE(umi_studio_gtk_workbench_execute_command(workbench, "project.board", NULL) ==
                  UMI_STATUS_OK);
          REQUIRE(!UmiKanbanGtk4WindowDocked(board) && gtk_window_get_child(board) != NULL);
        } else if (!strcmp(argv[1], "project-dock-independent")) {
          umi_studio_gtk_workbench_destroy(workbench);
          workbench = NULL;
          REQUIRE(!UmiKanbanGtk4WindowDocked(board) && gtk_widget_get_visible(GTK_WIDGET(board)));
          REQUIRE(gtk_window_get_child(board) != NULL);
          g_signal_emit_by_name(dock_button, "clicked");
          REQUIRE(gtk_window_get_child(board) != NULL);
        } else if (!strcmp(argv[1], "project-dock-switch")) {
          REQUIRE(umi_studio_workspace_close(project_services) == UMI_STATUS_OK);
          REQUIRE(umi_studio_workspace_open(project_services, other, 1, 0) == UMI_STATUS_OK);
          REQUIRE(UmiKanbanGtk4WindowDocked(board));
          UmiKanbanProjectLocation unchanged;
          REQUIRE(UmiKanbanGtk4ProjectLocation(board, &unchanged) == UMI_STATUS_OK &&
                  umi_path_equal(unchanged.project_root, project));
          g_signal_emit_by_name(dock_button, "clicked");
          REQUIRE(!UmiKanbanGtk4WindowDocked(board) && gtk_window_get_child(board) != NULL);
          GtkWindow *second = NULL;
          for (GList *i = gtk_application_get_windows(application); i; i = i->next) {
            UmiKanbanProjectLocation other_location;
            if (UmiKanbanGtk4ProjectLocation(GTK_WINDOW(i->data), &other_location) ==
                    UMI_STATUS_OK &&
                umi_path_equal(other_location.project_root, other))
              second = GTK_WINDOW(i->data);
          }
          REQUIRE(second != NULL && second != board && UmiKanbanGtk4WindowDocked(second));
          gtk_window_close(second);
        } else
          REQUIRE(!strcmp(argv[1], "project-dock"));
        g_object_unref(dock_button);
      }

      if (!strcmp(argv[1], "project-reuse")) {
        REQUIRE(umi_studio_gtk_workbench_execute_command(workbench, "project.board", NULL) ==
                UMI_STATUS_OK);
        size_t count = 0U;
        for (GList *item = gtk_application_get_windows(application); item; item = item->next)
          if (!strcmp(gtk_widget_get_name(GTK_WIDGET(item->data)), "umicom-kanban-window"))
            ++count;
        REQUIRE(count == 1U && FindBoard(application) == board);
      }
      if (!strcmp(argv[1], "project-switch")) {
        REQUIRE(umi_studio_workspace_close(project_services) == UMI_STATUS_OK);
        REQUIRE(umi_studio_workspace_open(project_services, other, 1, 0) == UMI_STATUS_OK);
        REQUIRE(umi_studio_gtk_workbench_execute_command(workbench, "project.board", NULL) ==
                UMI_STATUS_OK);
        UmiKanbanProjectLocation unchanged;
        REQUIRE(UmiKanbanGtk4ProjectLocation(board, &unchanged) == UMI_STATUS_OK &&
                !strcmp(unchanged.project_root, captured.project_root));
        GtkWindow *second = NULL;
        for (GList *item = gtk_application_get_windows(application); item; item = item->next) {
          UmiKanbanProjectLocation location;
          if (UmiKanbanGtk4ProjectLocation(GTK_WINDOW(item->data), &location) == UMI_STATUS_OK &&
              umi_path_equal(location.project_root, other))
            second = GTK_WINDOW(item->data);
        }
        REQUIRE(second != NULL && second != board);
        gtk_window_close(second);
      }
      if (!strcmp(argv[1], "project-independent")) {
        umi_studio_gtk_workbench_destroy(workbench);
        workbench = NULL;
        REQUIRE(gtk_window_get_application(board) == application);
        REQUIRE(UmiKanbanGtk4ProjectLocation(board, &captured) == UMI_STATUS_OK);
      }
      if (native_close) {
        GtkWidget *studio_root = GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
        UmiDocumentCoordinator *documents = umi_studio_ui_documents(umi_studio_bootstrap_ui(bootstrap));
        size_t original_count = umi_document_coordinator_count(documents);
        GtkWidget *new_file = umi_gtk4_automation_find_tagged_widget(studio_root, "file.new");
        REQUIRE(GTK_IS_BUTTON(new_file) && gtk_widget_get_sensitive(new_file));
        const bool with_editor = strcmp(argv[1], "project-native-close-with-editor") == 0;
        const char *first_draft = "int before_board_close(void) { return 1; }\n";
        const char *second_draft = "int after_board_close(void) { return 2; }\n";
        UmiDocumentWorkingCopySnapshot first_source = {0}, second_source = {0};
        char *archive = NULL, *board_leaf = NULL, *saved_board = NULL;
        gsize saved_length = 0;
        if (with_editor) {
          g_signal_emit_by_name(new_file, "clicked");
          REQUIRE(umi_document_coordinator_count(documents) == original_count + 1U);
          ++original_count;
          EditActiveSource(studio_root, documents, first_draft, &first_source);

          /* Save a real card through the independent board while Studio holds
           * a dirty source draft. Keep the exact archive bytes for comparison. */
          GtkWidget *board_root = GTK_WIDGET(board);
          GtkWidget *folder = FindNamed(board_root, "kanban-document-directory");
          GtkWidget *filename = FindNamed(board_root, "kanban-document-filename");
          REQUIRE(GTK_IS_EDITABLE(folder) && GTK_IS_EDITABLE(filename));
          /* Project windows suggest a name derived from their captured root. */
          REQUIRE(g_strcmp0(gtk_editable_get_text(GTK_EDITABLE(filename)), captured.filename) == 0);
          board_leaf = g_strdup(captured.filename);
          gtk_editable_set_text(GTK_EDITABLE(folder), directory);
          RunBoardFileAction(board_root, "Create board");
          GtkWidget *title = FindNamed(board_root, "kanban-title");
          GtkWidget *add_card = FindButton(board_root, "Add card");
          REQUIRE(GTK_IS_EDITABLE(title) && GTK_IS_BUTTON(add_card));
          gtk_editable_set_text(GTK_EDITABLE(title), "Review the source draft");
          g_signal_emit_by_name(add_card, "clicked");
          RunBoardFileAction(board_root, "Save board");
          archive = g_build_filename(directory, board_leaf, NULL);
          REQUIRE(g_file_get_contents(archive, &saved_board, &saved_length, NULL) && saved_length > 0U);
        }
        /* A clean board must actually leave the application window list.
         * Merely requesting close did not detect the reported stuck window. */
        gtk_window_close(board);
        /* Keep the original clean-close polling for review. Both closes now
         * share the same wait because an open board can release a file lease. */
#if 0
        gint64 deadline = g_get_monotonic_time() + 3 * G_TIME_SPAN_SECOND;
        while (FindBoard(application) != NULL && g_get_monotonic_time() < deadline) {
          for (unsigned event = 0U; event < 32U && g_main_context_pending(NULL); ++event)
            (void)g_main_context_iteration(NULL, FALSE);
          g_usleep(1000U);
        }
        REQUIRE(FindBoard(application) == NULL);
#endif
        WaitForBoardRemoval(application);
        REQUIRE(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
        if (with_editor)
          RequireSourceDraft(studio_root, first_source.view_id, first_draft);
        new_file = umi_gtk4_automation_find_tagged_widget(studio_root, "file.new");
        REQUIRE(GTK_IS_BUTTON(new_file) && gtk_widget_get_sensitive(new_file));
        g_signal_emit_by_name(new_file, "clicked");
        REQUIRE(umi_document_coordinator_count(documents) == original_count + 1U);
        if (with_editor)
          EditActiveSource(studio_root, documents, second_draft, &second_source);
        /* A newly opened document hides Welcome. Retain that lookup for review and reopen through the same persistent Explorer control. */
        #if 0
        GtkWidget *project_button = umi_gtk4_automation_find_tagged_widget(studio_root, "studio.project-board");
        #endif
        GtkWidget *project_button = umi_gtk4_automation_find_tagged_widget(studio_root, "studio.explorer.project-board");
        REQUIRE(GTK_IS_BUTTON(project_button));
        g_signal_emit_by_name(project_button, "clicked");
        GtkWindow *reopened = FindBoard(application);
        REQUIRE(reopened != NULL && reopened != board);
        REQUIRE(UmiKanbanGtk4ProjectLocation(reopened, &captured) == UMI_STATUS_OK &&
            umi_path_equal(captured.project_root, project));
        if (with_editor) {
          /* Reopen the saved archive through the same project's new window.
           * Loading it must leave both source tabs and the active draft intact. */
          GtkWidget *board_root = GTK_WIDGET(reopened);
          GtkWidget *folder = FindNamed(board_root, "kanban-document-directory");
          GtkWidget *filename = FindNamed(board_root, "kanban-document-filename");
          REQUIRE(GTK_IS_EDITABLE(folder) && GTK_IS_EDITABLE(filename));
          REQUIRE(g_strcmp0(gtk_editable_get_text(GTK_EDITABLE(filename)), board_leaf) == 0);
          gtk_editable_set_text(GTK_EDITABLE(folder), directory);
          RunBoardFileAction(board_root, "Open board");
          REQUIRE(GTK_IS_EDITABLE(FindNamed(board_root, "kanban-title")));
          UmiDocumentWorkingCopySnapshot active;
          REQUIRE(umi_document_coordinator_active_snapshot(documents, &active) == UMI_STATUS_OK);
          REQUIRE(active.dirty && active.revision == second_source.revision &&
                  strcmp(active.view_id, second_source.view_id) == 0);
          RequireSourceDraft(studio_root, first_source.view_id, first_draft);
          RequireSourceDraft(studio_root, second_source.view_id, second_draft);
        }
        gtk_window_close(reopened);
        /* The earlier immediate removal assertion remains for review. A
         * reopened archive closes asynchronously through the same worker. */
#if 0
        REQUIRE(FindBoard(application) == NULL);
#endif
        WaitForBoardRemoval(application);
        if (with_editor) {
          char *observed_board = NULL;
          gsize observed_length = 0;
          REQUIRE(g_file_get_contents(archive, &observed_board, &observed_length, NULL));
          REQUIRE(observed_length == saved_length && memcmp(saved_board, observed_board, saved_length) == 0);
          g_free(observed_board);
          g_free(saved_board);
          g_free(archive);
          /* The shared reader must acquire the released lease and see the saved card. */
          UmiKanbanDocument *closed_document = NULL;
          REQUIRE(umi_kanban_document_open(directory, board_leaf, &closed_document) == UMI_STATUS_OK);
          REQUIRE(umi_kanban_board_count(umi_kanban_document_board(closed_document)) == 1U);
          REQUIRE(umi_kanban_document_close(&closed_document, false) == UMI_STATUS_OK);
          g_free(board_leaf);
        }
        REQUIRE(umi_studio_workspace_close(project_services) == UMI_STATUS_OK);
        /* Native synchronization includes project identity and retained command
         * sensitivity. The legacy model/adapter refresh alone does not advance
         * the timer-owned Studio chrome. Retain that earlier call for review. */
#if 0
        REQUIRE(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
#endif
        REQUIRE(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
        REQUIRE(!gtk_widget_get_sensitive(project_control));
        /* Forced stale activation must still obey command admission. */
        g_signal_emit_by_name(project_control, "clicked");
        REQUIRE(FindBoard(application) == NULL);
        REQUIRE(umi_studio_workspace_open(project_services, other, 1, 0) == UMI_STATUS_OK);
        /* Native synchronization includes project identity and retained command
         * sensitivity. The legacy model/adapter refresh alone does not advance
         * the timer-owned Studio chrome. Retain that earlier call for review. */
#if 0
        REQUIRE(umi_studio_gtk_workbench_refresh(workbench) == UMI_STATUS_OK);
#endif
        REQUIRE(umi_studio_gtk_workbench_workspace_synchronise(workbench) == UMI_STATUS_OK);
        REQUIRE(gtk_widget_get_sensitive(project_control));
        g_signal_emit_by_name(project_control, "clicked");
        GtkWindow *other_board = FindBoard(application);
        REQUIRE(other_board != NULL);
        REQUIRE(UmiKanbanGtk4ProjectLocation(other_board, &captured) == UMI_STATUS_OK &&
            umi_path_equal(captured.project_root, other));
        gtk_window_close(other_board);
        REQUIRE(FindBoard(application) == NULL);
      } else gtk_window_close(board);
    }
    if (workbench != NULL) {
      umi_studio_gtk_workbench_destroy(workbench);
      workbench = NULL;
    }
    g_free(project);
    g_free(other);
  } else if (strcmp(argv[1], "entry") == 0) {
    g_signal_emit_by_name(button, "clicked");
    board = FindBoard(application);
    REQUIRE(board != NULL);
    g_object_ref(board);
    REQUIRE(gtk_widget_get_visible(GTK_WIDGET(board)));
    umi_studio_gtk_workbench_destroy(workbench);
    workbench = NULL;
    REQUIRE(gtk_window_get_application(board) == application);
    REQUIRE(gtk_widget_get_visible(GTK_WIDGET(board)));
    gtk_window_close(board);
  } else {
    REQUIRE(strcmp(argv[1], "retained") == 0);
    umi_studio_gtk_workbench_destroy(workbench);
    workbench = NULL;
    g_signal_emit_by_name(button, "clicked");
    REQUIRE(FindBoard(application) == NULL);
  }
  g_object_unref(button);
  g_clear_object(&project_control);
  g_clear_object(&board);
  umi_studio_bootstrap_destroy(bootstrap);
  umi_settings_destroy(settings);
  g_object_unref(application);
  /* Keep the isolated fixture folder for review; no product files are touched. */
  g_free(directory);
  return 0;
}
