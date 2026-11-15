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
#include "umicom/platform/native_arguments.h"
#include <stdio.h>
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
/* The previous entry ignored report output status and used narrow CRT
 * arguments on Windows. Retain it for review. Framework now captures native
 * UTF-8 arguments; report failure takes precedence over the severity gate. */
#if 0
int main(int argc,char **argv){const char *root=argc>1?argv[1]:".";UmiStudioCodeGuard *g=NULL;UmiCodeGuardProfile p=umi_codeguard_profile_default();/* Preserve the original failure result so the caller can respond to the correct cause. */ if(umi_studio_codeguard_create(root,&g)!=UMI_STATUS_OK)return 2;/* Preserve the original failure result so the caller can respond to the correct cause. */ if(umi_studio_codeguard_scan(g,&p)!=UMI_STATUS_OK){umi_studio_codeguard_destroy(g);return 3;}(void)umi_codeguard_report_write(stdout,UMI_CODEGUARD_REPORT_TEXT,umi_studio_codeguard_result(g));UmiCodeGuardSummary s=umi_studio_codeguard_summary(g);umi_studio_codeguard_destroy(g);return umi_codeguard_summary_failed(&s,p.fail_on)?1:0;}

#endif

static int StudioCodeGuardRun(int argc,char **argv)
{
    if(argc<1 || argv==NULL || (argc>1 && argv[1]==NULL))return 2;
    /* Preserve the established positional-root/default-profile invocation.
     * This entry does not acquire a separate scanner or duplicate report logic. */
    const char *root=argc>1?argv[1]:".";
    UmiStudioCodeGuard *guard=NULL;
    UmiCodeGuardProfile profile=umi_codeguard_profile_default();
    UmiStatus status=umi_studio_codeguard_create(root,&guard);
    if(status!=UMI_STATUS_OK){
        fprintf(stderr,"Cannot create CodeGuard session: %s\n",umi_status_text(status));
        return 2;
    }
    status=umi_studio_codeguard_scan(guard,&profile);
    if(status==UMI_STATUS_OK)
        status=umi_codeguard_report_write(stdout,UMI_CODEGUARD_REPORT_TEXT,
            umi_studio_codeguard_result(guard));
    UmiCodeGuardSummary summary=umi_studio_codeguard_summary(guard);
    umi_studio_codeguard_destroy(guard);
    if(status!=UMI_STATUS_OK){
        fprintf(stderr,"CodeGuard analysis or report output failed: %s\n",umi_status_text(status));
        return 3;
    }
    return umi_codeguard_summary_failed(&summary,profile.fail_on)?1:0;
}

int main(int argc,char **argv)
{
    int result=2;
    UmiStatus status=UmiNativeArgumentsDispatch(argc,argv,StudioCodeGuardRun,&result);
    if(status!=UMI_STATUS_OK){
        fprintf(stderr,"Cannot capture native CodeGuard arguments: %s\n",umi_status_text(status));
        return 2;
    }
    return result;
}
