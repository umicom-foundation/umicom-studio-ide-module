/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_polyglot_command_lifetime.c
 *
 * PURPOSE:
 *   Inject preview stage and allocation failures and verify client-owned values and the centre are always released.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/


#include "umicom/studio/compiler_centre.h"
#include "umicom/studio/compiler_export.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Intercept only this command translation unit. Real shared services still own
 * their internal allocations; the harness counts the command's three owners and
 * its actual created centre, including every early failure boundary. */
static unsigned fail_stage, fail_allocation, allocation_calls, live_values, live_centres;
static unsigned output_calls, report_calls, usage_calls, invalid_free;
static void *owned[3];
static char captured[32768];
static void *PreviewAllocate(size_t bytes)
{
    ++allocation_calls;
    if (allocation_calls == fail_allocation) return NULL;
    void *value = malloc(bytes);
    if (value != NULL && allocation_calls <= 3U) {
        owned[allocation_calls-1U] = value;
        ++live_values;
    }
    return value;
}
static void PreviewFree(void *value)
{
    if (value == NULL) return;
    for (size_t i = 0U; i < 3U; ++i) if (owned[i] == value) {
        owned[i] = NULL;
        --live_values;
        free(value);
        return;
    }
    ++invalid_free;
}
static UmiStatus PreviewTarget(UmiCompilerTarget *target)
{ return fail_stage == 1U ? UMI_STATUS_INVALID_STATE : umi_compiler_target_host(target); }
static UmiStatus PreviewCreate(UmiStudioToolchainCentre *tools,UmiStudioCompilerCentre **out)
{
    if (fail_stage == 2U) return UMI_STATUS_INVALID_STATE;
    UmiStatus status = umi_studio_compiler_centre_create(tools,out);
    if (status == UMI_STATUS_OK) ++live_centres;
    return status;
}
static void PreviewDestroy(UmiStudioCompilerCentre *centre)
{
    if (centre != NULL) { --live_centres; umi_studio_compiler_centre_destroy(centre); }
}
static UmiStatus PreviewDefaults(UmiStudioCompilerCentre *centre,const char *gcc,
    const char *clang,const char *rustc,const char *zig,const char *umicc,const char *target)
{
    return fail_stage == 3U ? UMI_STATUS_INVALID_STATE :
        umi_studio_compiler_centre_register_defaults(centre,gcc,clang,rustc,zig,umicc,target);
}
static UmiStatus PreviewManifest(UmiCompilerProjectManifest *manifest,const char *id,
    const char *name,const char *target)
{
    return fail_stage == 4U ? UMI_STATUS_INVALID_STATE :
        umi_compiler_manifest_init(manifest,id,name,target);
}
static UmiStatus PreviewUnit(UmiCompilerProjectManifest *manifest,const UmiCompilerUnitManifest *unit)
{ return fail_stage == 5U ? UMI_STATUS_INVALID_STATE : umi_compiler_manifest_add_unit(manifest,unit); }
static UmiStatus PreviewOpen(UmiStudioCompilerCentre *centre,const UmiCompilerProjectManifest *manifest)
{ return fail_stage == 6U ? UMI_STATUS_INVALID_STATE : umi_studio_compiler_centre_open_project(centre,manifest); }
static UmiStatus PreviewPlan(UmiStudioCompilerCentre *centre,const char *project,
    const char *unit,const char *directory,UmiCompilerPlan *plan)
{
    return fail_stage == 7U ? UMI_STATUS_INVALID_STATE :
        umi_studio_compiler_centre_plan_unit_in_directory(centre,project,unit,directory,plan);
}
static UmiStatus PreviewExport(const UmiCompilerCentre *centre,char *text,size_t capacity)
{ return fail_stage == 8U ? UMI_STATUS_INVALID_STATE : umi_studio_compiler_export_database(centre,text,capacity); }
static int PreviewPuts(const char *text)
{
    ++output_calls;
    if (fail_stage == 10U) return EOF;
    size_t length = strlen(text);
    if (length >= sizeof(captured)) return EOF;
    (void)memcpy(captured,text,length+1U);
    return 0;
}
static int PreviewFlush(FILE *stream)
{ (void)stream; return fail_stage == 11U ? EOF : 0; }
static int PreviewReport(FILE *stream,const char *format,...)
{ (void)stream;(void)format;++report_calls;return 0; }
static int PreviewUsage(const char *text,FILE *stream)
{ (void)text;(void)stream;++usage_calls;return 0; }

