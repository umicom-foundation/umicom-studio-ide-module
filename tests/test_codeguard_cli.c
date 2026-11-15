/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_codeguard_cli.c
 *
 * PURPOSE:
 *   Verify CLI report failures, severity gates and owned-session cleanup.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif
/* Compile the production entry directly so the fixture can also exercise its
 * internal callback with synthetic arguments. Native capture belongs only at
 * the real process boundary, covered separately by CTest process cases. */
#define main StudioCodeGuardProcessEntry
#include "../src/tools/codeguard_main.c"
#undef main
#include "studio_test_state.h"
#include <string.h>
#ifdef _WIN32
#include <io.h>
#define TEST_DUP _dup
#define TEST_DUP2 _dup2
#define TEST_FILENO _fileno
#define TEST_CLOSE _close
#else
#include <unistd.h>
#define TEST_DUP dup
#define TEST_DUP2 dup2
#define TEST_FILENO fileno
#define TEST_CLOSE close
#endif
#define CHECK(c) do {if(!(c)){fprintf(stderr,"%d: %s\n",__LINE__,#c);failed=1;goto cleanup;}}while(0)

#ifdef UMICOM_STUDIO_CODEGUARD_CLI_FAULTS
static const char *fault;
static unsigned creates,scans,reports,destroys,dispatches;
UmiStatus __real_umi_studio_codeguard_create(const char *,UmiStudioCodeGuard **);
UmiStatus __real_umi_studio_codeguard_scan(UmiStudioCodeGuard *,const UmiCodeGuardProfile *);
UmiStatus __real_umi_codeguard_report_write(FILE *,UmiCodeGuardReportFormat,const UmiCodeGuardResult *);
void __real_umi_studio_codeguard_destroy(UmiStudioCodeGuard *);
UmiStatus __wrap_umi_studio_codeguard_create(const char *root,UmiStudioCodeGuard **out)
{
    ++creates;
    if(strcmp(fault,"create")==0){*out=NULL;return UMI_STATUS_OUT_OF_MEMORY;}
    return __real_umi_studio_codeguard_create(root,out);
}
UmiStatus __wrap_umi_studio_codeguard_scan(UmiStudioCodeGuard *guard,const UmiCodeGuardProfile *profile)
{
    ++scans;
    if(strcmp(fault,"scan")==0)return UMI_STATUS_IO_ERROR;
    return __real_umi_studio_codeguard_scan(guard,profile);
}
UmiStatus __wrap_umi_codeguard_report_write(FILE *stream,UmiCodeGuardReportFormat format,
    const UmiCodeGuardResult *result)
{
    ++reports;
    if(strncmp(fault,"report",6U)==0)return UMI_STATUS_IO_ERROR;
    return __real_umi_codeguard_report_write(stream,format,result);
}
void __wrap_umi_studio_codeguard_destroy(UmiStudioCodeGuard *guard)
{
    ++destroys;
    __real_umi_studio_codeguard_destroy(guard);
}
UmiStatus __wrap_UmiNativeArgumentsDispatch(int argc,char **argv,
    UmiNativeMainFunction function,int *out_exit_code)
{
    (void)argc;(void)argv;(void)function;(void)out_exit_code;
    ++dispatches;
    /* A capture error must preserve the entry's failure default without
     * invoking its scanner callback. This shim is not a Unicode capture test. */
    return UMI_STATUS_INVALID_ARGUMENT;
}
#endif

