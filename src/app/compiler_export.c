/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/compiler_export.c
 *
 * PURPOSE:
 *   Implement the compiler export behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/compiler_export.h"
#include <stdio.h>
/*
 * Provide the studio compiler export summary operation used by this module and its client
 * applications.
 */
/* Reusable complete snapshot formatting now belongs to Framework. The Studio adapter retains its public interface and delegates admission and failure preservation; retain the previous formatter for comparison. The previous implementation is retained for review. */
#if 0
UmiStatus umi_studio_compiler_export_summary(const UmiCompilerCentreSnapshot *snapshot,char *out_text,size_t capacity)
{ int written; /* Protect caller-owned memory by checking that required state is available before it is used. */ if (snapshot == NULL || out_text == NULL || capacity == 0U) return UMI_STATUS_INVALID_ARGUMENT; written = snprintf(out_text,capacity,"Umicom Compiler Centre\nProviders: %zu (%zu available)\nProfiles: %zu\nProjects: %zu\nUnits: %zu\nCompilation commands: %zu\nActive profile: %s\n",snapshot->providers,snapshot->available_providers,snapshot->profiles,snapshot->manifests,snapshot->units,snapshot->compilation_commands,snapshot->active_profile_id); return written < 0 || (size_t)written >= capacity ? UMI_STATUS_CAPACITY_EXCEEDED : UMI_STATUS_OK; }

#endif
UmiStatus umi_studio_compiler_export_summary(const UmiCompilerCentreSnapshot *snapshot,char *out_text,size_t capacity)
{
    return umi_compiler_centre_format_snapshot(snapshot,out_text,capacity);
}
/*
 * Provide the studio compiler export database operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_compiler_export_database(const UmiCompilerCentre *centre,char *out_json,size_t capacity) { /* Protect caller-owned memory by checking that required state is available before it is used. */ if (centre == NULL) return UMI_STATUS_INVALID_ARGUMENT; return umi_polyglot_compilation_database_export_json(&centre->compilation_database,out_json,capacity); }
