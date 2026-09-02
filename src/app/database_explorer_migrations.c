/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/database_explorer_migrations.c
 *
 * PURPOSE:
 *   Implement the database explorer migrations behavior for
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
#include "umicom/studio/database_explorer_migrations.h"
#include <string.h>
/*
 * Add studio database migration only after its inputs and available capacity have been
 * checked.
 */
UmiStatus umi_studio_database_migration_register(UmiStudioDatabaseExplorerCentre *c,uint32_t version,const char *name,const char *checksum){UmiDatabaseMigrationInfo m={0};UmiStatus s;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(c==NULL||name==NULL||checksum==NULL)return UMI_STATUS_INVALID_ARGUMENT;m.version=version;m.state=UMI_DATABASE_MIGRATION_PENDING;s=umi_database_copy_text(m.name,sizeof(m.name),name);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_database_copy_text(m.checksum,sizeof(m.checksum),checksum);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_database_migration_catalog_upsert(&c->explorer->migrations,&m);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)c->revision+=1U;return s;}
/*
 * Provide the studio database migrations apply core operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_database_migrations_apply_core(UmiStudioDatabaseExplorerCentre *c,uint32_t target,uint64_t at,size_t *out_applied){UmiStatus s;size_t applied=0U;uint32_t v;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(c==NULL)return UMI_STATUS_INVALID_ARGUMENT;s=umi_studio_data_apply_core_migrations(c->services,target,&applied);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK){/* Visit each bounded item once so every record receives the same rule. */ for(v=1U;v<=target;++v){/* Protect caller-owned memory by checking that required state is available before it is used. */ if(umi_database_migration_catalog_mark_applied(&c->explorer->migrations,v,at)==UMI_STATUS_NOT_FOUND)continue;}c->revision+=1U;}/* Protect caller-owned memory by checking that required state is available before it is used. */ if(out_applied!=NULL)*out_applied=applied;return s;}