int main(int argc,char **argv)
{
    if(argc!=2)return 2;
    const char *mode=argv[1];
    int failed=0,expected=0,actual=-1,saved=-1,redirected=0;
    StudioTestState state={0};
    FILE *capture=NULL;
    char *root=NULL,*source=NULL,*output=NULL,*text=NULL;
    gsize length=0U;
    int finding=strstr(mode,"finding")!=NULL;
    int readonly=strncmp(mode,"stdout-",7U)==0;
    CHECK(StudioTestStateBegin(&state));
    root=g_build_filename(state.directory,strcmp(mode,"unicode")==0?"review-\xC3\xA9-\xE6\x97\xA5":"review",NULL);
    output=g_build_filename(state.directory,"captured-output.txt",NULL);
    CHECK(root!=NULL && output!=NULL && g_mkdir(root,0700)==0);
    if(finding) {
        source=g_build_filename(root,"unsafe.c",NULL);
        CHECK(source!=NULL && g_file_set_contents(source,
            "void copy(char *destination,const char *source) { strcpy(destination,source); }\n",-1,NULL));
    }
    char *args[4]={"umicom-studio-codeguard",root,"ignored-compatible-argument",NULL};
    int count=2;
    if(strcmp(mode,"default-root")==0){count=1;CHECK(g_chdir(root)==0);}
    else if(strcmp(mode,"extra-arguments")==0)count=3;
    else if(strcmp(mode,"missing-root")==0){
        g_free(root);root=g_build_filename(state.directory,"not-present",NULL);
        CHECK(root!=NULL);args[1]=root;expected=3;
    }else if(strcmp(mode,"empty-root")==0){args[1]="";expected=2;}
    else if(strcmp(mode,"bad-count")==0){count=0;expected=2;}
    else if(strcmp(mode,"null-vector")==0)expected=2;
    else if(strcmp(mode,"null-root")==0){args[1]=NULL;expected=2;}
    else if(finding)expected=1;
    if(readonly)expected=3;
#ifdef UMICOM_STUDIO_CODEGUARD_CLI_FAULTS
    fault=mode;
    expected=strcmp(mode,"create")==0 || strcmp(mode,"arguments")==0?2:3;
#endif
    if(readonly)CHECK(g_file_set_contents(output,"read-only fixture",-1,NULL));
    capture=g_fopen(output,readonly?"rb":"wb");
    CHECK(capture!=NULL && fflush(stdout)==0);
    saved=TEST_DUP(TEST_FILENO(stdout));CHECK(saved>=0);
    CHECK(TEST_DUP2(TEST_FILENO(capture),TEST_FILENO(stdout))>=0);redirected=1;
#ifdef UMICOM_STUDIO_CODEGUARD_CLI_FAULTS
    if(strcmp(mode,"arguments")==0)actual=StudioCodeGuardProcessEntry(count,args);
    else
#endif
        actual=StudioCodeGuardRun(count,strcmp(mode,"null-vector")==0?NULL:args);
    /* Restore output even when a check fails; leave the owned fixture on disk. */
    (void)fflush(stdout);
    CHECK(TEST_DUP2(saved,TEST_FILENO(stdout))>=0);redirected=0;
    clearerr(stdout);
    int close_status=fclose(capture);capture=NULL;
    CHECK(close_status==0);
    CHECK(actual==expected);
#ifdef UMICOM_STUDIO_CODEGUARD_CLI_FAULTS
    if(strcmp(mode,"arguments")==0)
        CHECK(dispatches==1U && creates==0U && scans==0U && reports==0U && destroys==0U);
    else if(strcmp(mode,"create")==0)
        CHECK(creates==1U && scans==0U && reports==0U && destroys==0U);
    else if(strcmp(mode,"scan")==0)
        CHECK(creates==1U && scans==1U && reports==0U && destroys==1U);
    else CHECK(creates==1U && scans==1U && reports==1U && destroys==1U);
#endif
    CHECK(g_file_get_contents(output,&text,&length,NULL));
    if(readonly)CHECK(strcmp(text,"read-only fixture")==0);
    else if(expected==0 || expected==1) {
        CHECK(length>0U);
        if(finding)CHECK(strstr(text,"CODEGUARD-C-BUF-001")!=NULL);
    }else CHECK(length==0U);
cleanup:
    if(redirected){(void)fflush(stdout);(void)TEST_DUP2(saved,TEST_FILENO(stdout));clearerr(stdout);}
    if(saved>=0)(void)TEST_CLOSE(saved);
    if(capture!=NULL)(void)fclose(capture);
    g_free(text);g_free(source);g_free(output);g_free(root);
    if(!StudioTestStateEnd(&state))failed=1;
    return failed;
}
