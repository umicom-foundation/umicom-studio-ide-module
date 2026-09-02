/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/database_explorer_views.h
 *
 * PURPOSE:
 *   Publish Studio's public database explorer views contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_DATABASE_EXPLORER_VIEWS_H
#define UMICOM_STUDIO_DATABASE_EXPLORER_VIEWS_H
#include "umicom/studio/database_explorer_centre.h"
/**
 * List the named studio database view kind values accepted by this public contract.
 */
typedef enum UmiStudioDatabaseViewKind { UMI_STUDIO_DATABASE_VIEW_OVERVIEW=1, UMI_STUDIO_DATABASE_VIEW_CONNECTIONS=2, UMI_STUDIO_DATABASE_VIEW_SCHEMA=3, UMI_STUDIO_DATABASE_VIEW_QUERY=4, UMI_STUDIO_DATABASE_VIEW_HISTORY=5, UMI_STUDIO_DATABASE_VIEW_MIGRATIONS=6, UMI_STUDIO_DATABASE_VIEW_TRANSFER=7, UMI_STUDIO_DATABASE_VIEW_BACKUP=8 } UmiStudioDatabaseViewKind;
/**
 * Represent the studio database view data shared with callers of this public contract.
 */
typedef struct UmiStudioDatabaseView { UmiStudioDatabaseViewKind kind; char id[UMI_DATABASE_ID_CAPACITY]; char title[UMI_DATABASE_NAME_CAPACITY]; char summary[UMI_DATABASE_TEXT_CAPACITY]; size_t row_count; uint64_t revision; } UmiStudioDatabaseView;
/**
 * Provide the studio database view kind id operation used by this module and its client
 * applications.
 */
const char *umi_studio_database_view_kind_id(UmiStudioDatabaseViewKind kind);
/**
 * Provide the studio database view build operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_database_view_build(const UmiStudioDatabaseExplorerCentre *centre,UmiStudioDatabaseViewKind kind,UmiStudioDatabaseView *out_view);
#endif
