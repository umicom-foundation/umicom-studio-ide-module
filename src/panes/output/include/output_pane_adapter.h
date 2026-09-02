/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/panes/output/include/output_pane_adapter.h
 *
 * PURPOSE:
 *   Declare the output pane adapter contract shared by Framework services and
 *   thin applications.
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
 * File: src/output_pane_adapter.h
 * PURPOSE: Bridge legacy UmiOutputPane API to UmiOutputConsole
 * Created by: Umicom Foundation | Author: Sammy Hegab | Date: 2025-10-01 | MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_OUTPUT_PANE_ADAPTER_H
#define UMICOM_OUTPUT_PANE_ADAPTER_H

#include "output_console.h"
#include "output_pane.h" /* legacy API */

/* Adapt a UmiOutputConsole to present the same widget as legacy pane,
 * so existing code using umi_output_pane_* continues to compile. */
typedef struct {
  UmiOutputConsole *console;
} UmiOutputPaneAdapter;

/**
 * Provide the output pane adapter new operation used by this module and its client
 * applications.
 */
UmiOutputPaneAdapter *umi_output_pane_adapter_new(void);
/**
 * Provide the output pane adapter widget operation used by this module and its client
 * applications.
 */
GtkWidget            *umi_output_pane_adapter_widget(UmiOutputPaneAdapter *a);

/* Legacy-style convenience wrappers */
static inline void umi_output_pane_adapter_append(UmiOutputPaneAdapter *a, const char *s){
  /* Apply this branch only when its contract condition is satisfied. */
  if(a && a->console) umi_output_console_append_line(a->console, s);
}
/**
 * Release or reset state held by output pane adapter so the same storage can be reused
 * safely.
 */
static inline void umi_output_pane_adapter_clear(UmiOutputPaneAdapter *a){
  /* Apply this branch only when its contract condition is satisfied. */
  if(a && a->console) umi_output_console_clear(a->console);
}

#endif /* UMICOM_OUTPUT_PANE_ADAPTER_H */
