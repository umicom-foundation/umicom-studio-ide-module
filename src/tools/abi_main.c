/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/abi_main.c
 *
 * PURPOSE:
 *   Report the Framework ABI and current platform from Studio.
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
#include "umicom/umicom.h"
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void){UmiAbiPlatform p=umi_abi_platform_current();printf("Framework ABI: %u\nPlatform: %s/%s\nPointer size: %zu\n",UMICOM_FRAMEWORK_ABI_VERSION,p.os,p.architecture,p.pointer_size);return 0;}
