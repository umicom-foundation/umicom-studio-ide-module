/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_products_platform.c
 *
 * PURPOSE:
 *   Validate products platform behaviour in the reference IDE consumer.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This integration test proves Studio consumes the public Framework contract rather than a private duplicate implementation.
 */
#include <assert.h>
#include <string.h>
#include "umicom/studio/products.h"
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void){const UmiProductProfile *p=umi_studio_product_find("org.umicom.trader");char b[512];assert(p!=NULL);assert(umi_studio_product_report(p,b,sizeof(b))==UMI_STATUS_OK);assert(strstr(b,"Umicom Trader")!=NULL);return 0;}
