/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/workspace-productivity/tests/test_editor_status.c
 *
 * PURPOSE:
 *   Exercise one workspace/editor productivity behaviour.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*
 * This file is part of Studio workspace/editor productivity. The implementation
 * uses bounded C23 data structures and explicit ownership so the behaviour is
 * easy to inspect, test and later connect to the main Studio workbench.
 */

#include <assert.h>
#include <string.h>
#include "umicom/studio/editor_status.h"
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void){
    UmiStudioEditorStatus s;
    char text[256];
    umi_studio_editor_status_init(&s);
    s.line=42U;s.column=7U;
    assert(umi_studio_editor_status_format(&s,text,sizeof(text))==UMI_STATUS_OK);
    assert(strstr(text,"Ln 42, Col 7")!=NULL);
    assert(strstr(text,"UTF-8")!=NULL);
    return 0;
}
