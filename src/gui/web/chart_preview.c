/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/web/chart_preview.c
 *
 * PURPOSE:
 *   Render a compact GTK4 chart-model summary for the reference workbench.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * Only presentation logic is GTK-specific; series and indicators stay in Framework.
 */

#include "chart_preview.h"
#include <stdio.h>
/*
 * Provide the studio chart preview new operation used by this module and its client
 * applications.
 */
GtkWidget *umi_studio_chart_preview_new(UmiStudioWebPlatform *platform){UmiChartModel *model;UmiChartSnapshot snap;char text[256];/* Protect caller-owned memory by checking that required state is available before it is used. */ if(platform==NULL)return gtk_label_new("Chart unavailable");model=umi_studio_web_platform_chart(platform);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(umi_chart_snapshot(model,&snap)!=UMI_STATUS_OK)return gtk_label_new("Chart snapshot failed");(void)snprintf(text,sizeof(text),"Chart: %s\nSeries: %zu\nPoints: %zu",snap.title,snap.series_count,snap.point_count);return gtk_label_new(text);}
