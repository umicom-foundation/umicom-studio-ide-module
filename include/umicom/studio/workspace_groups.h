/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/workspace_groups.h
 *
 * PURPOSE:
 *   Publish Studio's public workspace groups contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_WORKSPACE_GROUPS_H
#define UMICOM_STUDIO_WORKSPACE_GROUPS_H
#include "umicom/studio/professional_workspace.h"
/**
 * Provide the studio workspace groups seed operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_workspace_groups_seed(UmiStudioProfessionalWorkspace *workspace);
#endif
