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
/* Umicom Studio IDE | Polyglot compiler command | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/studio/compiler_centre.h"
#include "umicom/studio/compiler_export.h"
#include <stdio.h>
#include <string.h>
/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{ UmiStudioCompilerCentre *centre = NULL; UmiCompilerProjectManifest manifest; UmiCompilerUnitManifest unit = {0}; UmiCompilerPlan plan; UmiCompilerCentre *engine; char database[32768]; UmiCompilerTarget target; /* Preserve the original failure result so the caller can respond to the correct cause. */ if (umi_compiler_target_host(&target) != UMI_STATUS_OK) return 1; /* Preserve the original failure result so the caller can respond to the correct cause. */ if (umi_studio_compiler_centre_create(NULL,&centre) != UMI_STATUS_OK) return 2; /* Preserve the original failure result so the caller can respond to the correct cause. */ if (umi_studio_compiler_centre_register_defaults(centre,"gcc","clang","rustc","zig","",target.triple) != UMI_STATUS_OK) return 3; /* Preserve the original failure result so the caller can respond to the correct cause. */ if (umi_compiler_manifest_init(&manifest,"polyglot-demo","Polyglot Demo",target.triple) != UMI_STATUS_OK) return 4; (void)snprintf(unit.unit_id,sizeof(unit.unit_id),"main"); (void)snprintf(unit.source,sizeof(unit.source),"src/main.c"); (void)snprintf(unit.output,sizeof(unit.output),"obj/main.o"); unit.language = UMI_COMPILER_LANGUAGE_C; unit.entry_point = true; /* Preserve the original failure result so the caller can respond to the correct cause. */ if (umi_compiler_manifest_add_unit(&manifest,&unit) != UMI_STATUS_OK) return 5; /* Preserve the original failure result so the caller can respond to the correct cause. */ if (umi_studio_compiler_centre_open_project(centre,&manifest) != UMI_STATUS_OK) return 6; /* Preserve the original failure result so the caller can respond to the correct cause. */ if (umi_studio_compiler_centre_plan_unit(centre,"polyglot-demo","main",&plan) != UMI_STATUS_OK) return 7; engine = umi_studio_compiler_centre_engine(centre); /* Preserve the original failure result so the caller can respond to the correct cause. */ if (umi_studio_compiler_export_database(engine,database,sizeof(database)) != UMI_STATUS_OK) return 8; (void)puts(database); umi_studio_compiler_centre_destroy(centre); return 0; }
