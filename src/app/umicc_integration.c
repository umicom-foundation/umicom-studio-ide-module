/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/umicc_integration.c
 *
 * PURPOSE:
 *   Implement the umicc integration behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE | Umicc integration v2 | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/studio/umicc_integration.h"
#include <stdio.h>
#include <string.h>
/*
 * Provide the studio umicc integration configure operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_umicc_integration_configure(UmiStudioUmiccIntegration *integration,const char *executable,const char *version,const char *target,bool local_runtime)
{ /* Protect caller-owned memory by checking that required state is available before it is used. */ if (integration == NULL || executable == NULL || version == NULL || target == NULL || target[0] == '\0') return UMI_STATUS_INVALID_ARGUMENT; (void)memset(integration,0,sizeof(*integration)); (void)snprintf(integration->executable,sizeof(integration->executable),"%s",executable); (void)snprintf(integration->version,sizeof(integration->version),"%s",version); (void)snprintf(integration->target,sizeof(integration->target),"%s",target); integration->language_mask = UINT32_MAX; integration->available = executable[0] != '\0'; integration->local_runtime = local_runtime; integration->stable_c_abi = true; integration->revision = 1U; return UMI_STATUS_OK; }
/*
 * Provide the studio umicc integration provider operation used by this module and its
 * client applications.
 */
UmiCompilerProvider umi_studio_umicc_integration_provider(const UmiStudioUmiccIntegration *integration) { return umi_compiler_umicc_provider(integration != NULL ? integration->executable : "",integration != NULL ? integration->target : ""); }
/*
 * Check that studio umicc integration satisfies its contract before another service relies
 * on it.
 */
UmiStatus umi_studio_umicc_integration_validate(const UmiStudioUmiccIntegration *integration,char *out_reason,size_t capacity)
{ UmiStatus status = UMI_STATUS_OK; const char *reason = "Umicc integration is ready"; /* Protect caller-owned memory by checking that required state is available before it is used. */ if (integration == NULL || out_reason == NULL || capacity == 0U) return UMI_STATUS_INVALID_ARGUMENT; /* Protect caller-owned memory by checking that required state is available before it is used. */ if (!integration->available) { status = UMI_STATUS_UNAVAILABLE; reason = "Umicc executable is not configured"; } else /* Protect caller-owned memory by checking that required state is available before it is used. */ if (!integration->stable_c_abi) { status = UMI_STATUS_INVALID_STATE; reason = "Stable C ABI support is required"; } (void)snprintf(out_reason,capacity,"%s",reason); return status; }
