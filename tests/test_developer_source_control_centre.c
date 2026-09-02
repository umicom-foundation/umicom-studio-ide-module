/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_developer_source_control_centre.c
 *
 * PURPOSE:
 *   Verify the test developer source control centre behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_developer_source_control_centre.c
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include <stddef.h>
#include "umicom/studio/source_control_centre.h"
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void){UmiStudioSourceControlCentre*p=NULL;UmiStudioSourceControlCentreSnapshot s;/* Preserve the original failure result so the caller can respond to the correct cause. */ if(umi_studio_source_control_centre_create(&p)!=UMI_STATUS_OK)return 1;/* Preserve the original failure result so the caller can respond to the correct cause. */ if(umi_studio_source_control_centre_snapshot(p,&s)!=UMI_STATUS_OK||!s.available)return 2;umi_studio_source_control_centre_destroy(p);return 0;}
