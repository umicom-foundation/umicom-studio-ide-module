/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/architecture_guard.c
 *
 * PURPOSE:
 *   Check reference product profiles remain valid and compatible with public Framework capabilities.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * Studio is the reference consumer. These adapters call public Framework ABI, SDK and product APIs rather than duplicating those mechanisms inside the IDE.
 */
#include "umicom/studio/architecture_guard.h"
/*
 * Perform studio architecture guard through the module contract so client applications do
 * not duplicate its policy.
 */
UmiStatus umi_studio_architecture_guard_run(UmiStudioArchitectureGuard *o){size_t i;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(o==NULL)return UMI_STATUS_INVALID_ARGUMENT;o->profiles_checked=0U;o->invalid_profiles=0U;o->incompatible_profiles=0U;/* Visit each bounded item once so every record receives the same rule. */ for(i=0U;i<umi_reference_product_count();++i){UmiProductValidation v;UmiProductCompatibility c;const UmiProductProfile *p=umi_reference_product_at(i);++o->profiles_checked;(void)umi_product_validation_run(p,&v);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(!v.passed)++o->invalid_profiles;(void)umi_product_compatibility_check(p,&c);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(!c.compatible)++o->incompatible_profiles;}o->passed=o->invalid_profiles==0U&&o->incompatible_profiles==0U;return UMI_STATUS_OK;}
