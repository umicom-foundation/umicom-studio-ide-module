/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/include/editor_preferences_storage.h
 *
 * PURPOSE:
 *   Expose native preference I/O observation and explicit isolated paths for host integration.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_NATIVE_EDITOR_PREFERENCES_STORAGE_H
#define UMICOM_STUDIO_NATIVE_EDITOR_PREFERENCES_STORAGE_H
#include "umicom/editor/indentation.h"
#include "umicom/platform/path.h"
typedef struct UmiStudioGtkWorkbench UmiStudioGtkWorkbench;
typedef struct UmiStudioEditorPreferencesStorageSnapshot {
  int running;
  int save;
  int applied; /* A load changed session settings; native presentation can still fail. */
  UmiStatus status;
  UmiEditorIndentation preferences;
  char path[UMI_PATH_CAPACITY];
} UmiStudioEditorPreferencesStorageSnapshot;
/** Queue an owner-thread request. NULL path selects Studio's per-user config
 * directory; explicit paths must be absolute and are copied before returning.
 * save=1 writes the current applied pair, not a form draft. save=0 loads and
 * applies only if the reviewed current settings are unchanged at completion.
 * One request per window. No source file or document text is changed. */
UmiStatus umi_studio_gtk_workbench_editor_preferences_storage(UmiStudioGtkWorkbench *workbench,
                                                              const char *path, int save);
/** Copy pending/completed result; NOT_FOUND means no request was started. */
UmiStatus umi_studio_gtk_workbench_editor_preferences_storage_snapshot(
    UmiStudioGtkWorkbench *workbench, UmiStudioEditorPreferencesStorageSnapshot *out);
#endif
