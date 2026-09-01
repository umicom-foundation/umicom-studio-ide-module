/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/runtime-manager/tests/test_runtime_find.c
 *
 * PURPOSE:
 *   Exercise one Studio Application Hub runtime behaviour.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This file is part of the Studio Application Hub / Runtime Manager.
 * The code is intentionally explicit and bounded so a new C developer can
 * follow the data flow without hidden ownership or private product coupling.
 */

#include <assert.h>
#include "umicom/studio/runtime_catalogue.h"
int main(void){
    UmiStudioRuntimeManager m;
    umi_studio_runtime_manager_init(&m);
    assert(umi_studio_runtime_catalogue_populate(&m)==UMI_STATUS_OK);
    assert(umi_studio_runtime_manager_find_const(&m,"umicom.trader")!=NULL);
    assert(umi_studio_runtime_manager_find_const(&m,"missing")==NULL);
    return 0;
}
