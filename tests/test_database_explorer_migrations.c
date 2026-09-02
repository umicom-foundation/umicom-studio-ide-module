/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_database_explorer_migrations.c
 *
 * PURPOSE:
 *   Verify the test database explorer migrations behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE Tests | Sammy Hegab | Umicom Foundation | MIT */
#include <assert.h>
#include "umicom/studio/database_explorer_migrations.h"
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void){UmiStudioServices *services;UmiStudioDatabaseExplorerCentre *c=NULL;assert(umi_studio_services_create(NULL,NULL,&services)==UMI_STATUS_OK);assert(umi_studio_database_explorer_create(services,&c)==UMI_STATUS_OK);assert(umi_studio_database_migration_register(c,1U,"Studio core","sha256:core")==UMI_STATUS_OK);assert(umi_database_migration_catalog_pending(&c->explorer->migrations)==1U);umi_studio_database_explorer_destroy(c);umi_studio_services_destroy(services);return 0;}
