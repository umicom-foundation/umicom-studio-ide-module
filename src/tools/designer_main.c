/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/designer_main.c
 *
 * PURPOSE:
 *   Provide a native designer smoke command for semantic add, property, undo, redo and preview operations.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This command-line entry point parses user arguments, calls the shared Studio
 * service and reports the result; business logic stays in reusable services.
 */
#include "umicom/studio/services.h"
#include "umicom/studio/designer_state.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(int argc,char **argv)
{
    UmiStudioDeclarative *declarative=NULL;UmiStudioDesigner *designer=NULL;UmiDeclDiagnosticList diagnostics;char report[4096];char preview[4096];UmiStatus status;
    (void)argc;(void)argv;
    status=umi_studio_declarative_create(&declarative);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if(status==UMI_STATUS_OK)status=umi_studio_designer_create(declarative,&designer);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if(status==UMI_STATUS_OK)status=umi_studio_designer_add_component(designer,"editor","editor","root");
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if(status==UMI_STATUS_OK)status=umi_studio_designer_set_property(designer,"editor","title","Source Editor");
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if(status==UMI_STATUS_OK)status=umi_studio_designer_state_report(designer,report,sizeof(report));
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if(status==UMI_STATUS_OK)status=umi_studio_designer_preview(designer,preview,sizeof(preview),&diagnostics);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if(status==UMI_STATUS_OK){fputs(report,stdout);puts("Preview:");fputs(preview,stdout);}
    umi_studio_designer_destroy(designer);umi_studio_declarative_destroy(declarative);return status==UMI_STATUS_OK?EXIT_SUCCESS:EXIT_FAILURE;
}
