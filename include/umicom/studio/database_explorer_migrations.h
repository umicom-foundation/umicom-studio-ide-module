/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/database_explorer_migrations.h
 *
 * PURPOSE:
 *   Publish Studio's public database explorer migrations contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_DATABASE_EXPLORER_MIGRATIONS_H
#define UMICOM_STUDIO_DATABASE_EXPLORER_MIGRATIONS_H
#include "umicom/studio/database_explorer_centre.h"
/**
 * Add studio database migration only after its inputs and available capacity have been
 * checked.
 */
UmiStatus umi_studio_database_migration_register(UmiStudioDatabaseExplorerCentre *centre,uint32_t version,const char *name,const char *checksum);
/**
 * Provide the studio database migrations apply core operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_database_migrations_apply_core(UmiStudioDatabaseExplorerCentre *centre,uint32_t target_version,uint64_t applied_at,size_t *out_applied);
#endif
