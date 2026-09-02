/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/database_explorer_connections.h
 *
 * PURPOSE:
 *   Publish Studio's public database explorer connections contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_DATABASE_EXPLORER_CONNECTIONS_H
#define UMICOM_STUDIO_DATABASE_EXPLORER_CONNECTIONS_H
#include "umicom/studio/database_explorer_centre.h"
/**
 * Add studio database connection only after its inputs and available capacity have been
 * checked.
 */
UmiStatus umi_studio_database_connection_add(UmiStudioDatabaseExplorerCentre *centre,const char *id,const char *name,UmiDatabaseProvider provider,const char *endpoint,const char *database_name,const char *secret_reference,int read_only);
/**
 * Provide the studio database connection activate operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_database_connection_activate(UmiStudioDatabaseExplorerCentre *centre,const char *id);
#endif
