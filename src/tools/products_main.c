/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/products_main.c
 *
 * PURPOSE:
 *   List canonical Umicom reference products from Studio.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This command exposes one compatibility check in a form that is easy to run from PowerShell, CI, or the future Studio workbench UI.
 */
#include <stdio.h>
#include "umicom/product/product.h"
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void){size_t i;/* Visit each bounded item once so every record receives the same rule. */ for(i=0U;i<umi_reference_product_count();++i){const UmiProductProfile *p=umi_reference_product_at(i);printf("%s | %s | capabilities=%zu | frontends=%zu\n",p->descriptor->branding.display_name,p->descriptor->product_id,p->capability_count,p->frontend_count);}return 0;}
