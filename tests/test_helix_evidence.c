/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_helix_evidence.c
 *
 * PURPOSE:
 *   Validate Studio's Framework-backed AI and Helix integration.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This focused test verifies one product adapter while keeping the provider-neutral implementation inside Framework.
 */



#include <assert.h>
#include <string.h>
#include "umicom/studio/helix_evidence.h"
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void){UmiStudioAiPlatform *p=NULL;assert(umi_studio_ai_platform_create(&p)==UMI_STATUS_OK);assert(umi_studio_helix_record_evidence(p,"tests","passed")==UMI_STATUS_OK);assert(strcmp(umi_studio_helix_evidence(p,"tests"),"passed")==0);umi_studio_ai_platform_destroy(p);return 0;}
