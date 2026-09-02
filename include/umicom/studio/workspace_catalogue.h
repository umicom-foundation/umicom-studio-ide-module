/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/workspace_catalogue.h
 *
 * PURPOSE:
 *   Publish Studio's public workspace catalogue contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_WORKSPACE_CATALOGUE_H
#define UMICOM_STUDIO_WORKSPACE_CATALOGUE_H
#include "umicom/studio/professional_workspace.h"
/**
 * Provide the studio workspace catalogue seed operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_workspace_catalogue_seed(UmiStudioProfessionalWorkspace *workspace);
#endif
