/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/core/include/recent_files.h
 *
 * PURPOSE:
 *   Declare the recent files contract shared by Framework services and thin
 *   applications.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef UMI_RECENT_FILES_H
#define UMI_RECENT_FILES_H

#include <glib.h>

#include "umicom/platform/recent_items.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Studio keeps this small compatibility object while Framework owns the
 * reusable recent-work catalogue, ordering and persistence behavior. */
typedef struct UmiRecent {
  UmiRecentItemRegistry *registry;
  guint max_items;
  gboolean persistence_enabled;
} UmiRecent;

/* Load the Framework registry and import the older JSON list when needed. */
UmiRecent *umi_recent_load(void);

/* Persist recent work with Framework's atomic replacement format. */
gboolean   umi_recent_save(const UmiRecent *r);

/* Record one opened path, update its usage data and keep the list bounded. */
void       umi_recent_add(UmiRecent *r, const char *path);

/* Free the MRU object and all owned entries. */
void       umi_recent_free(UmiRecent *r);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* UMI_RECENT_FILES_H */
