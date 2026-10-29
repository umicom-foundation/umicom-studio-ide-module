/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_state_isolation.c
 *
 * PURPOSE:
 *   Verify state isolation preserves caller files and restores process context.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "studio_test_state.h"
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#x); result=1; goto done; } } while (0)
static int Matches(const char *expected)
{
    const char *actual=g_getenv("UMICOM_STUDIO_DATA_PATH");
    return expected==NULL ? actual==NULL : actual!=NULL && strcmp(actual,expected)==0;
}
int main(int argc,char **argv)
{
    if(argc!=2)return 2;
    const char *mode=argv[1];
    int result=0,first=0,second=0;
    StudioTestState caller={0},child={0},nested={0};
    char *marker=NULL,*text=NULL,*first_path=NULL,*cwd=NULL;
    CHECK(StudioTestStateBegin(&caller));first=1;
    marker=g_build_filename(caller.directory,".umicom","owner-marker",NULL);
    CHECK(g_file_set_contents(marker,"Caller state must survive.\n",-1,NULL));
    const char *expected=NULL;
    if(strcmp(mode,"existing")==0 || strcmp(mode,"nested")==0 || strcmp(mode,"repeat")==0)
        expected=caller.directory;
    else if(strcmp(mode,"empty")==0)expected="";
    else CHECK(strcmp(mode,"unset")==0);
    if(expected!=NULL)CHECK(g_setenv("UMICOM_STUDIO_DATA_PATH",expected,TRUE));
    else g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    CHECK(StudioTestStateBegin(&child));second=1;
    CHECK(g_getenv("UMICOM_STUDIO_DATA_PATH")==NULL);
    CHECK(strcmp(child.directory,caller.directory)!=0);
    first_path=g_strdup(child.directory);
    cwd=g_get_current_dir();CHECK(strcmp(cwd,child.directory)==0);g_clear_pointer(&cwd,g_free);
    CHECK(!g_file_test(".umicom/owner-marker",G_FILE_TEST_EXISTS));
    CHECK(g_file_set_contents(".umicom/child-marker","Child state is retained.\n",-1,NULL));
    if(strcmp(mode,"nested")==0) {
        CHECK(StudioTestStateBegin(&nested));
        CHECK(strcmp(nested.directory,child.directory)!=0);
        CHECK(!g_file_test(".umicom/child-marker",G_FILE_TEST_EXISTS));
        CHECK(StudioTestStateEnd(&nested));
        cwd=g_get_current_dir();CHECK(strcmp(cwd,child.directory)==0);g_clear_pointer(&cwd,g_free);
        CHECK(g_getenv("UMICOM_STUDIO_DATA_PATH")==NULL);
    }
    CHECK(StudioTestStateEnd(&child));second=0;
    CHECK(Matches(expected));
    cwd=g_get_current_dir();CHECK(strcmp(cwd,caller.directory)==0);g_clear_pointer(&cwd,g_free);
    CHECK(g_file_get_contents(marker,&text,NULL,NULL));
    CHECK(strcmp(text,"Caller state must survive.\n")==0);g_clear_pointer(&text,g_free);
    if(strcmp(mode,"repeat")==0) {
        CHECK(StudioTestStateBegin(&child));second=1;
        CHECK(strcmp(child.directory,first_path)!=0);
        CHECK(!g_file_test(".umicom/child-marker",G_FILE_TEST_EXISTS));
        CHECK(StudioTestStateEnd(&child));second=0;
        CHECK(Matches(expected));
    }
    char *retained=g_build_filename(first_path,".umicom","child-marker",NULL);
    int kept=g_file_get_contents(retained,&text,NULL,NULL);g_free(retained);
    CHECK(kept && strcmp(text,"Child state is retained.\n")==0);
done:
    if(nested.changed && !StudioTestStateEnd(&nested))result=1;
    if(second && !StudioTestStateEnd(&child))result=1;
    if(first && !StudioTestStateEnd(&caller))result=1;
    g_free(marker);g_free(text);g_free(first_path);g_free(cwd);
    return result;
}