int StudioPolyglotPreviewMain(int argc,char **argv);
#define main StudioPolyglotPreviewMain
#define malloc PreviewAllocate
#define free PreviewFree
#define umi_compiler_target_host PreviewTarget
#define umi_studio_compiler_centre_create PreviewCreate
#define umi_studio_compiler_centre_destroy PreviewDestroy
#define umi_studio_compiler_centre_register_defaults PreviewDefaults
#define umi_compiler_manifest_init PreviewManifest
#define umi_compiler_manifest_add_unit PreviewUnit
#define umi_studio_compiler_centre_open_project PreviewOpen
#define umi_studio_compiler_centre_plan_unit_in_directory PreviewPlan
#define umi_studio_compiler_export_database PreviewExport
#define puts PreviewPuts
#define fflush PreviewFlush
#define fprintf PreviewReport
#define fputs PreviewUsage
#include "../src/tools/polyglot_compiler_main.c"
#undef main
#undef malloc
#undef free
#undef umi_compiler_target_host
#undef umi_studio_compiler_centre_create
#undef umi_studio_compiler_centre_destroy
#undef umi_studio_compiler_centre_register_defaults
#undef umi_compiler_manifest_init
#undef umi_compiler_manifest_add_unit
#undef umi_studio_compiler_centre_open_project
#undef umi_studio_compiler_centre_plan_unit_in_directory
#undef umi_studio_compiler_export_database
#undef puts
#undef fflush
#undef fprintf
#undef fputs

static void Reset(unsigned stage,unsigned allocation)
{
    fail_stage = stage;fail_allocation = allocation;allocation_calls = 0U;
    output_calls = 0U;report_calls = 0U;usage_calls = 0U;captured[0] = '\0';
}
#define CHECK(c) do { if (!(c)) { (void)fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#c); return 1; } } while (0)
#define RELEASED() (live_values == 0U && live_centres == 0U && invalid_free == 0U)
int main(void)
{
    char program[] = "preview", option[] = "--directory", directory[] = "reviewed project";
    char unknown[] = "--unknown";
    char *normal[] = {program,NULL};
    char *explicit_directory[] = {program,option,directory,NULL};
    char *invalid[] = {program,unknown,NULL};
    for (unsigned stage = 1U; stage <= 8U; ++stage) {
        Reset(stage,0U);
        CHECK(StudioPolyglotPreviewMain(1,normal) == (int)stage);
        CHECK(RELEASED() && output_calls == 0U && report_calls == 1U);
    }
    for (unsigned allocation = 1U; allocation <= 3U; ++allocation) {
        Reset(0U,allocation);
        CHECK(StudioPolyglotPreviewMain(1,normal) == 9);
        CHECK(RELEASED() && output_calls == 0U && report_calls == 1U);
    }
    for (unsigned stage = 10U; stage <= 11U; ++stage) {
        Reset(stage,0U);
        CHECK(StudioPolyglotPreviewMain(1,normal) == 10);
        CHECK(RELEASED() && output_calls == 1U && report_calls == 1U);
    }
    Reset(0U,0U);
    CHECK(StudioPolyglotPreviewMain(3,explicit_directory) == 0);
    CHECK(RELEASED() && output_calls == 1U && report_calls == 0U);
    CHECK(strstr(captured,"reviewed project") != NULL && strstr(captured,"\"gcc\"") != NULL);
    Reset(0U,0U);
    CHECK(StudioPolyglotPreviewMain(2,invalid) == 2);
    CHECK(RELEASED() && allocation_calls == 0U && output_calls == 0U && usage_calls == 1U);
    return 0;
}
