/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/database_explorer_queries.h
 *
 * PURPOSE:
 *   Publish Studio's public database explorer queries contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_DATABASE_EXPLORER_QUERIES_H
#define UMICOM_STUDIO_DATABASE_EXPLORER_QUERIES_H
#include "umicom/studio/database_explorer_centre.h"
/**
 * Perform studio database query through the module contract so client applications do not
 * duplicate its policy.
 */
UmiStatus umi_studio_database_query_execute(UmiStudioDatabaseExplorerCentre *centre,const char *statement,UmiDatabaseQueryResult *out_result);
/**
 * Provide the studio database transaction begin operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_database_transaction_begin(UmiStudioDatabaseExplorerCentre *centre);
/**
 * Provide the studio database transaction commit operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_database_transaction_commit(UmiStudioDatabaseExplorerCentre *centre);
/**
 * Provide the studio database transaction rollback operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_database_transaction_rollback(UmiStudioDatabaseExplorerCentre *centre);
#endif
