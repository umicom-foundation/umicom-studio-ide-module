/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_source_check_output.c
 *
 * PURPOSE:
 *   Inject command output failures without changing real file, document or compiler behavior.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned output_call, fail_call;
static int fail_flush;
static char diagnostic[1024];
static int TestPrintf(const char *format,...)
{
    if(++output_call==fail_call) return -1;
    va_list args; va_start(args,format);
    int result=vprintf(format,args); va_end(args); return result;
}
static int TestFprintf(FILE *stream,const char *format,...)
{
    if(stream==stdout && ++output_call==fail_call) return -1;
    va_list args; va_start(args,format);
    int result=stream==stderr ? vsnprintf(diagnostic,sizeof diagnostic,format,args)
                              : vfprintf(stream,format,args);
    va_end(args); return result;
}
static int TestFlush(FILE *stream)
{
    return stream==stdout && fail_flush ? EOF : fflush(stream);
}
/* The command is the production source. Only output failures are injected;
 * file reading, document ownership and compilation use their actual services. */
#define printf TestPrintf
#define fprintf TestFprintf
#define fflush TestFlush
#define main SourceCheckMain
#include "../src/tools/source_check_main.c"
#undef main
#undef fflush
#undef fprintf
#undef printf
int main(int argc,char **argv)
{
    if(argc!=3) return 2;
    fail_flush=strcmp(argv[1],"flush")==0;
    fail_call=fail_flush ? 0U : 1U;
    char *arguments[]={"source-check",argv[2],NULL};
    int result=SourceCheckMain(2,arguments);
    if(result!=1 || strstr(diagnostic,umi_status_text(UMI_STATUS_IO_ERROR))==NULL) {
        fprintf(stderr,"Expected I/O failure; got %d: %s\n",result,diagnostic);
        return 1;
    }
    return 0;
}
