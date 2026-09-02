/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/security_identities.h
 *
 * PURPOSE:
 *   Publish Studio's public security identities contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_SECURITY_IDENTITIES_H
#define UMICOM_STUDIO_SECURITY_IDENTITIES_H
#include "umicom/studio/security_centre.h"
/**
 * Add studio security identity only after its inputs and available capacity have been
 * checked.
 */
UmiStatus umi_studio_security_identity_add(UmiStudioSecurityCentre *centre,const UmiSecurityIdentityProfile *identity);
/**
 * Provide the studio security identity set enabled operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_security_identity_set_enabled(UmiStudioSecurityCentre *centre,const char *identity_id,bool enabled);
/**
 * Find studio security identity while leaving the underlying catalogue or model owned by
 * this module.
 */
const UmiSecurityIdentityProfile *umi_studio_security_identity_find(const UmiStudioSecurityCentre *centre,const char *identity_id);
#endif
