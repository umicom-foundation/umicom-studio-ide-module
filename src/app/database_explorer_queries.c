/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/database_explorer_queries.c
 *
 * PURPOSE:
 *   Implement the database explorer queries behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/studio/database_explorer_queries.h"
/*
 * Perform studio database query through the module contract so client applications do not
 * duplicate its policy.
 */
UmiStatus umi_studio_database_query_execute(UmiStudioDatabaseExplorerCentre *c,const char *statement,UmiDatabaseQueryResult *out){UmiStatus s;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(c==NULL)return UMI_STATUS_INVALID_ARGUMENT;s=umi_database_query_session_execute(&c->explorer->queries,statement,out);c->revision+=1U;return s;}
/*
 * Provide the studio database transaction begin operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_database_transaction_begin(UmiStudioDatabaseExplorerCentre *c){UmiStatus s;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(c==NULL)return UMI_STATUS_INVALID_ARGUMENT;s=umi_database_explorer_transaction_begin(&c->explorer->transaction);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)c->revision+=1U;return s;}
/*
 * Provide the studio database transaction commit operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_database_transaction_commit(UmiStudioDatabaseExplorerCentre *c){UmiStatus s;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(c==NULL)return UMI_STATUS_INVALID_ARGUMENT;s=umi_database_explorer_transaction_commit(&c->explorer->transaction);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)c->revision+=1U;return s;}
/*
 * Provide the studio database transaction rollback operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_database_transaction_rollback(UmiStudioDatabaseExplorerCentre *c){UmiStatus s;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(c==NULL)return UMI_STATUS_INVALID_ARGUMENT;s=umi_database_explorer_transaction_rollback(&c->explorer->transaction);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)c->revision+=1U;return s;}
