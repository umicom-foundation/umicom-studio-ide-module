/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_authorengine.c
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

/* BEGINNER NOTE:
 * This focused test verifies one product adapter while keeping the provider-neutral implementation inside Framework.
 */



#include <assert.h>
#include "umicom/studio/authorengine.h"
int main(void){UmiAiAuthorEngineConfig c;assert(umi_studio_authorengine_config("uaengine",".",&c)==UMI_STATUS_OK);return 0;}
