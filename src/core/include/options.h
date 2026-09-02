/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/core/include/options.h
 *
 * PURPOSE:
 *   Declare the options contract shared by Framework services and thin
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
 * File: src/options.h
 * PURPOSE: Runtime options parsing (GOptionContext)
 * Created by: Umicom Foundation | Author: Sammy Hegab | Date: 2025-10-01 | MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_OPTIONS_H
#define UMICOM_OPTIONS_H

#include <glib.h>

typedef struct {
  gboolean verbose;
  gint     log_level;     /* 0=DEBUG..3=ERROR */
  gchar   *workspace;
  gboolean headless;
} UmiOptions;

/**
 * Read options into validated module state and return a status when input cannot be used.
 */
UmiOptions *umi_options_parse(int *argc, char ***argv, GError **err);
/**
 * Provide the options free operation used by this module and its client applications.
 */
void        umi_options_free(UmiOptions *o);

#endif /* UMICOM_OPTIONS_H */
