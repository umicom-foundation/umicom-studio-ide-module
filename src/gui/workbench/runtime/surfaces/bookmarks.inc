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
/* Preserve the earlier completion callback for review. Navigation changes
 * the document before the layout is presented; reporting these stages separately
 * prevents a successful cursor move from hiding an editor-presentation failure.
 * Resolve the existing weak owner again after any native notification. */
#if 0
static void RuntimeBookmarkPanelResult(void *data, UmiStatus status, int visited) {
  UmiStudioFileDialogRequest *request = data;
  GObject *window = NULL;
  UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(request, &window);
  if (runtime != NULL && status == UMI_STATUS_OK && visited)
    (void)RuntimePresentActiveSource(runtime);
  g_clear_object(&window);
}
#endif
static void RuntimeBookmarkPanelResult(void *data, UmiStatus status, int visited)
{
    UmiStudioFileDialogRequest *request = data;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(request, &window);
    if (runtime != NULL && visited) {
        if (status == UMI_STATUS_OK) {
            UmiStatus presentation = RuntimePresentActiveSource(runtime);
            runtime = RuntimeResolveOwner(request, &window);
            if (runtime != NULL && presentation != UMI_STATUS_OK &&
                presentation != UMI_STATUS_CANCELLED) {
                /* Diagnostic text excludes paths and document contents. Keep
                 * the original presentation status visible for owner-run tests. */
                g_printerr("[Studio bookmark] Present selected editor: %s\n",
                    umi_status_text(presentation));
                char *message = g_strdup_printf(
                    "The bookmark was selected, but the editor could not be shown: %s. "
                    "Open the Editor pane and keep your current draft.",
                    umi_status_text(presentation));
                runtime_show_notification(runtime, message);
                g_free(message);
            }
        } else if (status != UMI_STATUS_CANCELLED) {
            /* The shared adapter reports navigation and its document refresh
             * result. Do not relabel this as a failed layout operation. */
            g_printerr("[Studio bookmark] Navigate and refresh document: %s\n",
                umi_status_text(status));
        }
    }
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
