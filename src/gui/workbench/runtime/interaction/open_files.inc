/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/open_files.inc
 *
 * PURPOSE:
 *   The native host schedules Framework's captured open plan on the owner thread.
 *   Each idle step keeps the application responsive between individual reads.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* Umicom Studio IDE | Sammy Hegab, Umicom Foundation | MIT
 * The native host schedules Framework's captured open plan on the owner thread.
 * Each idle step keeps the application responsive between individual reads. */
#include "umicom/document/open_batch.h"
typedef struct StudioOpenFilesJob {
  UmiStudioFileDialogRequest *owner;
  UmiDocumentOpenBatch *batch;
} StudioOpenFilesJob;
#define STUDIO_OPEN_FILES_JOB "umicom-studio-open-files-job"
#define STUDIO_OPEN_FILES_RESULT "umicom-studio-open-files-result"
static void RuntimeOpenFilesResultFree(gpointer data) { UmiDocumentOpenBatchDestroy(data); }
static void RuntimeOpenFilesFree(gpointer data) {
  StudioOpenFilesJob *job = data;
  GObject *window = g_weak_ref_get(&job->owner->window);
  if (window != NULL && g_object_get_data(window, STUDIO_OPEN_FILES_JOB) == job)
    g_object_set_data(window, STUDIO_OPEN_FILES_JOB, NULL);
  g_clear_object(&window);
  UmiDocumentOpenBatchDestroy(job->batch);
  runtime_file_dialog_request_free(job->owner);
  free(job);
}
/* Results are plain text, so file names cannot become markup. Copy all text
 * before constructing native controls, which may notify application observers. */
static char *RuntimeOpenFilesText(UmiDocumentOpenBatch *batch) {
  GString *text =
      g_string_new("Files are opened independently. Existing unsaved tabs are preserved.\n\n");
  UmiDocumentOpenBatchSummary summary;
  if (UmiDocumentOpenBatchInspect(batch, &summary) != UMI_STATUS_OK)
    return g_string_free(text, FALSE);
  for (size_t i = 0U; i < summary.count; ++i) {
    UmiDocumentOpenItem item;
    if (UmiDocumentOpenBatchAt(batch, i, &item) != UMI_STATUS_OK)
      continue;
    char *display =
        g_utf8_make_valid(item.path[0] != '\0' ? item.path : "(unavailable local path)", -1);
    g_string_append_printf(
        text, "%zu. %s\n   %s\n\n", i + 1U, display,
        item.state == UMI_DOCUMENT_OPEN_SUCCEEDED ? "Opened" : umi_status_text(item.status));
    g_free(display);
  }
  return g_string_free(text, FALSE);
}
/* Retain the original report composition for review. The active version owns
 * its window during construction and disconnects retained buttons with their
 * receiver, avoiding access after native parent/child removal. */
