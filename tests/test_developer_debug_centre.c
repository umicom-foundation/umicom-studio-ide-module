/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_developer_debug_centre.c
 *
 * PURPOSE:
 *   Verify the test developer debug centre behavior for
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
 * File: applications/studio/tests/test_developer_debug_centre.c
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include <stddef.h>
#include "umicom/studio/debug_centre.h"
int main(void){UmiStudioDebugCentre*p=NULL;UmiStudioDebugCentreSnapshot s;if(umi_studio_debug_centre_create(&p)!=UMI_STATUS_OK)return 1;if(umi_studio_debug_centre_snapshot(p,&s)!=UMI_STATUS_OK||!s.available)return 2;umi_studio_debug_centre_destroy(p);return 0;}
