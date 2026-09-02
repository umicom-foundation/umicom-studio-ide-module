/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/consolidation.c
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
#include "umicom/studio/consolidation.h"
/*
 * Provide the studio consolidation check operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_consolidation_check(const char *root,UmiStudioConsolidationReport *o){UmiStudioCodeGuard *g=NULL;UmiCodeGuardProfile p=umi_codeguard_profile_architecture();UmiStatus s;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(root==NULL||o==NULL)return UMI_STATUS_INVALID_ARGUMENT;s=umi_studio_codeguard_create(root,&g);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_studio_codeguard_scan(g,&p);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK){UmiCodeGuardSummary m=umi_studio_codeguard_summary(g);o->architecture_findings=m.architecture;o->duplicates=m.duplicates;o->high_risk=m.critical+m.high;o->ready=(o->architecture_findings==0U&&o->high_risk==0U);}umi_studio_codeguard_destroy(g);return s;}
