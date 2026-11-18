/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/source_check.h
 *
 * PURPOSE:
 *   Define a Studio owner for explicit revision-aware Umicom C source checks.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/


#ifndef UMICOM_STUDIO_SOURCE_CHECK_H
#define UMICOM_STUDIO_SOURCE_CHECK_H
#include "umicom/toolchain/cvm_document_check.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct UmiStudioSourceCheck UmiStudioSourceCheck;
/* One controller belongs to one live document store. It owns its latest result,
 * not the store. Serialize controller calls on the owning frontend thread.
 * This is an explicit Umicom C subset check, never an automatic C23 check. */
UmiStatus umi_studio_source_check_create(UmiDocumentStore *store,UmiStudioSourceCheck **out);
void umi_studio_source_check_destroy(UmiStudioSourceCheck *check);
UmiStatus umi_studio_source_check_run(UmiStudioSourceCheck *check,UmiDocumentId document);
/* Borrow until the next run/destroy, only while the captured document is current.
 * A current compiler error is returned too; inspect check_status/completed. */
const UmiCvmDocumentCheckResult *umi_studio_source_check_current(const UmiStudioSourceCheck *check);
UmiStatus umi_studio_source_check_warning(const UmiStudioSourceCheck *check,size_t index,UmiDiagnosticSnapshot *out);
UmiStatus umi_studio_source_check_error(const UmiStudioSourceCheck *check,UmiDiagnosticSnapshot *out);
#ifdef __cplusplus
}
#endif
#endif
