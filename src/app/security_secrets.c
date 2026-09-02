/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/security_secrets.c
 *
 * PURPOSE:
 *   Implement the security secrets behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE | Security secret inventory v2 | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/studio/security_secrets.h"
/*
 * Add studio security secret only after its inputs and available capacity have been
 * checked.
 */
UmiStatus umi_studio_security_secret_add(UmiStudioSecurityCentre *centre,const UmiSecuritySecretMetadata *metadata)
{
    UmiStatus status;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    status = umi_security_secret_inventory_add(&centre->governance.secrets,metadata);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status == UMI_STATUS_OK) { centre->governance.revision += 1U; centre->revision += 1U; }
    return status;
}
/*
 * Find studio security secret while leaving the underlying catalogue or model owned by
 * this module.
 */
const UmiSecuritySecretMetadata *umi_studio_security_secret_find(const UmiStudioSecurityCentre *centre,const char *reference) { return centre == NULL ? NULL : umi_security_secret_inventory_find(&centre->governance.secrets,reference); }
/*
 * Provide the studio security redact field operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_security_redact_field(const UmiStudioSecurityCentre *centre,const char *field_name,const char *value,char *out_value,size_t capacity) { return centre == NULL ? UMI_STATUS_INVALID_ARGUMENT : umi_security_redaction_copy(&centre->governance.redaction,field_name,value,out_value,capacity); }
