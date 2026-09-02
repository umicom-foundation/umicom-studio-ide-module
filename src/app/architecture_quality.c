/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/architecture_quality.c
 *
 * PURPOSE:
 *   Integrate Framework CodeGuard analysis into Studio security and
 *   consolidation workflows.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This file is intentionally small and focused. CodeGuard separates scanning
 * rules, analysis engines and reports so new developers can understand one
 * responsibility at a time and test it independently.
 */
#include "umicom/studio/architecture_quality.h"
#include "umicom/studio/consolidation.h"
#include <stdio.h>
/*
 * Provide the studio architecture quality report operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_architecture_quality_report(const char *root,char *buffer,size_t cap){UmiStudioConsolidationReport r;UmiStatus s;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(buffer==NULL||cap==0U)return UMI_STATUS_INVALID_ARGUMENT;s=umi_studio_consolidation_check(root,&r);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)(void)snprintf(buffer,cap,"architecture=%zu duplicates=%zu high-risk=%zu ready=%s",r.architecture_findings,r.duplicates,r.high_risk,r.ready?"yes":"no");return s;}
