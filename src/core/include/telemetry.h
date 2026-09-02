/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/core/include/telemetry.h
 *
 * PURPOSE:
 *   Declare the telemetry contract shared by Framework services and thin
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
 * File: src/telemetry.h
 * PURPOSE: Local counters for usage stats (no network)
 * Created by: Umicom Foundation | Author: Sammy Hegab | Date: 2025-10-01 | MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_TELEMETRY_H
#define UMICOM_TELEMETRY_H

#include <glib.h>

/**
 * Provide the tel inc operation used by this module and its client applications.
 */
void umi_tel_inc(const char *counter);
/**
 * Provide the tel get operation used by this module and its client applications.
 */
guint64 umi_tel_get(const char *counter);
/**
 * Provide the tel reset all operation used by this module and its client applications.
 */
void umi_tel_reset_all(void);

#endif /* UMICOM_TELEMETRY_H */
