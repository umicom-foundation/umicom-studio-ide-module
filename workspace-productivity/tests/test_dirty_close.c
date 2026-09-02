/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/workspace-productivity/tests/test_dirty_close.c
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
#include "umicom/studio/document_set.h"
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void){
    UmiStudioDocumentSet s;
    umi_studio_document_set_init(&s);
    assert(umi_studio_document_set_open(&s,"a.c","C",false)==UMI_STATUS_OK);
    assert(umi_studio_document_set_mark_dirty(&s,"a.c",true)==UMI_STATUS_OK);
    assert(umi_studio_document_set_close(&s,"a.c",false)==UMI_STATUS_BUSY);
    assert(umi_studio_document_set_close(&s,"a.c",true)==UMI_STATUS_OK);
    return 0;
}
