/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/tools/compiler_centre_main.c
 *
 * PURPOSE:
 *   Provide the compiler centre main development utility for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/compiler_catalogue.h"
#include "umicom/studio/compiler_centre.h"
#include "umicom/studio/compiler_export.h"
#include <stdio.h>
#include <stdlib.h>
/* Ignored projection/formatting failures and a profile-id/provider-id mismatch are replaced by checked complete views and actual active-provider selection. Keep the prior native output flow for review. The previous implementation is retained for review. */
#if 0
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(int argc,char **argv)
{ UmiStudioCompilerCentre *centre = NULL; UmiStudioCompilerCentreSnapshot snapshot; UmiStudioCompilerCatalogue catalogue; UmiCompilerCentre *engine; char summary[2048]; size_t index; const char *gcc = argc > 1 ? argv[1] : "gcc"; const char *clang = argc > 2 ? argv[2] : "clang"; const char *umicc = argc > 3 ? argv[3] : ""; /* Keep the operation inside its valid bounds before reading, writing or adding data. */ if (umi_studio_compiler_centre_create(NULL,&centre) != UMI_STATUS_OK) return 1; /* Keep the operation inside its valid bounds before reading, writing or adding data. */ if (umi_studio_compiler_centre_register_defaults(centre,gcc,clang,"rustc","zig",umicc,NULL) != UMI_STATUS_OK) { umi_studio_compiler_centre_destroy(centre); return 2; } /* Keep the operation inside its valid bounds before reading, writing or adding data. */ if (umi_studio_compiler_centre_snapshot(centre,&snapshot) != UMI_STATUS_OK) { umi_studio_compiler_centre_destroy(centre); return 3; } engine = umi_studio_compiler_centre_engine(centre); (void)umi_studio_compiler_catalogue_project(&engine->providers,snapshot.compiler.active_profile_id,&catalogue); (void)umi_studio_compiler_export_summary(&snapshot.compiler,summary,sizeof(summary)); (void)puts(summary); /* Visit each bounded item once so every record receives the same rule. */ for (index = 0U; index < catalogue.count; ++index) (void)printf("%-10s %-12s %-24s %s\n",catalogue.rows[index].provider_id,catalogue.rows[index].family,catalogue.rows[index].languages,catalogue.rows[index].available ? "available" : "missing"); umi_studio_compiler_centre_destroy(centre); return 0; }

#endif
/* Build complete views before printing and keep the acquired centre/catalogue
 * on one cleanup path. Driver names remain metadata; this command runs no probe. */
int main(int argc,char **argv)
{
    if (argc > 4) {
        (void)fputs("Usage: umicom-studio-compiler-centre [GCC [CLANG [UMICC]]]\n",stderr);
        return 8;
    }
    const char *gcc = argc > 1 ? argv[1] : "gcc";
    const char *clang = argc > 2 ? argv[2] : "clang";
    const char *umicc = argc > 3 ? argv[3] : "";
    UmiStudioCompilerCentre *centre = NULL;
    UmiStudioCompilerCatalogue *catalogue = malloc(sizeof(*catalogue));
    UmiStudioCompilerCentreSnapshot snapshot;
    char summary[2048];
    int result = 6;
    const char *stage = "allocation";
    UmiStatus status = UMI_STATUS_OUT_OF_MEMORY;
    if (catalogue == NULL) goto cleanup;
    stage = "centre creation"; result = 1;
    status = umi_studio_compiler_centre_create(NULL,&centre);
    if (status != UMI_STATUS_OK) goto cleanup;
    stage = "provider registration"; result = 2;
    status = umi_studio_compiler_centre_register_defaults(centre,gcc,clang,"rustc","zig",umicc,NULL);
    if (status != UMI_STATUS_OK) goto cleanup;
    stage = "snapshot"; result = 3;
    status = umi_studio_compiler_centre_snapshot(centre,&snapshot);
    if (status != UMI_STATUS_OK) goto cleanup;
    stage = "catalogue"; result = 4;
    UmiCompilerCentre *engine = umi_studio_compiler_centre_engine(centre);
    const UmiCompilerProfile *active = umi_compiler_centre_active_profile(engine);
    if (active == NULL) { status = UMI_STATUS_INVALID_STATE; goto cleanup; }
    status = umi_studio_compiler_catalogue_project(&engine->providers,active->provider_id,catalogue);
    if (status != UMI_STATUS_OK) goto cleanup;
    stage = "summary"; result = 5;
    status = umi_studio_compiler_export_summary(&snapshot.compiler,summary,sizeof(summary));
    if (status != UMI_STATUS_OK) goto cleanup;
    stage = "standard output"; result = 7;status = UMI_STATUS_IO_ERROR;
    if (puts(summary) == EOF) goto cleanup;
    for (size_t index = 0U; index < catalogue->count; ++index) {
        const UmiStudioCompilerCatalogueRow *row = &catalogue->rows[index];
        if (printf("%-10s %-12s %-24s %s%s\n",row->provider_id,row->family,
                   row->languages,row->available ? "available" : "missing",
                   row->selected ? " selected" : "") < 0) goto cleanup;
    }
    if (fflush(stdout) == EOF) goto cleanup;
    result = 0;
cleanup:
    if (result != 0)
        (void)fprintf(stderr,"Compiler inspection %s refused (status %d).\n",stage,(int)status);
    umi_studio_compiler_centre_destroy(centre);
    free(catalogue);
    return result;
}