#if 0
static void RuntimeOpenFilesReport(GtkWindow *parent, const char *text) {
  GtkWidget *dialog = gtk_window_new();
  gtk_window_set_title(GTK_WINDOW(dialog), "File opening results");
  gtk_window_set_default_size(GTK_WINDOW(dialog), 640, 400);
  gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
  gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
  GtkWidget *scroll = gtk_scrolled_window_new();
  GtkWidget *view = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
  gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_WORD_CHAR);
  gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(view)), text, -1);
  gtk_widget_set_vexpand(scroll, TRUE);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), view);
  gtk_box_append(GTK_BOX(box), scroll);
  GtkWidget *close = gtk_button_new_with_label("Close results");
  g_signal_connect_swapped(close, "clicked", G_CALLBACK(gtk_window_destroy), dialog);
  gtk_box_append(GTK_BOX(box), close);
  gtk_window_set_child(GTK_WINDOW(dialog), box);
  (void)umi_gtk4_automation_tag_widget(dialog, "studio.file.open-results");
  gtk_window_present(GTK_WINDOW(dialog));
}
#endif
static void RuntimeCloseOpenFilesReport(GtkButton *button, gpointer data) {
  (void)button;
  if (UmiGtk4WindowIsOpen(GTK_WINDOW(data))) gtk_window_destroy(GTK_WINDOW(data));
}
static void RuntimeOpenFilesReport(GtkWindow *parent, const char *text) {
  GtkWidget *dialog = g_object_ref_sink(gtk_window_new());
  gtk_window_set_title(GTK_WINDOW(dialog), "File opening results");
  gtk_window_set_default_size(GTK_WINDOW(dialog), 640, 400);
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
  GtkWidget *scroll = gtk_scrolled_window_new();
  GtkWidget *view = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
  gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_WORD_CHAR);
  gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(view)), text, -1);
  gtk_widget_set_vexpand(scroll, TRUE);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), view);
  gtk_box_append(GTK_BOX(box), scroll);
  GtkWidget *close = gtk_button_new_with_label("Close results");
  g_signal_connect_object(close, "clicked", G_CALLBACK(RuntimeCloseOpenFilesReport), dialog, 0);
  gtk_box_append(GTK_BOX(box), close);
  gtk_window_set_child(GTK_WINDOW(dialog), box);
  (void)umi_gtk4_automation_tag_widget(dialog, "studio.file.open-results");
  /* Publish only after construction, then recheck after native notifications. */
  if (UmiGtk4WindowIsOpen(parent) && UmiGtk4WindowIsOpen(GTK_WINDOW(dialog))) {
    gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
    if (UmiGtk4WindowIsOpen(parent) && UmiGtk4WindowIsOpen(GTK_WINDOW(dialog)))
      gtk_window_present(GTK_WINDOW(dialog));
    else if (UmiGtk4WindowIsOpen(GTK_WINDOW(dialog)))
      gtk_window_destroy(GTK_WINDOW(dialog));
  } else if (UmiGtk4WindowIsOpen(GTK_WINDOW(dialog))) {
    gtk_window_destroy(GTK_WINDOW(dialog));
  }
  g_object_unref(dialog);
}
static gboolean RuntimeOpenFilesTick(gpointer data) {
  StudioOpenFilesJob *job = data;
  GObject *window = NULL;
  UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(job->owner, &window);
  if (runtime == NULL || !UmiGtk4WindowIsOpen(GTK_WINDOW(window))) {
    (void)UmiDocumentOpenBatchCancel(job->batch);
    g_clear_object(&window);
    return G_SOURCE_REMOVE;
  }
  size_t index = 0U;
  UmiStatus step =
      UmiDocumentOpenBatchStep(job->batch, umi_studio_ui_documents(runtime->owner->ui), &index);
  if (step == UMI_STATUS_OK) {
    UmiDocumentOpenItem item;
    if (UmiDocumentOpenBatchAt(job->batch, index, &item) == UMI_STATUS_OK &&
        item.state == UMI_DOCUMENT_OPEN_SUCCEEDED) {
      runtime = RuntimeResolveOwner(job->owner, &window);
      if (runtime != NULL) {
        runtime_copy_selection(runtime->selected_file_path, sizeof runtime->selected_file_path,
                               item.path);
        (void)RuntimePresentActiveSource(runtime);
      }
    }
  } else if (step != UMI_STATUS_NOT_FOUND) {
    (void)UmiDocumentOpenBatchCancel(job->batch);
  }
  runtime = RuntimeResolveOwner(job->owner, &window);
  if (runtime == NULL || !UmiGtk4WindowIsOpen(GTK_WINDOW(window))) {
    (void)UmiDocumentOpenBatchCancel(job->batch);
    g_clear_object(&window);
    return G_SOURCE_REMOVE;
  }
  UmiDocumentOpenBatchSummary summary;
  (void)UmiDocumentOpenBatchInspect(job->batch, &summary);
  if (summary.pending != 0U) {
    g_clear_object(&window);
    return G_SOURCE_CONTINUE;
  }
  char *details = summary.failed != 0U ? RuntimeOpenFilesText(job->batch) : NULL;
  char message[160];
  (void)snprintf(message, sizeof message, "%zu selected file(s) opened; %zu failed; %zu cancelled.",
                 summary.succeeded, summary.failed, summary.cancelled);
  /* Transfer the immutable completed report before notifications. It remains
   * inspectable until another batch completes or the native window is freed. */
  g_object_set_data(window, STUDIO_OPEN_FILES_JOB, NULL);
  g_object_set_data_full(window, STUDIO_OPEN_FILES_RESULT, job->batch, RuntimeOpenFilesResultFree);
  job->batch = NULL;
  runtime_show_notification(runtime, message);
  runtime = RuntimeResolveOwner(job->owner, &window);
  if (details != NULL && runtime != NULL && UmiGtk4WindowIsOpen(GTK_WINDOW(window)))
    RuntimeOpenFilesReport(GTK_WINDOW(window), details);
  g_free(details);
  g_clear_object(&window);
  return G_SOURCE_REMOVE;
}
static UmiStatus RuntimeStartOpenFiles(UmiStudioGtkRuntimeChrome *runtime, const char *const *paths,
                                       size_t count) {
  if (runtime == NULL || runtime->owner == NULL || runtime->owner->window == NULL)
    return UMI_STATUS_INVALID_ARGUMENT;
  GObject *window = G_OBJECT(runtime->owner->window);
  if (g_object_get_data(window, STUDIO_OPEN_FILES_JOB) != NULL)
    return UMI_STATUS_BUSY;
  if (!UmiGtk4WindowIsOpen(GTK_WINDOW(window)))
    return UMI_STATUS_CANCELLED;
  StudioOpenFilesJob *job = calloc(1U, sizeof *job);
  if (job == NULL)
    return UMI_STATUS_OUT_OF_MEMORY;
  UmiStatus status = UmiDocumentOpenBatchCreate(paths, count, &job->batch);
  if (status != UMI_STATUS_OK) {
    free(job);
    return status;
  }
  job->owner = runtime_file_dialog_request_new(runtime, 0U);
  if (job->owner == NULL) {
    UmiDocumentOpenBatchDestroy(job->batch);
    free(job);
    return UMI_STATUS_OUT_OF_MEMORY;
  }
  g_object_set_data(window, STUDIO_OPEN_FILES_JOB, job);
  if (g_idle_add_full(G_PRIORITY_DEFAULT_IDLE, RuntimeOpenFilesTick, job, RuntimeOpenFilesFree) ==
      0U) {
    RuntimeOpenFilesFree(job);
    return UMI_STATUS_UNAVAILABLE;
  }
  return UMI_STATUS_OK;
}
static void runtime_open_files_complete(GObject *source, GAsyncResult *result, gpointer data) {
  UmiStudioFileDialogRequest *request = data;
  GError *error = NULL;
  GListModel *files = gtk_file_dialog_open_multiple_finish(GTK_FILE_DIALOG(source), result, &error);
  GObject *window = NULL;
  UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(request, &window);
  if (runtime != NULL && files == NULL && error != NULL && !runtime_file_dialog_cancelled(error))
    runtime_show_notification(runtime, error->message);
  if (runtime != NULL && files != NULL) {
    const guint count = g_list_model_get_n_items(files);
    UmiStatus status = UMI_STATUS_CAPACITY_EXCEEDED;
    char *paths[UMI_DOCUMENT_OPEN_BATCH_MAXIMUM] = {0};
    if (count <= UMI_DOCUMENT_OPEN_BATCH_MAXIMUM) {
      for (guint i = 0U; i < count; ++i) {
        GFile *file = g_list_model_get_item(files, i);
        if (G_IS_FILE(file))
          paths[i] = g_file_get_path(file);
        g_clear_object(&file);
      }
      runtime = RuntimeResolveOwner(request, &window);
      status = runtime != NULL ? RuntimeStartOpenFiles(runtime, (const char *const *)paths, count)
                               : UMI_STATUS_CANCELLED;
    }
    for (size_t i = 0U; i < UMI_DOCUMENT_OPEN_BATCH_MAXIMUM; ++i)
      g_free(paths[i]);
    runtime = RuntimeResolveOwner(request, &window);
    if (runtime != NULL && status != UMI_STATUS_OK)
      runtime_show_notification(
          runtime,
          status == UMI_STATUS_CAPACITY_EXCEEDED
              ? "Select at most 32 files at a time. No files from this selection were opened."
              : umi_status_text(status));
  }
  g_clear_object(&window);
  g_clear_object(&files);
  g_clear_error(&error);
  runtime_file_dialog_request_free(request);
}

/* Retained result bytes are copied before native window construction, which
 * may deliver application observers and retire the current document host. */
static UmiStatus RuntimeReviewOpenFiles(UmiStudioGtkRuntimeChrome *runtime)
{
    if (runtime == NULL || runtime->owner == NULL || runtime->owner->window == NULL)
        return UMI_STATUS_INVALID_ARGUMENT;
    GtkWindow *window = runtime->owner->window;
    if (!UmiGtk4WindowIsOpen(window)) return UMI_STATUS_CANCELLED;
    if (g_object_get_data(G_OBJECT(window), STUDIO_OPEN_FILES_JOB) != NULL)
        return UMI_STATUS_BUSY;
    UmiDocumentOpenBatch *batch = g_object_get_data(G_OBJECT(window), STUDIO_OPEN_FILES_RESULT);
    if (batch == NULL) return UMI_STATUS_NOT_FOUND;
    g_object_ref(window);
    char *text = RuntimeOpenFilesText(batch);
    RuntimeOpenFilesReport(window, text);
    g_free(text);
    g_object_unref(window);
    return UMI_STATUS_OK;
}
