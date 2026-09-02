/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/database_explorer_centre.h
 *
 * PURPOSE:
 *   Publish Studio's public database explorer centre contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_DATABASE_EXPLORER_CENTRE_H
#define UMICOM_STUDIO_DATABASE_EXPLORER_CENTRE_H
#include "umicom/studio/data.h"
#include "umicom/data/explorer.h"
/**
 * Represent the studio database explorer centre data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioDatabaseExplorerCentre { UmiStudioServices *services; UmiDatabaseExplorer *explorer; char active_pane[UMI_DATABASE_ID_CAPACITY]; uint64_t revision; } UmiStudioDatabaseExplorerCentre;
/**
 * Represent the studio database explorer snapshot data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioDatabaseExplorerSnapshot { UmiDatabaseExplorerSnapshot database; char active_pane[UMI_DATABASE_ID_CAPACITY]; int write_mode; uint64_t revision; } UmiStudioDatabaseExplorerSnapshot;
/**
 * Initialise studio database explorer from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_database_explorer_create(UmiStudioServices *services,UmiStudioDatabaseExplorerCentre **out_centre);
/**
 * Release or reset state held by studio database explorer so the same storage can be
 * reused safely.
 */
void umi_studio_database_explorer_destroy(UmiStudioDatabaseExplorerCentre *centre);
/**
 * Provide the studio database explorer activate operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_database_explorer_activate(UmiStudioDatabaseExplorerCentre *centre,const char *pane_id);
/**
 * Provide the studio database explorer set write mode operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_database_explorer_set_write_mode(UmiStudioDatabaseExplorerCentre *centre,int enabled);
/**
 * Provide the studio database explorer snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_database_explorer_snapshot(const UmiStudioDatabaseExplorerCentre *centre,UmiStudioDatabaseExplorerSnapshot *out_snapshot);
/**
 * Provide the studio database explorer service operation used by this module and its
 * client applications.
 */
UmiDatabaseExplorer *umi_studio_database_explorer_service(UmiStudioDatabaseExplorerCentre *centre);
#endif
