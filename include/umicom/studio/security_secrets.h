/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/security_secrets.h
 *
 * PURPOSE:
 *   Publish Studio's public security secrets contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_SECURITY_SECRETS_H
#define UMICOM_STUDIO_SECURITY_SECRETS_H
#include "umicom/studio/security_centre.h"
/**
 * Add studio security secret only after its inputs and available capacity have been
 * checked.
 */
UmiStatus umi_studio_security_secret_add(UmiStudioSecurityCentre *centre,const UmiSecuritySecretMetadata *metadata);
/**
 * Find studio security secret while leaving the underlying catalogue or model owned by
 * this module.
 */
const UmiSecuritySecretMetadata *umi_studio_security_secret_find(const UmiStudioSecurityCentre *centre,const char *reference);
/**
 * Provide the studio security redact field operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_security_redact_field(const UmiStudioSecurityCentre *centre,const char *field_name,const char *value,char *out_value,size_t capacity);
#endif
