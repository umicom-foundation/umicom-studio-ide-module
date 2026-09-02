/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/include/umicom/studio/files.h
 *
 * PURPOSE:
 *   Provide workspace-scoped Studio file operations that resolve paths through
 *   Framework, enforce workspace trust for mutations and keep the shared index
 *   current after successful changes.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_FILES_H
#define UMICOM_STUDIO_FILES_H

#include "umicom/studio/services.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Provide the studio files resolve operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_files_resolve(UmiStudioServices *services,
                                   const char *workspace_path,
                                   char *out_path,
                                   size_t capacity);
/**
 * Provide the studio files read text operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_files_read_text(UmiStudioServices *services,
                                     const char *workspace_path,
                                     char **out_text,
                                     size_t *out_size);
/**
 * Provide the studio files write text operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_files_write_text(UmiStudioServices *services,
                                      const char *workspace_path,
                                      const char *text);
/**
 * Provide the studio files append text operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_files_append_text(UmiStudioServices *services,
                                       const char *workspace_path,
                                       const char *text);
/**
 * Provide the studio files create directory operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_files_create_directory(UmiStudioServices *services,
                                            const char *workspace_path);
/**
 * Provide the studio files rename operation used by this module and its client
 * applications.
 */
UmiStatus umi_studio_files_rename(UmiStudioServices *services,
                                  const char *source_workspace_path,
                                  const char *destination_workspace_path);
/**
 * Remove studio files while keeping the remaining records in a valid and discoverable
 * state.
 */
UmiStatus umi_studio_files_remove(UmiStudioServices *services,
                                  const char *workspace_path);

#ifdef __cplusplus
}
#endif

#endif
