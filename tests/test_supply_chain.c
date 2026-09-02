/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_supply_chain.c
 *
 * PURPOSE:
 *   Verify local and signed stable release supply-chain evidence.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include <assert.h>
#include "umicom/studio/supply_chain.h"

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioSupplyChain supply_chain;
    umi_studio_supply_chain_init(&supply_chain);
    assert(umi_studio_supply_chain_mark_local_complete(&supply_chain) ==
           UMI_STATUS_OK);
    assert(umi_studio_supply_chain_ready(
        &supply_chain, UMI_RELEASE_DEVELOPMENT));
    assert(!umi_studio_supply_chain_ready(
        &supply_chain, UMI_RELEASE_STABLE));
    assert(umi_studio_supply_chain_approve_signature(
               &supply_chain, "Umicom Foundation") == UMI_STATUS_OK);
    assert(umi_studio_supply_chain_ready(
        &supply_chain, UMI_RELEASE_STABLE));
    return 0;
}
