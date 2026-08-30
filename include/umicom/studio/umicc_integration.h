/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/umicc_integration.h
 *
 * PURPOSE:
 *   Publish Studio's public umicc integration contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_UMICC_INTEGRATION_H
#define UMICOM_STUDIO_UMICC_INTEGRATION_H
#include "umicom/compiler/compiler.h"
typedef struct UmiStudioUmiccIntegration { char executable[UMI_COMPILER_PATH_CAPACITY]; char version[64U]; char target[UMI_COMPILER_NAME_CAPACITY]; uint32_t language_mask; bool available; bool local_runtime; bool stable_c_abi; uint64_t revision; } UmiStudioUmiccIntegration;
UmiStatus umi_studio_umicc_integration_configure(UmiStudioUmiccIntegration *integration,const char *executable,const char *version,const char *target,bool local_runtime);
UmiCompilerProvider umi_studio_umicc_integration_provider(const UmiStudioUmiccIntegration *integration);
UmiStatus umi_studio_umicc_integration_validate(const UmiStudioUmiccIntegration *integration,char *out_reason,size_t capacity);
#endif
