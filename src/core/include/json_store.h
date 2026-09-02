/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/core/include/json_store.h
 *
 * PURPOSE:
 *   Declare the json store contract shared by Framework services and thin
 *   applications.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/core/json_store.h
 * PURPOSE: Simple JSON-backed key-value store (string->string) using JSON-GLib.
 * Created by: Umicom Foundation | Author: Sammy Hegab | Date: 2025-10-08 | MIT
 *---------------------------------------------------------------------------*/
#ifndef UMI_JSON_STORE_H
#define UMI_JSON_STORE_H
#include <glib.h>
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Represent the json store data shared with callers of this public contract.
 */
typedef struct UmiJsonStore {
  char       *path;
  GHashTable *kv;
} UmiJsonStore;
/**
 * Read json store into validated module state and return a status when input cannot be
 * used.
 */
UmiJsonStore *umi_json_store_load(const char *path);
/**
 * Provide the json get operation used by this module and its client applications.
 */
const char *umi_json_get(UmiJsonStore *s, const char *key);
/**
 * Copy json into module-owned storage so callers keep ownership of their input values.
 */
void        umi_json_set(UmiJsonStore *s, const char *key, const char *value);
/**
 * Write json in its stable representation and report capacity or input failures to the
 * caller.
 */
gboolean    umi_json_save(UmiJsonStore *s, GError **err);
/**
 * Provide the json free operation used by this module and its client applications.
 */
void        umi_json_free(UmiJsonStore *s);
#ifdef __cplusplus
}
#endif
#endif /* UMI_JSON_STORE_H */
