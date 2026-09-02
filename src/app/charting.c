/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/charting.c
 *
 * PURPOSE:
 *   Implement Studio chart use cases over the reusable Framework chart model.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * The Studio layer names the series and chooses indicators. It does not own the generic numerical implementation.
 */

#include "umicom/studio/charting.h"
/* Release or reset state held by studio charting so the same storage can be reused safely. */
UmiStatus umi_studio_charting_reset(UmiStudioWebPlatform *p,const char *title){UmiChartModel *m;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(p==NULL)return UMI_STATUS_INVALID_ARGUMENT;m=umi_studio_web_platform_chart(p);return umi_chart_model_init(m,title!=NULL?title:"Umicom Studio");}
/*
 * Provide the studio charting add close operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_charting_add_close(UmiStudioWebPlatform *p,int64_t time_ms,double close){UmiChartModel *m;UmiChartSeries *s;UmiChartSeries created;UmiStatus st;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(p==NULL)return UMI_STATUS_INVALID_ARGUMENT;m=umi_studio_web_platform_chart(p);s=umi_chart_model_find_series(m,"close");/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==NULL){st=umi_chart_series_init(&created,"close",UMI_CHART_LINE);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(st!=UMI_STATUS_OK)return st;st=umi_chart_model_add_series(m,&created);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(st!=UMI_STATUS_OK)return st;s=umi_chart_model_find_series(m,"close");}return umi_chart_series_add(s,(UmiChartPoint){time_ms,close});}
/*
 * Provide the studio charting add sma operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_charting_add_sma(UmiStudioWebPlatform *p,size_t period){UmiChartModel *m;UmiChartSeries out;UmiChartSeries *close;UmiStatus st;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(p==NULL)return UMI_STATUS_INVALID_ARGUMENT;m=umi_studio_web_platform_chart(p);close=umi_chart_model_find_series(m,"close");/* Protect caller-owned memory by checking that required state is available before it is used. */ if(close==NULL)return UMI_STATUS_NOT_FOUND;st=umi_chart_indicator_sma(close,period,&out);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(st!=UMI_STATUS_OK)return st;return umi_chart_model_add_series(m,&out);}
