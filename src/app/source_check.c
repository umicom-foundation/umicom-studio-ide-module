/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/source_check.c
 *
 * PURPOSE:
 *   Compose document source checks without duplicating compiler or diagnostic logic.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/


#include "umicom/studio/source_check.h"
#include <stdlib.h>
struct UmiStudioSourceCheck {
    UmiDocumentStore *store;
    UmiCvmDocumentCheckResult result;
};
UmiStatus umi_studio_source_check_create(UmiDocumentStore *store,UmiStudioSourceCheck **out)
{
    if(store==NULL || out==NULL || *out!=NULL) return UMI_STATUS_INVALID_ARGUMENT;
    UmiStudioSourceCheck *check=calloc(1U,sizeof *check);
    if(check==NULL) return UMI_STATUS_OUT_OF_MEMORY;
    check->store=store;
    *out=check;
    return UMI_STATUS_OK;
}
void umi_studio_source_check_destroy(UmiStudioSourceCheck *check) { free(check); }
UmiStatus umi_studio_source_check_run(UmiStudioSourceCheck *check,UmiDocumentId document)
{
    if(check==NULL) return UMI_STATUS_INVALID_ARGUMENT;
    /* The Framework owns capture, compiler invocation, source limits and
     * freshness. Studio supplies the selected document and retains one result. */
    return umi_cvm_check_document(check->store,document,UMC_SOURCE_LIMIT,
        &check->result,sizeof check->result);
}
const UmiCvmDocumentCheckResult *umi_studio_source_check_current(const UmiStudioSourceCheck *check)
{
    return check!=NULL && umi_cvm_document_check_matches(check->store,&check->result,sizeof check->result)
        ? &check->result : NULL;
}
UmiStatus umi_studio_source_check_warning(const UmiStudioSourceCheck *check,size_t index,UmiDiagnosticSnapshot *out)
{
    if(check==NULL) return UMI_STATUS_INVALID_ARGUMENT;
    return umi_cvm_document_check_warning(check->store,&check->result,sizeof check->result,index,out);
}
UmiStatus umi_studio_source_check_error(const UmiStudioSourceCheck *check,UmiDiagnosticSnapshot *out)
{
    if(check==NULL) return UMI_STATUS_INVALID_ARGUMENT;
    return umi_cvm_document_check_error(check->store,&check->result,sizeof check->result,out);
}
