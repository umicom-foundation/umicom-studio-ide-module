/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/surfaces/bookmarks.inc
 *
 * PURPOSE:
 *   Embed Framework session bookmarks and reveal the active source after successful navigation.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/ui/gtk4/bookmark_panel.h"

/* The panel owns this weak window request, never the Studio runtime itself. */
static void RuntimeBookmarkPanelOwnerFree(gpointer data) { runtime_file_dialog_request_free(data); }
static void RuntimeBookmarkPanelResult(void *data, UmiStatus status, int visited) {
  UmiStudioFileDialogRequest *request = data;
  GObject *window = NULL;
  UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(request, &window);
  if (runtime != NULL && status == UMI_STATUS_OK && visited)
    (void)RuntimePresentActiveSource(runtime);
  g_clear_object(&window);
}
static GtkWidget *runtime_build_surface_bookmarks(UmiStudioGtkRuntimeChrome *runtime) {
  UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
  if (request == NULL)
    return gtk_label_new("Bookmarks are unavailable for this window.");
  /* Capture both lifetimes before allocating widgets. Creation may notify
   * native observers, so no borrowed runtime is used after this call. */
  GtkWidget *panel = UmiGtk4BookmarkPanelCreate(runtime->owner->adapter, RuntimeBookmarkPanelResult,
                                                request, RuntimeBookmarkPanelOwnerFree);
  if (panel == NULL) {
    runtime_file_dialog_request_free(request);
    return gtk_label_new("Bookmarks need an active document owner.");
  }
  GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_append(GTK_BOX(root), panel);
  g_object_unref(panel);
  return root;
}
