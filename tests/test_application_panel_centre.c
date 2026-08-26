/*-----------------------------------------------------------------------------
 * Umicom Studio IDE Module
 * File: tests/test_application_panel_centre.c
 *
 * PURPOSE:
 *   Verify Studio resolves canonical Trader panels and layouts from Framework.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

#include <assert.h>

#include "umicom/studio/application_panel_centre.h"

int main(void)
{
    UmiApplicationPanelProjection panel;
    UmiApplicationLayoutProjection layout;
    assert(umi_studio_application_panel_row("org.umicom.trader", 0U, &panel) == UMI_STATUS_OK);
    assert(umi_studio_application_layout_row("org.umicom.trader", 0U, &layout) == UMI_STATUS_OK);
    return 0;
}
