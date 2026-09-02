/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/quality_main.c
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
#include "umicom/studio/quality_gate.h"
#include <stdio.h>
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(int argc,char **argv){int passed=0;UmiCodeGuardSummary s;const char *root=argc>1?argv[1]:".";/* Preserve the original failure result so the caller can respond to the correct cause. */ if(umi_studio_quality_gate(root,&passed,&s)!=UMI_STATUS_OK)return 2;printf("Code quality gate: %s (critical=%zu high=%zu architecture=%zu duplicates=%zu)\n",passed?"PASS":"FAIL",s.critical,s.high,s.architecture,s.duplicates);return passed?0:1;}
