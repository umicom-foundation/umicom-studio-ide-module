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
UmiStatus umi_studio_database_query_execute(UmiStudioDatabaseExplorerCentre *centre,const char *statement,UmiDatabaseQueryResult *out_result);
UmiStatus umi_studio_database_transaction_begin(UmiStudioDatabaseExplorerCentre *centre);
UmiStatus umi_studio_database_transaction_commit(UmiStudioDatabaseExplorerCentre *centre);
UmiStatus umi_studio_database_transaction_rollback(UmiStudioDatabaseExplorerCentre *centre);
#endif
