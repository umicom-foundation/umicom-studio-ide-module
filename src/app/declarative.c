/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/declarative.c
 *
 * PURPOSE:
 *   Implement Studio declarative-language services over the reusable Framework parser, schema and compiler.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This Studio adapter keeps product composition thin: it delegates reusable
 * behaviour to Framework services and adds only Studio-specific coordination.
 */

#include "umicom/studio/declarative.h"
#include <stdlib.h>
#include <string.h>
struct UmiStudioDeclarative{UmiDeclComponentRegistry *components;UmiDeclSchema schema;};
/*
 * Initialise studio declarative from caller-provided values so later operations receive a
 * known state.
 */
UmiStatus umi_studio_declarative_create(UmiStudioDeclarative **out){UmiStudioDeclarative *s;UmiStatus status;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(out==NULL)return UMI_STATUS_INVALID_ARGUMENT;*out=NULL;s=calloc(1U,sizeof(*s));/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==NULL)return UMI_STATUS_OUT_OF_MEMORY;status=umi_designer_catalogue_create(&s->components,&s->schema);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(status!=UMI_STATUS_OK){free(s);return status;}*out=s;return UMI_STATUS_OK;}
/*
 * Release or reset state held by studio declarative so the same storage can be reused
 * safely.
 */
void umi_studio_declarative_destroy(UmiStudioDeclarative *s){/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==NULL)return;umi_decl_component_registry_destroy(s->components);free(s);}
/*
 * Read studio declarative into validated module state and return a status when input
 * cannot be used.
 */
UmiStatus umi_studio_declarative_parse(UmiStudioDeclarative *s,const char *src,UmiDeclDocument **out,UmiDeclDiagnosticList *d){/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==NULL)return UMI_STATUS_INVALID_ARGUMENT;return umi_decl_parse_text(src,out,d);}
/*
 * Check that studio declarative satisfies its contract before another service relies on
 * it.
 */
UmiStatus umi_studio_declarative_validate(UmiStudioDeclarative *s,const UmiDeclDocument *doc,UmiDeclDiagnosticList *d){return s!=NULL?umi_decl_validate(doc,&s->schema,d):UMI_STATUS_INVALID_ARGUMENT;}
/*
 * Provide the studio declarative compile operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_declarative_compile(UmiStudioDeclarative *s,const UmiDeclDocument *doc,UmiDeclApplicationPlan *plan,UmiDeclDiagnosticList *d){return s!=NULL?umi_decl_compile(doc,&s->schema,plan,d):UMI_STATUS_INVALID_ARGUMENT;}
/*
 * Provide the studio declarative format operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_declarative_format(UmiStudioDeclarative *s,const char *src,char *out,size_t cap,UmiDeclDiagnosticList *d){(void)s;return umi_decl_format_text(src,out,cap,d);}
/*
 * Provide the studio declarative snapshot operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_declarative_snapshot(const UmiStudioDeclarative *s,UmiStudioDeclarativeSnapshot *out){/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==NULL||out==NULL)return UMI_STATUS_INVALID_ARGUMENT;(void)memset(out,0,sizeof(*out));out->component_types=umi_decl_component_registry_count(s->components);out->schema_version=s->schema.version;return UMI_STATUS_OK;}
/*
 * Provide the studio declarative components operation used by this module and its client
 * applications.
 */
UmiDeclComponentRegistry *umi_studio_declarative_components(UmiStudioDeclarative *s){return s!=NULL?s->components:NULL;}const UmiDeclSchema *umi_studio_declarative_schema(const UmiStudioDeclarative *s){return s!=NULL?&s->schema:NULL;}
