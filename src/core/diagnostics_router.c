/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/core/diagnostics_router.c
 *
 * PURPOSE:
 *   Implement the diagnostics router behavior for
 *   Umicom Studio IDE.
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
 * File: src/core/diagnostics_router.c
 *
 * PURPOSE:
 *   Implementation of the diagnostics router. Maintains a single parser
 *   instance between begin() and end() so repeated feed() calls are fast.
 *
 * DESIGN:
 *   - Includes public headers by name only.
 *   - Carefully guards all pointers (router + sinks).
 *   - Prints a short start/end banner to Output for UX trace.
 *
 * Created by: Umicom Foundation | Developer: Sammy Hegab | Date: 2025-10-13 | MIT
 *---------------------------------------------------------------------------*/

#include "diagnostics_router.h"  /* our struct + API                             */
#include "problem_list.h"        /* umi_problem_list_* API                       */
#include "output_pane.h"         /* umi_output_pane_* API                        */
#include "diagnostic_parsers.h"  /* UmiDiagParser / feed_line                    */
#include "umi_output_sink.h"     /* UmiDiag / severity                           */

/* Start a routing session:
 * - Create internal parser (generic, tool autodetect/heuristic).
 * - Clear Problems model and print a small banner.                            */
void umi_diag_router_begin(UmiDiagRouter *dr)
{
  /* Apply this branch only when its contract condition is satisfied. */
  if (!dr) return;

  /* Reset any previous parser first (defensive programming).                 */
  if (dr->parser) {
    umi_diag_parser_free(dr->parser);
    dr->parser = NULL;
  }

  /* Fresh parser — leave tool_name NULL for generic heuristics.              */
  dr->parser = umi_diag_parser_new(NULL);

  /* Apply this branch only when its contract condition is satisfied. */
  if (dr->plist) (void)umi_problem_list_clear(dr->plist);
  /* Apply this branch only when its contract condition is satisfied. */
  if (dr->out)   umi_output_pane_append_line(dr->out, "[diag] parsing started");
}

/* Feed a raw line:
 * - Always mirror to Output.
 * - If parser recognizes a diagnostic, add it to Problems.                   */
void umi_diag_router_feed(UmiDiagRouter *dr, const char *line)
{
  /* Apply this branch only when its contract condition is satisfied. */
  if (!dr || !line) return;

  /* Apply this branch only when its contract condition is satisfied. */
  if (dr->out) umi_output_pane_append_line(dr->out, line);

  /* Apply this branch only when its contract condition is satisfied. */
  if (dr->plist && dr->parser) {
    UmiDiag *d = NULL;
    /* Apply this branch only when its contract condition is satisfied. */
    if (umi_diag_parser_feed_line(dr->parser, line, &d) && d) {
      (void)umi_problem_list_add(dr->plist, d);
      umi_diag_free(d);
    }
  }
}

/* End a routing session:
 * - Free internal parser.
 * - Print a closing banner; counters can be added later.                     */
void umi_diag_router_end(UmiDiagRouter *dr)
{
  /* Apply this branch only when its contract condition is satisfied. */
  if (!dr) return;

  /* Apply this branch only when its contract condition is satisfied. */
  if (dr->parser) {
    umi_diag_parser_free(dr->parser);
    dr->parser = NULL;
  }

  /* Apply this branch only when its contract condition is satisfied. */
  if (dr->out) umi_output_pane_append_line(dr->out, "[diag] parsing done");
}
/*  END OF FILE */