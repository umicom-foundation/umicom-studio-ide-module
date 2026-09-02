/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/util/log/include/status_util.h
 *
 * PURPOSE:
 *   Declare the status util contract shared by Framework services and thin
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
 * File: src/util/log/include/status_util.h
 * PURPOSE: Compatibility names for Studio's canonical GTK4 status bar
 * Created by: Umicom Foundation | Author: Sammy Hegab | Date: 2025-10-01 | MIT
 *---------------------------------------------------------------------------*/
#pragma once
#ifndef UMICOM_STATUS_UTIL_H
#define UMICOM_STATUS_UTIL_H

#include "status_bar.h"   /* Canonical GTK4 UmiStatusBar contract. */

/* Create the canonical GTK4 status bar through the older utility name. This
 * compatibility entry point lets existing callers migrate without defining a
 * second status type that would collide with Framework's UmiStatus enum. */
UmiStatusBar *umi_status_new(void);

/* Push a formatted message to the status bar. NULL/"" clears it. */
void          umi_status_push(UmiStatusBar *st, const char *msg);

/* Free the status bar created by umi_status_new(). */
void          umi_status_free(UmiStatusBar *st);

#endif /* UMICOM_STATUS_UTIL_H */
/* End of src/util/log/include/status_util.h */
