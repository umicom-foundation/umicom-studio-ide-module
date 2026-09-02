/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/compiler_diagnostics.c
 *
 * PURPOSE:
 *   Implement the compiler diagnostics behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
/* Umicom Studio IDE | Compiler diagnostics projection v2 | Sammy Hegab | Umicom Foundation | MIT */
#include "umicom/studio/compiler_diagnostics.h"
#include <stdio.h>
#include <string.h>
/*
 * Provide the studio compiler diagnostic matches operation used by this module and its
 * client applications.
 */
bool umi_studio_compiler_diagnostic_matches(const UmiCompilerDiagnostic *diagnostic,const UmiStudioCompilerDiagnosticFilter *filter)
{ /* Protect caller-owned memory by checking that required state is available before it is used. */ if (diagnostic == NULL) return false; /* Protect caller-owned memory by checking that required state is available before it is used. */ if (filter == NULL) return true; /* Protect caller-owned memory by checking that required state is available before it is used. */ if (diagnostic->severity < filter->minimum_severity) return false; /* Protect caller-owned memory by checking that required state is available before it is used. */ if (filter->file_contains[0] != '\0' && strstr(diagnostic->file,filter->file_contains) == NULL) return false; return filter->code_contains[0] == '\0' || strstr(diagnostic->code,filter->code_contains) != NULL; }
/*
 * Provide the studio compiler diagnostics summarise operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_compiler_diagnostics_summarise(const UmiCompilerDiagnosticSet *set,const UmiStudioCompilerDiagnosticFilter *filter,UmiStudioCompilerDiagnosticSummary *out_summary)
{
    size_t index; /* Protect caller-owned memory by checking that required state is available before it is used. */ if (set == NULL || out_summary == NULL) return UMI_STATUS_INVALID_ARGUMENT; (void)memset(out_summary,0,sizeof(*out_summary));
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < set->count; ++index) /* Keep the operation inside its valid bounds before reading, writing or adding data. */ if (umi_studio_compiler_diagnostic_matches(&set->items[index],filter)) { out_summary->visible += 1U; /* Keep the operation inside its valid bounds before reading, writing or adding data. */ if (set->items[index].severity >= UMI_COMPILER_DIAGNOSTIC_ERROR) out_summary->errors += 1U; else /* Keep the operation inside its valid bounds before reading, writing or adding data. */ if (set->items[index].severity == UMI_COMPILER_DIAGNOSTIC_WARNING) out_summary->warnings += 1U; /* Use this fallback path when the earlier condition does not apply. */ else out_summary->notes += 1U; }
    (void)snprintf(out_summary->headline,sizeof(out_summary->headline),"%zu compiler problem(s): %zu error(s), %zu warning(s), %zu note(s)",out_summary->visible,out_summary->errors,out_summary->warnings,out_summary->notes); return UMI_STATUS_OK;
}
