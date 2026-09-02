/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/panes/output/include/output_pane.h
 *
 * PURPOSE:
 *   Declare the output pane contract shared by Framework services and thin
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
 * File: src/output_pane.h
 * PURPOSE: Simple output console widget with append APIs
 * Created by: Umicom Foundation | Author: Sammy Hegab | Date: 2025-10-01 | MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_OUTPUT_PANE_H
#define UMICOM_OUTPUT_PANE_H

#include <gtk/gtk.h>

/**
 * Represent the output pane data shared with callers of this public contract.
 */
typedef struct _UmiOutputPane UmiOutputPane;

/**
 * Provide the output pane new operation used by this module and its client applications.
 */
UmiOutputPane* umi_output_pane_new(void);
/**
 * Provide the output pane widget operation used by this module and its client
 * applications.
 */
GtkWidget*     umi_output_pane_widget(UmiOutputPane *p);
/**
 * Release or reset state held by output pane so the same storage can be reused safely.
 */
void           umi_output_pane_clear(UmiOutputPane *p);
/**
 * Add output pane only after its inputs and available capacity have been checked.
 */
void           umi_output_pane_append(UmiOutputPane *p, const char *text);
/**
 * Provide the output pane append line operation used by this module and its client
 * applications.
 */
void           umi_output_pane_append_line(UmiOutputPane *p, const char *text);
/**
 * Provide the output pane append line err operation used by this module and its client
 * applications.
 */
void           umi_output_pane_append_line_err(UmiOutputPane *p, const char *text);

#endif /* UMICOM_OUTPUT_PANE_H */
