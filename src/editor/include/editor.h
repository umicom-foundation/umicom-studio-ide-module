/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/editor/include/editor.h
 *
 * PURPOSE:
 *   Declare the editor contract shared by Framework services and thin
 *   applications.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/editor/editor.h
 *
 * PURPOSE:
 *   Public composite editor state and API: holds the top-level editor widget,
 *   an Output pane, and a Problems list; plus a minimal text buffer for
 *   file operations until the full editor component is integrated.
 *
 * DESIGN:
 *   - Header kept lightweight with forward declarations.
 *   - Avoids deep/relative include paths; names only.
 *
 * API:
 *   UmiEditor *umi_editor_new(void);
 *   GtkWidget *umi_editor_widget(UmiEditor *ed);
 *   void       umi_editor_free(UmiEditor *ed);
 *
 *   // file ops (implemented in editor_actions.c)
 *   gboolean umi_editor_open_file(UmiEditor *ed, const char *path, GError **err);
 *   gboolean umi_editor_save     (UmiEditor *ed, GError **err);
 *   gboolean umi_editor_save_as  (UmiEditor *ed, GError **err);
 *
 * Created by: Umicom Foundation | Developer: Sammy Hegab | Date: 2025-10-13 | MIT
 *---------------------------------------------------------------------------*/
#pragma once
#include <gtk/gtk.h>
#include <glib.h>

G_BEGIN_DECLS  /* allow clean inclusion from C++ units if needed */

/* Forward declarations to keep this header light */
typedef struct _UmiOutputPane  UmiOutputPane;
/**
 * Represent the problem list data shared with callers of this public contract.
 */
typedef struct _UmiProblemList UmiProblemList;
/* Forward-declare Studio's GTK4 status component. Framework's UmiStatus name
 * remains reserved for operation result codes. */
typedef struct _UmiStatusBar UmiStatusBar;

/* Public editor state used across the app. */
typedef struct _UmiEditor {
  GtkWidget      *root;         /* top-level container (VBox + Paned + Notebook) */
  UmiOutputPane  *out;          /* console/output pane                            */
  UmiProblemList *problems;     /* diagnostics/problems list                      */

  GtkTextBuffer  *buffer;       /* detached text buffer (no view yet)            */
  char           *current_file; /* full path of current file (or NULL)           */
  UmiStatusBar   *status;       /* optional GTK4 status component                 */
} UmiEditor;

/**
 * Provide the editor new operation used by this module and its client applications.
 */
UmiEditor *umi_editor_new(void);
/**
 * Provide the editor widget operation used by this module and its client applications.
 */
GtkWidget *umi_editor_widget(UmiEditor *ed);
/**
 * Provide the editor free operation used by this module and its client applications.
 */
void       umi_editor_free(UmiEditor *ed);

/* File ops used elsewhere in the app (see editor_actions.c) */
gboolean umi_editor_open_file(UmiEditor *ed, const char *path, GError **err);
/**
 * Write editor in its stable representation and report capacity or input failures to the
 * caller.
 */
gboolean umi_editor_save     (UmiEditor *ed, GError **err);
/**
 * Provide the editor save as operation used by this module and its client applications.
 */
gboolean umi_editor_save_as  (UmiEditor *ed, GError **err); /* wrapper (no dialog yet) */

G_END_DECLS
/*  END OF FILE */
