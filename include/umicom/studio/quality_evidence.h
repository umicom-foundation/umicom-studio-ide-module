/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/quality_evidence.h
 *
 * PURPOSE:
 *   Publish Studio's public quality evidence contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_QUALITY_EVIDENCE_H
#define UMICOM_STUDIO_QUALITY_EVIDENCE_H
#include "umicom/studio/quality_centre.h"
/**
 * Provide the studio quality record check operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_quality_record_check(UmiStudioQualityCentre *centre,const char *id,UmiCodeGuardEvidenceKind kind,bool passed,size_t observed,size_t allowed,const char *path,size_t line,const char *summary,const char *remediation);
/**
 * Provide the studio quality record architecture dependency operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_quality_record_architecture_dependency(UmiStudioQualityCentre *centre,const char *source_path,const char *target_path);
/**
 * Provide the studio quality find evidence operation used by this module and its client
 * applications.
 */
const UmiCodeGuardEvidence *umi_studio_quality_find_evidence(const UmiStudioQualityCentre *centre,const char *evidence_id);
#endif
