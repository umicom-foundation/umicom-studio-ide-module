/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_source_check.c
 *
 * PURPOSE:
 *   Verify selected-document ownership and invalidation over real Framework source checks.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/


#include "umicom/studio/source_check.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);return 1;} } while(0)
int main(void)
{
    UmiDocumentStore *store=NULL; UmiDocumentId first=0U,second=0U;
    UmiStudioSourceCheck *check=NULL;
    CHECK(umi_studio_source_check_create(NULL,&check)==UMI_STATUS_INVALID_ARGUMENT);
    CHECK(umi_document_store_create(&store)==UMI_STATUS_OK);
    CHECK(umi_document_store_new(store,"first.uc",&first)==UMI_STATUS_OK);
    CHECK(umi_document_store_new(store,"second.uc",&second)==UMI_STATUS_OK);
    CHECK(umi_studio_source_check_create(store,&check)==UMI_STATUS_OK);
    CHECK(umi_studio_source_check_create(store,&check)==UMI_STATUS_INVALID_ARGUMENT);
    CHECK(umi_studio_source_check_current(check)==NULL);
    const char *source="int main(void){int unused=7;return 42;}";
    CHECK(umi_document_store_replace_text(store,first,source,strlen(source))==UMI_STATUS_OK);
    CHECK(umi_studio_source_check_run(check,first)==UMI_STATUS_OK);
    const UmiCvmDocumentCheckResult *result=umi_studio_source_check_current(check);
    CHECK(result && result->document.document_id==first && result->source.report.warning_count==1U);
    UmiDiagnosticSnapshot row;
    CHECK(umi_studio_source_check_warning(check,0U,&row)==UMI_STATUS_OK);
    CHECK(row.correlation_id==first);
    CHECK(umi_document_store_replace_text(store,first,"",0U)==UMI_STATUS_OK);
    CHECK(umi_studio_source_check_current(check)==NULL);
    CHECK(umi_studio_source_check_warning(check,0U,&row)==UMI_STATUS_INVALID_STATE);
    CHECK(umi_studio_source_check_run(check,second)==UMI_STATUS_PARSE_ERROR);
    result=umi_studio_source_check_current(check);
    CHECK(result && result->document.document_id==second && !result->source.completed);
    CHECK(umi_studio_source_check_error(check,&row)==UMI_STATUS_OK);
    CHECK(umi_document_store_close(store,second,1)==UMI_STATUS_OK);
    CHECK(umi_studio_source_check_current(check)==NULL);
    CHECK(umi_studio_source_check_run(check,second)==UMI_STATUS_NOT_FOUND);
    CHECK(umi_studio_source_check_current(check)==NULL);
    CHECK(umi_studio_source_check_run(NULL,first)==UMI_STATUS_INVALID_ARGUMENT);
    umi_studio_source_check_destroy(check);
    umi_document_store_destroy(store);
    return 0;
}
