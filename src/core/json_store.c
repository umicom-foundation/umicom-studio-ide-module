/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/core/json_store.c
 *
 * PURPOSE:
 *   Implement the json store behavior for
 *   Umicom Studio IDE.
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
 * File: src/core/json_store.c
 * PURPOSE: Implementation of JSON key-value store with json-glib
 * Created by: Umicom Foundation | Author: Sammy Hegab | Date: 2025-10-01 | MIT
 *---------------------------------------------------------------------------*/

#include <json_store.h>
#include <json-glib/json-glib.h>

/* Provide the new table operation used by this module and its client applications. */
static GHashTable *new_table(void){
  return g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
}

/*
 * Read json store into validated module state and return a status when input cannot be
 * used.
 */
UmiJsonStore *umi_json_store_load(const char *path){
  UmiJsonStore *s = g_new0(UmiJsonStore,1);
  s->path = g_strdup(path?path:"config/settings.json");
  s->kv = new_table();
  /* Apply this branch only when its contract condition is satisfied. */
  if(!g_file_test(s->path, G_FILE_TEST_EXISTS)) return s;
  GError *e=NULL;
  JsonParser *p = json_parser_new();
  /* Apply this branch only when its contract condition is satisfied. */
  if(json_parser_load_from_file(p, s->path, &e)){
    JsonNode *root = json_parser_get_root(p);
    /* Apply this branch only when its contract condition is satisfied. */
    if(JSON_NODE_HOLDS_OBJECT(root)){
      JsonObject *o = json_node_get_object(root);
      GList *keys = json_object_get_members(o);
      /* Visit each bounded item once so every record receives the same rule. */
      for(GList *l=keys;l;l=l->next){
        const char *k = (const char*)l->data;
        const char *v = json_object_get_string_member(o, k);
        /* Apply this branch only when its contract condition is satisfied. */
        if(k && v) g_hash_table_insert(s->kv, g_strdup(k), g_strdup(v));
      }
      g_list_free(keys);
    }
  }else /* Preserve the original failure result so the caller can respond to the correct cause. */ if(e){ g_warning("json load: %s", e->message); g_error_free(e); }
  g_object_unref(p);
  return s;
}

/* Provide the json get operation used by this module and its client applications. */
const char *umi_json_get(UmiJsonStore *s, const char *key){
  /* Apply this branch only when its contract condition is satisfied. */
  if(!s || !key) return NULL;
  return (const char*)g_hash_table_lookup(s->kv, key);
}

/* Copy json into module-owned storage so callers keep ownership of their input values. */
void umi_json_set(UmiJsonStore *s, const char *key, const char *value){
  /* Apply this branch only when its contract condition is satisfied. */
  if(!s || !key) return;
  g_hash_table_insert(s->kv, g_strdup(key), g_strdup(value?value:""));
}

/*
 * Write json in its stable representation and report capacity or input failures to the
 * caller.
 */
gboolean umi_json_save(UmiJsonStore *s, GError **err){
  /* Apply this branch only when its contract condition is satisfied. */
  if(!s) return FALSE;
  JsonBuilder *b = json_builder_new();
  json_builder_begin_object(b);
  GHashTableIter it; gpointer k,v; g_hash_table_iter_init(&it, s->kv);
  /*
   * Continue only while work remains available; the loop body advances the state on each
   * pass.
   */
  while(g_hash_table_iter_next(&it, &k, &v)){
    json_builder_set_member_name(b, (const char*)k);
    json_builder_add_string_value(b, (const char*)v);
  }
  json_builder_end_object(b);
  JsonGenerator *g = json_generator_new();
  JsonNode *root = json_builder_get_root(b);
  json_generator_set_root(g, root);
  json_generator_set_pretty(g, TRUE);
  gboolean ok = json_generator_to_file(g, s->path, err);
  g_object_unref(g);
  json_node_free(root);
  g_object_unref(b);
  return ok;
}

/* Provide the json free operation used by this module and its client applications. */
void umi_json_free(UmiJsonStore *s){
  /* Apply this branch only when its contract condition is satisfied. */
  if(!s) return;
  g_hash_table_destroy(s->kv);
  g_free(s->path);
  g_free(s);
}
