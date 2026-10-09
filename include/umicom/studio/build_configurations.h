/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/build_configurations.h
 * PURPOSE: Bind saved build configurations to the current Studio workspace and storage owner.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_STUDIO_BUILD_CONFIGURATIONS_H
#define UMICOM_STUDIO_BUILD_CONFIGURATIONS_H
#include "umicom/build/configuration_library.h"
#include "umicom/studio/services.h"
#ifdef __cplusplus
extern "C"
{
#endif
    typedef struct UmiStudioBuildConfigurationContext
    {
        uint64_t workspace_revision;
        uint64_t storage_generation;
        char root[UMI_BUILD_PATH_CAPACITY];
    } UmiStudioBuildConfigurationContext;
    /** Capture the current workspace and settings-storage binding. This grants no
 * execution trust and borrows no database handle. Capture again after changing
 * workspace, workspace trust or the settings-storage binding. */
    UmiStatus UmiStudioBuildConfigurationContextRead(UmiStudioServices *services,
                                                     UmiStudioBuildConfigurationContext *out);
    /** Copy saved names only while the captured workspace/storage binding remains
 * current. Active native build/debug jobs refuse library actions. */
    UmiStatus UmiStudioBuildConfigurationsCapture(UmiStudioServices *services,
                                                  const UmiStudioBuildConfigurationContext *context,
                                                  UmiBuildConfigurationCatalogue *out);
    /** Read a saved configuration for review; active build settings stay unchanged.
 * The catalogue revision rejects a selection made before another window saved. */
    UmiStatus UmiStudioBuildConfigurationLoad(UmiStudioServices *services,
                                              const UmiStudioBuildConfigurationContext *context,
                                              const char *name, uint64_t revision,
                                              UmiBuildProfile *out);
    /** Save form settings as a named configuration without applying them or storing
 * trust. The profile must belong to the captured project. Stale requests keep
 * the previously saved record and leave out_revision unchanged. */
    UmiStatus UmiStudioBuildConfigurationSave(UmiStudioServices *services,
                                              const UmiStudioBuildConfigurationContext *context,
                                              const char *name, const UmiBuildProfile *profile,
                                              uint64_t revision, uint64_t *out_revision);
/** Rename a saved configuration only while the captured project and storage
 * binding remain current. An existing destination is not replaced. */
UmiStatus UmiStudioBuildConfigurationRename(UmiStudioServices *services,
    const UmiStudioBuildConfigurationContext *context, const char *name, const char *replacement,
    uint64_t revision, uint64_t *out_revision);
/** Remove an explicitly confirmed saved configuration through Framework's
 * transaction. Active settings, project files and trust remain unchanged. */
UmiStatus UmiStudioBuildConfigurationRemove(UmiStudioServices *services,
    const UmiStudioBuildConfigurationContext *context, const char *name,
    uint64_t revision, uint64_t *out_revision);
#ifdef __cplusplus
}
#endif
#endif
