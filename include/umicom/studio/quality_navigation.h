/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/quality_navigation.h
 *
 * PURPOSE:
 *   Publish Studio's public quality navigation contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_QUALITY_NAVIGATION_H
#define UMICOM_STUDIO_QUALITY_NAVIGATION_H
#include "umicom/studio/quality_centre.h"
typedef struct UmiStudioQualityNavigationTarget {
    char evidence_id[UMI_CODEGUARD_QUALITY_ID_CAPACITY];
    char path[UMI_CODEGUARD_QUALITY_PATH_CAPACITY];
    size_t line;
    size_t column;
    char label[UMI_CODEGUARD_QUALITY_TEXT_CAPACITY];
} UmiStudioQualityNavigationTarget;
UmiStatus umi_studio_quality_navigation_resolve(const UmiStudioQualityCentre *centre,const char *evidence_id,UmiStudioQualityNavigationTarget *out_target);
#endif
