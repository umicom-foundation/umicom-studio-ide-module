/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/database_explorer_views.c
 *
 * PURPOSE:
 *   Implement the database explorer views behavior for
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
#include "umicom/studio/database_explorer_views.h"
#include <stdio.h>
#include <string.h>
/*
 * Provide the studio database view kind id operation used by this module and its client
 * applications.
 */
const char *umi_studio_database_view_kind_id(UmiStudioDatabaseViewKind k){/* Select the behaviour associated with the requested command or state value. */ switch(k){case UMI_STUDIO_DATABASE_VIEW_OVERVIEW:return "overview";case UMI_STUDIO_DATABASE_VIEW_CONNECTIONS:return "connections";case UMI_STUDIO_DATABASE_VIEW_SCHEMA:return "schema";case UMI_STUDIO_DATABASE_VIEW_QUERY:return "query";case UMI_STUDIO_DATABASE_VIEW_HISTORY:return "history";case UMI_STUDIO_DATABASE_VIEW_MIGRATIONS:return "migrations";case UMI_STUDIO_DATABASE_VIEW_TRANSFER:return "transfer";case UMI_STUDIO_DATABASE_VIEW_BACKUP:return "backup";default:return "unknown";}}
/* Provide the title operation used by this module and its client applications. */
static const char *title(UmiStudioDatabaseViewKind k){/* Select the behaviour associated with the requested command or state value. */ switch(k){case UMI_STUDIO_DATABASE_VIEW_OVERVIEW:return "Database Overview";case UMI_STUDIO_DATABASE_VIEW_CONNECTIONS:return "Connections";case UMI_STUDIO_DATABASE_VIEW_SCHEMA:return "Schemas and Tables";case UMI_STUDIO_DATABASE_VIEW_QUERY:return "Query Editor";case UMI_STUDIO_DATABASE_VIEW_HISTORY:return "Query History";case UMI_STUDIO_DATABASE_VIEW_MIGRATIONS:return "Migrations";case UMI_STUDIO_DATABASE_VIEW_TRANSFER:return "Import and Export";case UMI_STUDIO_DATABASE_VIEW_BACKUP:return "Backups";default:return "Database Explorer";}}
/*
 * Provide the studio database view build operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_database_view_build(const UmiStudioDatabaseExplorerCentre *c,UmiStudioDatabaseViewKind k,UmiStudioDatabaseView *out){UmiStudioDatabaseExplorerSnapshot s;UmiStatus st;int n;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(c==NULL||out==NULL||k<UMI_STUDIO_DATABASE_VIEW_OVERVIEW||k>UMI_STUDIO_DATABASE_VIEW_BACKUP)return UMI_STATUS_INVALID_ARGUMENT;st=umi_studio_database_explorer_snapshot(c,&s);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(st!=UMI_STATUS_OK)return st;(void)memset(out,0,sizeof(*out));out->kind=k;st=umi_database_copy_text(out->id,sizeof(out->id),umi_studio_database_view_kind_id(k));/* Protect caller-owned memory by checking that required state is available before it is used. */ if(st==UMI_STATUS_OK)st=umi_database_copy_text(out->title,sizeof(out->title),title(k));/* Protect caller-owned memory by checking that required state is available before it is used. */ if(st!=UMI_STATUS_OK)return st;/* Select the behaviour associated with the requested command or state value. */ switch(k){case UMI_STUDIO_DATABASE_VIEW_CONNECTIONS:out->row_count=s.database.connections;break;case UMI_STUDIO_DATABASE_VIEW_SCHEMA:out->row_count=s.database.tables;break;case UMI_STUDIO_DATABASE_VIEW_HISTORY:out->row_count=s.database.query_history;break;case UMI_STUDIO_DATABASE_VIEW_MIGRATIONS:out->row_count=s.database.pending_migrations;break;case UMI_STUDIO_DATABASE_VIEW_TRANSFER:out->row_count=s.database.transfers;break;case UMI_STUDIO_DATABASE_VIEW_BACKUP:out->row_count=s.database.backups;break;default:out->row_count=1U;break;}n=snprintf(out->summary,sizeof(out->summary),"%s: %zu row(s); %s backend; %zu stored record(s)",out->title,out->row_count,s.database.backend,s.database.records);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(n<0||(size_t)n>=sizeof(out->summary))return UMI_STATUS_CAPACITY_EXCEEDED;out->revision=s.revision;return UMI_STATUS_OK;}
