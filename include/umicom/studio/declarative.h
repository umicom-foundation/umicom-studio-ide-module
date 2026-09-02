/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/declarative.h
 *
 * PURPOSE:
 *   Expose Studio use cases for parsing, validating, formatting and compiling toolkit-neutral .umiapp documents.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This Studio-facing contract composes reusable Framework capabilities.
 * Generic designer and declarative logic remains owned by Umicom Framework.
 */

#ifndef UMICOM_STUDIO_DECLARATIVE_H
#define UMICOM_STUDIO_DECLARATIVE_H
#include "umicom/umicom.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Represent the studio declarative data shared with callers of this public contract.
 */
typedef struct UmiStudioDeclarative UmiStudioDeclarative;
/**
 * Represent the studio declarative snapshot data shared with callers of this public
 * contract.
 */
typedef struct UmiStudioDeclarativeSnapshot { size_t component_types; UmiDeclVersion schema_version; } UmiStudioDeclarativeSnapshot;
/**
 * Initialise studio declarative from caller-provided values so later operations receive a
 * known state.
 */
UmiStatus umi_studio_declarative_create(UmiStudioDeclarative **out_service);
/**
 * Release or reset state held by studio declarative so the same storage can be reused
 * safely.
 */
void umi_studio_declarative_destroy(UmiStudioDeclarative *service);
/**
 * Read studio declarative into validated module state and return a status when input
 * cannot be used.
 */
UmiStatus umi_studio_declarative_parse(UmiStudioDeclarative *service,const char *source,UmiDeclDocument **out_document,UmiDeclDiagnosticList *diagnostics);
/**
 * Check that studio declarative satisfies its contract before another service relies on
 * it.
 */
UmiStatus umi_studio_declarative_validate(UmiStudioDeclarative *service,const UmiDeclDocument *document,UmiDeclDiagnosticList *diagnostics);
/**
 * Provide the studio declarative compile operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_declarative_compile(UmiStudioDeclarative *service,const UmiDeclDocument *document,UmiDeclApplicationPlan *out_plan,UmiDeclDiagnosticList *diagnostics);
/**
 * Provide the studio declarative format operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_declarative_format(UmiStudioDeclarative *service,const char *source,char *out_text,size_t capacity,UmiDeclDiagnosticList *diagnostics);
/**
 * Provide the studio declarative snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_declarative_snapshot(const UmiStudioDeclarative *service,UmiStudioDeclarativeSnapshot *out_snapshot);
/**
 * Provide the studio declarative components operation used by this module and its client
 * applications.
 */
UmiDeclComponentRegistry *umi_studio_declarative_components(UmiStudioDeclarative *service);
/**
 * Provide the studio declarative schema operation used by this module and its client
 * applications.
 */
const UmiDeclSchema *umi_studio_declarative_schema(const UmiStudioDeclarative *service);
#ifdef __cplusplus
}
#endif
#endif
