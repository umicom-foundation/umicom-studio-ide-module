/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/file_drop.inc
 *
 * PURPOSE:
 *   Send editor-area file drops through the shared queued document-opening service.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/ui/gtk4/local_file_drop.h"

static void RuntimeFileDropRelease(gpointer data) { runtime_file_dialog_request_free(data); }

static UmiStatus RuntimeReceiveDroppedFiles(GtkWidget *widget, const char *const *paths,
                                            size_t count, gpointer data) {
  (void)widget;
  UmiStudioFileDialogRequest *request = data;
  GObject *window = NULL;
  UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(request, &window);
  UmiStatus status = UMI_STATUS_CANCELLED;
  if (runtime != NULL && runtime->runtime_ready && UmiGtk4WindowIsOpen(GTK_WINDOW(window))) {
    /* Use the same command permission boundary as explicit file opening.
     * Retain the uncompiled draft spelling for source review. */
#if 0
        status = runtime_command_is_available(runtime, "file.open")
            ? RuntimeStartOpenFiles(runtime, paths, count) : UMI_STATUS_UNAVAILABLE;
#endif
    status = runtime_command_enabled(runtime, "file.open")
                 ? RuntimeStartOpenFiles(runtime, paths, count)
                 : UMI_STATUS_PERMISSION_DENIED;
    if (status != UMI_STATUS_OK)
      runtime_show_notification(runtime, umi_status_text(status));
  }
  g_clear_object(&window);
  return status;
}

static UmiStatus RuntimeAttachFileDrop(UmiStudioGtkRuntimeChrome *runtime) {
  UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
  if (request == NULL)
    return UMI_STATUS_OUT_OF_MEMORY;
  GtkDropTarget *drop = UmiLocalFileDropGtk4Create(
      UMI_DOCUMENT_OPEN_BATCH_MAXIMUM, RuntimeReceiveDroppedFiles, request, RuntimeFileDropRelease);
  if (drop == NULL) {
    runtime_file_dialog_request_free(request);
    return UMI_STATUS_OUT_OF_MEMORY;
  }
  /* The editor region receives native files before a text widget can treat
   * a converted URI as inserted text. Framework owns the captured paths;
   * Studio contributes only its existing command availability and queue. */
  gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(drop), GTK_PHASE_CAPTURE);
  (void)umi_gtk4_automation_tag_widget(runtime->centre_stack, "studio.files.drop-area");
  gtk_widget_add_controller(runtime->centre_stack, GTK_EVENT_CONTROLLER(drop));
  return UMI_STATUS_OK;
}
