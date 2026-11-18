/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/tools/source_check_main.c
 *
 * PURPOSE:
 *   Check a bounded selected local source file through shared Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/


#include "umicom/studio/source_check.h"
#include "umicom/platform/input_file.h"
#include <stdio.h>
#include <string.h>
static int Usage(FILE *stream)
{
    return fputs("Umicom Studio source check\nUsage: umicom-studio-source-check ABSOLUTE_FILE.uc\n"
        "Checks the bounded Umicom C subset without executing or saving code.\n"
        "Exit 0: accepted (warnings allowed); 2: compiler refusal; 1: input/output failure; 64: usage.\n",stream)==EOF ? 1 : 0;
}
int main(int argc,char **argv)
{
    if(argc==2 && strcmp(argv[1],"--help")==0) {
        if(Usage(stdout)!=0 || fflush(stdout)==EOF) return 1;
        return 0;
    }
    if(argc!=2) { (void)Usage(stderr); return 64; }
    unsigned char *bytes=NULL; size_t length=0U;
    UmiDocumentStore *store=NULL; UmiDocumentId id=0U;
    UmiStudioSourceCheck *check=NULL;
    int exit_code=1;
    UmiStatus status=UmiInputFileRead(argv[1],UMC_SOURCE_LIMIT,&bytes,&length);
    if(status!=UMI_STATUS_OK) goto done;
    status=umi_document_store_create(&store);
    if(status!=UMI_STATUS_OK) goto done;
    status=umi_document_store_create_loaded(store,"source.uc",argv[1],(const char *)bytes,length,&id);
    UmiInputFileFree(bytes); bytes=NULL;
    if(status!=UMI_STATUS_OK) goto done;
    status=umi_studio_source_check_create(store,&check);
    if(status!=UMI_STATUS_OK) goto done;
    status=umi_studio_source_check_run(check,id);
    const UmiCvmDocumentCheckResult *current=umi_studio_source_check_current(check);
/* Preserve the actual failing stage status so command errors never report an unrelated earlier success. The former implementation is retained for owner review. */
#if 0
    if(current==NULL) goto done;
#endif
    if(current==NULL) { if(status==UMI_STATUS_OK) status=UMI_STATUS_INVALID_STATE; goto done; }
    UmiDiagnosticSnapshot row;
    if(status!=UMI_STATUS_OK) {
/* Preserve the actual failing stage status so command errors never report an unrelated earlier success. The former implementation is retained for owner review. */
#if 0
        if(umi_studio_source_check_error(check,&row)!=UMI_STATUS_OK) goto done;
#endif
        status=umi_studio_source_check_error(check,&row);
        if(status!=UMI_STATUS_OK) goto done;
/* Retain output failure as an I/O status through common cleanup. The former implementation is retained for owner review. */
#if 0
        if(fprintf(stdout,"%s:%u:%u: error: %s\n",row.uri,row.line,row.column,row.message)<0) goto done;
#endif
        if(fprintf(stdout,"%s:%u:%u: error: %s\n",row.uri,row.line,row.column,row.message)<0) { status=UMI_STATUS_IO_ERROR; goto done; }
        exit_code=2;
    } else {
        for(size_t i=0U;i<current->source.report.warning_count;++i) {
/* Preserve the actual failing stage status so command errors never report an unrelated earlier success. The former implementation is retained for owner review. */
#if 0
            if(umi_studio_source_check_warning(check,i,&row)!=UMI_STATUS_OK) goto done;
#endif
            status=umi_studio_source_check_warning(check,i,&row);
            if(status!=UMI_STATUS_OK) goto done;
/* Retain output failure as an I/O status through common cleanup. The former implementation is retained for owner review. */
#if 0
            if(fprintf(stdout,"%s:%u:%u: warning: %s [%s]\n",row.uri,row.line,row.column,row.message,row.code)<0) goto done;
#endif
            if(fprintf(stdout,"%s:%u:%u: warning: %s [%s]\n",row.uri,row.line,row.column,row.message,row.code)<0) { status=UMI_STATUS_IO_ERROR; goto done; }
        }
/* Retain output failure as an I/O status through common cleanup. The former implementation is retained for owner review. */
#if 0
        if(printf("Umicom C subset: %zu warning(s); %zu instructions checked, not executed.\n",
            current->source.report.warning_count,current->source.report.instruction_count)<0) goto done;
#endif
        if(printf("Umicom C subset: %zu warning(s); %zu instructions checked, not executed.\n",
            current->source.report.warning_count,current->source.report.instruction_count)<0) { status=UMI_STATUS_IO_ERROR; goto done; }
        exit_code=0;
    }
/* Preserve the actual failing stage status so command errors never report an unrelated earlier success. The former implementation is retained for owner review. */
#if 0
    if(fflush(stdout)==EOF) exit_code=1;
#endif
    if(fflush(stdout)==EOF) { exit_code=1; status=UMI_STATUS_IO_ERROR; }
done:
    if(exit_code==1)
        (void)fprintf(stderr,"Source check could not complete: %s.\n",umi_status_text(status));
    umi_studio_source_check_destroy(check);
    umi_document_store_destroy(store);
    UmiInputFileFree(bytes);
    return exit_code;
}
