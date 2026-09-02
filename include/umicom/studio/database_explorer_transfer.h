/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/database_explorer_transfer.h
 *
 * PURPOSE:
 *   Publish Studio's public database explorer transfer contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_DATABASE_EXPLORER_TRANSFER_H
#define UMICOM_STUDIO_DATABASE_EXPLORER_TRANSFER_H
#include "umicom/studio/database_explorer_centre.h"
/**
 * Add studio database transfer only after its inputs and available capacity have been
 * checked.
 */
UmiStatus umi_studio_database_transfer_add(UmiStudioDatabaseExplorerCentre *centre,const UmiDatabaseTransferPlan *plan);
/**
 * Add studio database backup only after its inputs and available capacity have been
 * checked.
 */
UmiStatus umi_studio_database_backup_add(UmiStudioDatabaseExplorerCentre *centre,const UmiDatabaseBackupRecord *record);
/**
 * Provide the studio database backup verify operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_database_backup_verify(UmiStudioDatabaseExplorerCentre *centre,const char *backup_id);
#endif
