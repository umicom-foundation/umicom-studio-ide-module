/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/consolidation_main.c
 *
 * PURPOSE:
 *   Provide a native Studio command for CodeGuard security and consolidation
 *   inspection.
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
#include "umicom/studio/consolidation.h"
#include <stdio.h>
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(int argc,char **argv){UmiStudioConsolidationReport r;const char *root=argc>1?argv[1]:".";/* Preserve the original failure result so the caller can respond to the correct cause. */ if(umi_studio_consolidation_check(root,&r)!=UMI_STATUS_OK)return 2;printf("Studio consolidation: architecture=%zu duplicates=%zu high-risk=%zu ready=%s\n",r.architecture_findings,r.duplicates,r.high_risk,r.ready?"yes":"no");return r.ready?0:1;}
