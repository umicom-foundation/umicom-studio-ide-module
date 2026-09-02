/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/util/log/status_util.c
 *
 * PURPOSE:
 *   Implement the status util behavior for
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
 * File: src/util/log/status_util.c
 * PURPOSE: Compatibility names for Studio's canonical GTK4 status bar
 * Created by: Umicom Foundation | Author: Sammy Hegab | Date: 2025-10-01 | MIT
 *---------------------------------------------------------------------------*/

#include "status_util.h"  /* UmiStatusBar API */

/* Create the canonical GTK4 status bar through the compatibility API. */
UmiStatusBar *
umi_status_new(void)
{
    return umi_status_bar_new();
}

/* Replace the visible message through Studio's single GTK4 status component. */
void
umi_status_push(UmiStatusBar *st, const char *msg)
{
    if (st == NULL) {
        return;
    }
    umi_status_bar_set(st, msg);
}

/* Release the canonical status component through its matching owner API. */
void
umi_status_free(UmiStatusBar *st)
{
    umi_status_bar_free(st);
}
/*  END OF FILE */
