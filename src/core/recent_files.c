/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/core/recent_files.c
 *
 * PURPOSE:
 *   Adapt Studio's original recent-file API to the Framework recent-work
 *   registry. Existing JSON data is imported once, while new state uses the
 *   shared bounded, queryable and atomically replaced representation.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "recent_files.h"

#include <json-glib/json-glib.h>
#include <stdio.h>
#include <string.h>

static const char *RECENT_ITEMS_PATH = ".umicom/studio-recent-items";
static const char *LEGACY_RECENT_JSON_PATH = "config/recent.json";

/* Build a complete Framework snapshot from Studio's older path-only API. */
static gboolean recent_snapshot_from_path(const char *path,
                                          guint64 opened_at,
                                          UmiRecentItemSnapshot *out_item)
{
    gchar *canonical_path;
    gchar *display_name;

    if (path == NULL || path[0] == '\0' || out_item == NULL) return FALSE;
    canonical_path = g_canonicalize_filename(path, NULL);
    if (canonical_path == NULL) return FALSE;
    display_name = g_path_get_basename(canonical_path);
    if (display_name == NULL ||
        strlen(canonical_path) >= sizeof(out_item->uri) ||
        strlen(display_name) >= sizeof(out_item->label)) {
        g_free(display_name);
        g_free(canonical_path);
        return FALSE;
    }

    (void)memset(out_item, 0, sizeof(*out_item));
    out_item->struct_size = (uint32_t)sizeof(*out_item);
    out_item->api_version = 1U;
    (void)snprintf(out_item->uri, sizeof(out_item->uri),
                   "%s", canonical_path);
    if (umi_platform_recent_item_id_from_uri(
            "studio-workspace", out_item->uri,
            out_item->id, sizeof(out_item->id)) != UMI_STATUS_OK) {
        g_free(display_name);
        g_free(canonical_path);
        return FALSE;
    }
    (void)snprintf(out_item->label, sizeof(out_item->label),
                   "%s", display_name);
    (void)snprintf(out_item->kind, sizeof(out_item->kind), "%s", "workspace");
    out_item->last_opened = (uint64_t)opened_at;
    out_item->open_count = 1U;

    g_free(display_name);
    g_free(canonical_path);
    return TRUE;
}

/* Add or refresh one path while preserving user-controlled pinned state and
 * the existing open counter. */
static gboolean recent_upsert_path(UmiRecent *recent,
                                   const char *path,
                                   guint64 opened_at)
{
    UmiRecentItemSnapshot item;
    UmiRecentItemSnapshot existing;

    if (recent == NULL || recent->registry == NULL ||
        !recent_snapshot_from_path(path, opened_at, &item)) {
        return FALSE;
    }
    if (umi_platform_recent_items_registry_find(
            recent->registry, item.id, &existing) == UMI_STATUS_OK) {
        item.open_count = existing.open_count < UINT64_MAX
            ? existing.open_count + 1U
            : UINT64_MAX;
        item.pinned = existing.pinned;
    }
    return umi_platform_recent_items_registry_upsert(
               recent->registry, &item) == UMI_STATUS_OK;
}

/* Import the previous JSON array without deleting or rewriting it. Keeping
 * that source file makes migration recoverable if a developer needs it. */
static gboolean recent_import_legacy_json(UmiRecent *recent)
{
    gchar *text = NULL;
    gsize text_length = 0U;
    JsonParser *parser;
    JsonNode *root;
    JsonArray *items;
    guint index;
    guint item_count;
    gboolean imported = FALSE;

    if (!g_file_get_contents(
            LEGACY_RECENT_JSON_PATH, &text, &text_length, NULL)) {
        return FALSE;
    }
    parser = json_parser_new();
    if (!json_parser_load_from_data(
            parser, text, (gssize)text_length, NULL)) {
        g_object_unref(parser);
        g_free(text);
        return FALSE;
    }

    root = json_parser_get_root(parser);
    if (root == NULL || json_node_get_node_type(root) != JSON_NODE_ARRAY) {
        g_object_unref(parser);
        g_free(text);
        return FALSE;
    }
    items = json_node_get_array(root);
    item_count = json_array_get_length(items);
    for (index = 0U; index < item_count && index < recent->max_items; ++index) {
        const char *path = json_array_get_string_element(items, index);
        guint64 ordering_time = (guint64)(item_count - index);
        if (path != NULL && path[0] != '\0' &&
            recent_upsert_path(recent, path, ordering_time)) {
            imported = TRUE;
        }
    }

    g_object_unref(parser);
    g_free(text);
    (void)umi_platform_recent_items_registry_trim(
        recent->registry, (size_t)recent->max_items);
    return imported;
}

/* Load shared recent work and perform a recoverable legacy import if needed. */
UmiRecent *umi_recent_load(void)
{
    UmiRecent *recent = g_new0(UmiRecent, 1);
    UmiStatus status;
    int loaded = 0;

    recent->max_items = 20U;
    status = umi_platform_recent_items_registry_load(
        RECENT_ITEMS_PATH, &recent->registry, &loaded);
    if (status != UMI_STATUS_OK) {
        /* Preserve an unreadable file for diagnosis and continue with a safe
         * empty model instead of making Studio startup fail. */
        g_warning("Recent work could not be loaded: %s", umi_status_text(status));
        if (umi_platform_recent_items_registry_create(&recent->registry) !=
            UMI_STATUS_OK) {
            g_free(recent);
            return NULL;
        }
        recent->persistence_enabled = FALSE;
    } else {
        recent->persistence_enabled = TRUE;
    }
    if (loaded == 0 && recent_import_legacy_json(recent)) {
        (void)umi_recent_save(recent);
    }
    return recent;
}

/* Save through Framework only when the source state was safe to replace. */
gboolean umi_recent_save(const UmiRecent *recent)
{
    if (recent == NULL || recent->registry == NULL ||
        !recent->persistence_enabled) return FALSE;
    return umi_platform_recent_items_registry_save(
               recent->registry, RECENT_ITEMS_PATH) == UMI_STATUS_OK;
}

/* Record one successful open and apply the configured history limit. */
void umi_recent_add(UmiRecent *recent, const char *path)
{
    guint64 opened_at;

    if (recent == NULL || path == NULL || path[0] == '\0') return;
    opened_at = (guint64)(g_get_real_time() / G_USEC_PER_SEC);
    if (recent_upsert_path(recent, path, opened_at)) {
        (void)umi_platform_recent_items_registry_trim(
            recent->registry, (size_t)recent->max_items);
    }
}

/* Release the Framework registry and its small Studio compatibility owner. */
void umi_recent_free(UmiRecent *recent)
{
    if (recent == NULL) return;
    umi_platform_recent_items_registry_destroy(recent->registry);
    g_free(recent);
}
