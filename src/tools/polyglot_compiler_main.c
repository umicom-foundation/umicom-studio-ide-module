/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/tools/polyglot_compiler_main.c
 *
 * PURPOSE:
 *   Provide the polyglot compiler main development utility for
 *   Umicom Studio IDE.
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
/* Replace large automatic preview values and early leaking exits with explicit owners and a common cleanup path. Preserve no-argument preview behavior and prior stage exit codes while allowing an explicit metadata directory. The prior command remains for review. The previous implementation is retained for review. */
#if 0
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{ UmiStudioCompilerCentre *centre = NULL; UmiCompilerProjectManifest manifest; UmiCompilerUnitManifest unit = {0}; UmiCompilerPlan plan; UmiCompilerCentre *engine; char database[32768]; UmiCompilerTarget target; /* Preserve the original failure result so the caller can respond to the correct cause. */ if (umi_compiler_target_host(&target) != UMI_STATUS_OK) return 1; /* Preserve the original failure result so the caller can respond to the correct cause. */ if (umi_studio_compiler_centre_create(NULL,&centre) != UMI_STATUS_OK) return 2; /* Preserve the original failure result so the caller can respond to the correct cause. */ if (umi_studio_compiler_centre_register_defaults(centre,"gcc","clang","rustc","zig","",target.triple) != UMI_STATUS_OK) return 3; /* Preserve the original failure result so the caller can respond to the correct cause. */ if (umi_compiler_manifest_init(&manifest,"polyglot-demo","Polyglot Demo",target.triple) != UMI_STATUS_OK) return 4; (void)snprintf(unit.unit_id,sizeof(unit.unit_id),"main"); (void)snprintf(unit.source,sizeof(unit.source),"src/main.c"); (void)snprintf(unit.output,sizeof(unit.output),"obj/main.o"); unit.language = UMI_COMPILER_LANGUAGE_C; unit.entry_point = true; /* Preserve the original failure result so the caller can respond to the correct cause. */ if (umi_compiler_manifest_add_unit(&manifest,&unit) != UMI_STATUS_OK) return 5; /* Preserve the original failure result so the caller can respond to the correct cause. */ if (umi_studio_compiler_centre_open_project(centre,&manifest) != UMI_STATUS_OK) return 6; /* Preserve the original failure result so the caller can respond to the correct cause. */ if (umi_studio_compiler_centre_plan_unit(centre,"polyglot-demo","main",&plan) != UMI_STATUS_OK) return 7; engine = umi_studio_compiler_centre_engine(centre); /* Preserve the original failure result so the caller can respond to the correct cause. */ if (umi_studio_compiler_export_database(engine,database,sizeof(database)) != UMI_STATUS_OK) return 8; (void)puts(database); umi_studio_compiler_centre_destroy(centre); return 0; }

#endif
/* This command previews metadata only. Large inline values belong to explicit
 * heap owners, and every exit after acquisition passes through one cleanup path.
 * Argument planning and JSON export remain shared Framework/Studio services. */
int main(int argc,char **argv)
{
    const char *directory = ".";
    if (argc == 3 && strcmp(argv[1],"--directory") == 0 && argv[2][0] != '\0')
        directory = argv[2];
    else if (argc != 1) {
        (void)fputs("Usage: umicom-studio-polyglot-compiler [--directory PATH]\n",stderr);
        return 2;
    }
    UmiCompilerTarget target;
    UmiStatus status = umi_compiler_target_host(&target);
    if (status != UMI_STATUS_OK) {
        (void)fprintf(stderr,"Compiler preview target refused (status %d).\n",(int)status);
        return 1;
    }
    UmiStudioCompilerCentre *centre = NULL;
    UmiCompilerProjectManifest *manifest = malloc(sizeof(*manifest));
    UmiCompilerPlan *plan = malloc(sizeof(*plan));
    char *database = malloc(32768U);
    UmiCompilerUnitManifest unit = {0};
    int result = 9;
    const char *stage = "allocation";
    if (manifest == NULL || plan == NULL || database == NULL) {
        status = UMI_STATUS_OUT_OF_MEMORY;
        goto cleanup;
    }
    stage = "centre creation"; result = 2;
    status = umi_studio_compiler_centre_create(NULL,&centre);
    if (status != UMI_STATUS_OK) goto cleanup;
    stage = "provider registration"; result = 3;
    status = umi_studio_compiler_centre_register_defaults(
        centre,"gcc","clang","rustc","zig","",target.triple);
    if (status != UMI_STATUS_OK) goto cleanup;
    stage = "manifest initialization"; result = 4;
    status = umi_compiler_manifest_init(manifest,"polyglot-demo","Polyglot Demo",target.triple);
    if (status != UMI_STATUS_OK) goto cleanup;
    (void)strcpy(unit.unit_id,"main");
    (void)strcpy(unit.source,"src/main.c");
    (void)strcpy(unit.output,"obj/main.o");
    unit.language = UMI_COMPILER_LANGUAGE_C;
    unit.entry_point = true;
    stage = "unit insertion"; result = 5;
    status = umi_compiler_manifest_add_unit(manifest,&unit);
    if (status != UMI_STATUS_OK) goto cleanup;
    stage = "project publication"; result = 6;
    status = umi_studio_compiler_centre_open_project(centre,manifest);
    if (status != UMI_STATUS_OK) goto cleanup;
    stage = "unit planning"; result = 7;
    status = umi_studio_compiler_centre_plan_unit_in_directory(
        centre,"polyglot-demo","main",directory,plan);
    if (status != UMI_STATUS_OK) goto cleanup;
    stage = "database export"; result = 8;
    status = umi_studio_compiler_export_database(
        umi_studio_compiler_centre_engine(centre),database,32768U);
    if (status != UMI_STATUS_OK) goto cleanup;
    /* A failed output stream is a failed command even when planning succeeded.
     * The stream may contain a partial document; consumers must inspect exit status. */
    stage = "standard output"; result = 10;
    if (puts(database) == EOF || fflush(stdout) == EOF) {
        status = UMI_STATUS_IO_ERROR;
        goto cleanup;
    }
    result = 0;
cleanup:
    if (result != 0)
        (void)fprintf(stderr,"Compiler preview %s refused (status %d).\n",stage,(int)status);
    umi_studio_compiler_centre_destroy(centre);
    free(database);
    free(plan);
    free(manifest);
    return result;
}
