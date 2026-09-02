/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/database_explorer_transfer.c
 *
 * PURPOSE:
 *   Implement the database explorer transfer behavior for
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
#include "umicom/studio/database_explorer_transfer.h"
/*
 * Add studio database transfer only after its inputs and available capacity have been
 * checked.
 */
UmiStatus umi_studio_database_transfer_add(UmiStudioDatabaseExplorerCentre *c,const UmiDatabaseTransferPlan *p){UmiStatus s;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(c==NULL)return UMI_STATUS_INVALID_ARGUMENT;s=umi_database_explorer_add_transfer(c->explorer,p);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)c->revision+=1U;return s;}
/*
 * Add studio database backup only after its inputs and available capacity have been
 * checked.
 */
UmiStatus umi_studio_database_backup_add(UmiStudioDatabaseExplorerCentre *c,const UmiDatabaseBackupRecord *r){UmiStatus s;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(c==NULL)return UMI_STATUS_INVALID_ARGUMENT;s=umi_database_backup_catalog_add(&c->explorer->backups,r);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK){c->explorer->revision+=1U;c->revision+=1U;}return s;}
/*
 * Provide the studio database backup verify operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_database_backup_verify(UmiStudioDatabaseExplorerCentre *c,const char *id){UmiStatus s;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(c==NULL)return UMI_STATUS_INVALID_ARGUMENT;s=umi_database_backup_catalog_mark_verified(&c->explorer->backups,id);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK){c->explorer->revision+=1U;c->revision+=1U;}return s;}
