/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/version.c
 *
 * PURPOSE:
 *   Implement the runtime Studio version API from the public version macros.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/version.h"

/* Provide the studio version operation used by this module and its client applications. */
UmiVersion umi_studio_version(void)
{
    return (UmiVersion){UMICOM_STUDIO_VERSION_MAJOR,
                        UMICOM_STUDIO_VERSION_MINOR,
                        UMICOM_STUDIO_VERSION_PATCH};
}

/*
 * Provide the studio version string operation used by this module and its client
 * applications.
 */
const char *umi_studio_version_string(void)
{
    return UMICOM_STUDIO_VERSION_STRING;
}
