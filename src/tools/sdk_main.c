/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/sdk_main.c
 *
 * PURPOSE:
 *   Probe an installed Framework SDK from Studio.
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
#include "umicom/studio/sdk.h"
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(int argc,char **argv){UmiSdkValidationResult r;/* Preserve the original failure result so the caller can respond to the correct cause. */ if(argc<2){puts("Usage: umicom-studio-sdk <sdk-prefix>");return 2;}/* Preserve the original failure result so the caller can respond to the correct cause. */ if(umi_studio_sdk_probe(argv[1],&r)!=UMI_STATUS_OK)return 1;printf("SDK probe: %s\nConformance: %zu/%zu\n",r.passed?"PASS":"FAIL",r.conformance.checks_passed,r.conformance.checks_run);return r.passed?0:3;}
