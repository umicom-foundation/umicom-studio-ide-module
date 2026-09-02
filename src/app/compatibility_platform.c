/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/compatibility_platform.c
 *
 * PURPOSE:
 *   Construct Studio compatibility state from Framework ABI, SDK and reference-product APIs.
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
#include "umicom/studio/compatibility_platform.h"
#include <stdio.h>
#include <stdlib.h>
struct UmiStudioCompatibilityPlatform{UmiProductRegistry *products;UmiAbiBaseline baseline;};
/*
 * Initialise studio compatibility platform from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_compatibility_platform_create(UmiStudioCompatibilityPlatform **o){size_t i;UmiStudioCompatibilityPlatform *p;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(o==NULL)return UMI_STATUS_INVALID_ARGUMENT;*o=NULL;p=(UmiStudioCompatibilityPlatform*)calloc(1U,sizeof(*p));/* Protect caller-owned memory by checking that required state is available before it is used. */ if(p==NULL)return UMI_STATUS_OUT_OF_MEMORY;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(umi_product_registry_create(&p->products)!=UMI_STATUS_OK){free(p);return UMI_STATUS_OUT_OF_MEMORY;}/* Visit each bounded item once so every record receives the same rule. */ for(i=0U;i<umi_reference_product_count();++i)(void)umi_product_registry_register(p->products,umi_reference_product_at(i));p->baseline=umi_abi_baseline_current(0U);*o=p;return UMI_STATUS_OK;}
/*
 * Release or reset state held by studio compatibility platform so the same storage can be
 * reused safely.
 */
void umi_studio_compatibility_platform_destroy(UmiStudioCompatibilityPlatform *p){/* Protect caller-owned memory by checking that required state is available before it is used. */ if(p==NULL)return;umi_product_registry_destroy(p->products);free(p);}
/*
 * Return the number of records represented by studio compatibility product without
 * changing their state.
 */
size_t umi_studio_compatibility_product_count(const UmiStudioCompatibilityPlatform *p){return p==NULL?0U:umi_product_registry_count(p->products);}
/*
 * Find studio compatibility product while leaving the underlying catalogue or model owned
 * by this module.
 */
const UmiProductProfile *umi_studio_compatibility_product_at(const UmiStudioCompatibilityPlatform *p,size_t i){/* Protect caller-owned memory by checking that required state is available before it is used. */ if(p==NULL||i>=umi_reference_product_count())return NULL;return umi_reference_product_at(i);}
/*
 * Provide the studio compatibility summary operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_compatibility_summary(const UmiStudioCompatibilityPlatform *p,char *b,size_t cap){int n;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(p==NULL||b==NULL||cap==0U)return UMI_STATUS_INVALID_ARGUMENT;n=snprintf(b,cap,"Framework ABI: %u\nReference products: %zu\n",p->baseline.framework_abi,umi_studio_compatibility_product_count(p));return n<0||(size_t)n>=cap?UMI_STATUS_CAPACITY_EXCEEDED:UMI_STATUS_OK;}
