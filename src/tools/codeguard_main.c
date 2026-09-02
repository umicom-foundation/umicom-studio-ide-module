/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/codeguard_main.c
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
#include "umicom/studio/codeguard.h"
#include "umicom/codeguard/report.h"
#include <stdio.h>
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(int argc,char **argv){const char *root=argc>1?argv[1]:".";UmiStudioCodeGuard *g=NULL;UmiCodeGuardProfile p=umi_codeguard_profile_default();/* Preserve the original failure result so the caller can respond to the correct cause. */ if(umi_studio_codeguard_create(root,&g)!=UMI_STATUS_OK)return 2;/* Preserve the original failure result so the caller can respond to the correct cause. */ if(umi_studio_codeguard_scan(g,&p)!=UMI_STATUS_OK){umi_studio_codeguard_destroy(g);return 3;}(void)umi_codeguard_report_write(stdout,UMI_CODEGUARD_REPORT_TEXT,umi_studio_codeguard_result(g));UmiCodeGuardSummary s=umi_studio_codeguard_summary(g);umi_studio_codeguard_destroy(g);return umi_codeguard_summary_failed(&s,p.fail_on)?1:0;}
