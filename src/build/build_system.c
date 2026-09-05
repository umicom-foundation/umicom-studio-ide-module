/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/build/build_system.c
 *
 * PURPOSE:
 *   Implement the build system behavior for
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
 * File: src/build/build_system.c
 * PURPOSE: Implementation of build system detection and argv expansion
 * Created by: Umicom Foundation | Author: Sammy Hegab | Date: 2025-10-01 | MIT
 *---------------------------------------------------------------------------*/

#include <glib.h>          /* GPtrArray, g_shell_parse_argv, etc. */
#include <string.h>        /* strchr */
#include "build_system.h"  /* public types/prototypes */

/* Duplicate command text while treating a missing command as an empty one. */
static gchar *sdup(const char *s) { return g_strdup(s ? s : ""); }

/* Check a build marker beneath the selected project instead of relying on the
 * process current directory, which may be the IDE installation directory. */
static gboolean marker_exists(const char *root, const char *marker)
{
  gchar *path;
  gboolean exists;

  if (!marker || !*marker) return FALSE;
  path = g_build_filename(root && *root ? root : ".", marker, NULL);
  exists = g_file_test(path, G_FILE_TEST_EXISTS);
  g_free(path);
  return exists;
}

/* Compare paths in the form CMake stores them, independent of slash style and
 * Windows drive-letter case. */
static gboolean same_workspace_path(const char *left, const char *right)
{
  gchar *left_absolute;
  gchar *right_absolute;
  gchar *left_normal;
  gchar *right_normal;
  gboolean same;

  if (!left || !right) return FALSE;
  left_absolute = g_canonicalize_filename(left, NULL);
  right_absolute = g_canonicalize_filename(right, NULL);
  left_normal = g_ascii_strdown(left_absolute, -1);
  right_normal = g_ascii_strdown(right_absolute, -1);
  g_strdelimit(left_normal, "\\", '/');
  g_strdelimit(right_normal, "\\", '/');
  same = g_strcmp0(left_normal, right_normal) == 0;
  g_free(left_absolute);
  g_free(right_absolute);
  g_free(left_normal);
  g_free(right_normal);
  return same;
}

/* Reject a generated cache that belongs to a different checkout. */
static gboolean cache_matches_workspace(const char *root,
                                        const char *relative_build_directory)
{
  gchar *cache_path;
  gchar *contents = NULL;
  gsize length = 0U;
  const gchar *marker;
  const gchar *value;
  const gchar *end;
  gchar *recorded_root;
  gboolean matches;

  cache_path = g_build_filename(root && *root ? root : ".",
                                relative_build_directory,
                                "CMakeCache.txt", NULL);
  if (!g_file_test(cache_path, G_FILE_TEST_EXISTS)) {
    g_free(cache_path);
    return TRUE;
  }
  if (!g_file_get_contents(cache_path, &contents, &length, NULL)) {
    g_free(cache_path);
    return FALSE;
  }
  marker = g_strstr_len(contents, (gssize)length,
                        "CMAKE_HOME_DIRECTORY:INTERNAL=");
  /* An unfamiliar cache format is left usable; only an explicit mismatch is
   * rejected so older generators remain compatible. */
  if (marker == NULL) {
    g_free(contents);
    g_free(cache_path);
    return TRUE;
  }
  value = marker + strlen("CMAKE_HOME_DIRECTORY:INTERNAL=");
  end = strchr(value, '\n');
  recorded_root = g_strndup(value, end != NULL ? (gsize)(end - value)
                                               : strlen(value));
  g_strchomp(recorded_root);
  matches = same_workspace_path(root, recorded_root);
  g_free(recorded_root);
  g_free(contents);
  g_free(cache_path);
  return matches;
}

/*
 * Find an already-configured CMake build directory below the workspace.
 *
 * CMake presets normally place their generated files in a nested directory
 * such as build/windows-ucrt64-debug.  Looking only for build.ninja beside
 * CMakeLists.txt makes the IDE incorrectly report that no build system exists,
 * even though an incremental build is ready to use.  The explicit preference
 * order keeps selection deterministic and lets headless builds win only when
 * no normal debug build is available.
 */
static gchar *find_nested_build_directory(const char *root)
{
  static const char *const candidates[] = {
    "build/windows-ucrt64-debug",
    "build/windows-ucrt64-headless-debug",
    "build/windows-ucrt64-all-debug",
    "build/headless-debug",
    "build/debug",
    "build/release",
    "build"
  };
  gsize index;

  for (index = 0U; index < G_N_ELEMENTS(candidates); ++index) {
    gchar *ninja_path = g_build_filename(root && *root ? root : ".",
                                         candidates[index], "build.ninja", NULL);
    gchar *make_path = g_build_filename(root && *root ? root : ".",
                                        candidates[index], "Makefile", NULL);
    gchar *cache_path = g_build_filename(root && *root ? root : ".",
                                         candidates[index], "CMakeCache.txt", NULL);
    gboolean configured = (g_file_test(ninja_path, G_FILE_TEST_EXISTS) ||
                           g_file_test(make_path, G_FILE_TEST_EXISTS) ||
                           g_file_test(cache_path, G_FILE_TEST_EXISTS)) &&
                          cache_matches_workspace(root, candidates[index]);
    g_free(ninja_path);
    g_free(make_path);
    g_free(cache_path);
    if (configured) return g_strdup(candidates[index]);
  }
  return NULL;
}

/* Return a known build-preset name when every generated cache was stale. */
static gchar *find_cmake_preset(const char *root)
{
#ifdef G_OS_WIN32
  static const char *const candidates[] = {
    "windows-ucrt64-debug",
    "windows-ucrt64-headless-debug",
    "windows-ucrt64-all-debug",
    "headless-debug"
  };
#else
  static const char *const candidates[] = {
    "headless-debug",
    "debug"
  };
#endif
  const gchar *requested = g_getenv("UMICOM_CMAKE_BUILD_PRESET");
  gchar *presets_path;
  gchar *contents = NULL;
  gsize length = 0U;
  gsize index;

  presets_path = g_build_filename(root && *root ? root : ".",
                                  "CMakePresets.json", NULL);
  if (!g_file_get_contents(presets_path, &contents, &length, NULL)) {
    g_free(presets_path);
    return NULL;
  }
  if (requested != NULL && requested[0] != '\0' &&
      g_strstr_len(contents, (gssize)length, requested) != NULL) {
    gchar *result = g_strdup(requested);
    g_free(contents);
    g_free(presets_path);
    return result;
  }
  for (index = 0U; index < G_N_ELEMENTS(candidates); ++index) {
    if (g_strstr_len(contents, (gssize)length, candidates[index]) != NULL) {
      gchar *result = g_strdup(candidates[index]);
      g_free(contents);
      g_free(presets_path);
      return result;
    }
  }
  g_free(contents);
  g_free(presets_path);
  return NULL;
}

/* Build a shell-parser-safe command string from a relative build directory or
 * a preset when no cache from this checkout is available. */
static gchar *cmake_build_command(const char *root,
                                  const char *build_directory,
                                  const char *suffix)
{
  gchar *preset;
  gchar *quoted;
  gchar *command;

  if (build_directory == NULL) {
    preset = find_cmake_preset(root);
    if (preset != NULL) {
      quoted = g_shell_quote(preset);
      command = g_strdup_printf("cmake --build --preset %s%s", quoted,
                                suffix ? suffix : "");
      g_free(quoted);
      g_free(preset);
      return command;
    }
  }
  quoted = g_shell_quote(build_directory ? build_directory : "build");
  command = g_strdup_printf("cmake --build %s%s", quoted,
                            suffix ? suffix : "");
  g_free(quoted);
  return command;
}

/* Build a shell-parser-safe CTest command for the selected build directory or
 * the matching preset when an old cache was rejected. */
static gchar *ctest_command(const char *root, const char *build_directory)
{
  gchar *preset;
  gchar *quoted;
  gchar *command;

  if (build_directory == NULL) {
    preset = find_cmake_preset(root);
    if (preset != NULL) {
      quoted = g_shell_quote(preset);
      command = g_strdup_printf("ctest --preset %s --output-on-failure",
                                quoted);
      g_free(quoted);
      g_free(preset);
      return command;
    }
  }
  quoted = g_shell_quote(build_directory ? build_directory : "build");
  command = g_strdup_printf("ctest --test-dir %s --output-on-failure", quoted);
  g_free(quoted);
  return command;
}

/* Detect the build metadata that belongs to the selected workspace root. */
UmiBuildSys *umi_buildsys_detect(const char *root){
  UmiBuildSys *b = g_new0(UmiBuildSys,1);
#ifdef G_OS_WIN32
  if (marker_exists(root, "build.ninja")) {
    b->tool = UMI_TOOL_NINJA;
    b->build_cmd = sdup("ninja");
    b->run_cmd   = sdup("ninja run");
    b->test_cmd  = sdup("ninja test");
  } else if (marker_exists(root, "Makefile")) {
    b->tool = UMI_TOOL_MAKE;
    b->build_cmd = sdup("mingw32-make -j");
    b->run_cmd   = sdup("mingw32-make run");
    b->test_cmd  = sdup("mingw32-make test");
  } else if (marker_exists(root, "CMakeLists.txt") ||
             marker_exists(root, "CMakePresets.json")) {
    gchar *build_directory = find_nested_build_directory(root);
    /* CMake is selected from workspace metadata and reuses the first existing
     * preset output, so the IDE does not silently create a second cache. */
    b->tool = UMI_TOOL_CUSTOM;
    b->build_cmd = cmake_build_command(root, build_directory, NULL);
    b->run_cmd   = cmake_build_command(root, build_directory, " --target run");
    b->test_cmd  = ctest_command(root, build_directory);
    g_free(build_directory);
  } else {
    b->tool = UMI_TOOL_MSBUILD;
    b->build_cmd = sdup("msbuild /m");
    b->run_cmd   = sdup("build\\app.exe");
    b->test_cmd  = sdup("ctest");
  }
#else
  if (marker_exists(root, "build.ninja")) {
    b->tool = UMI_TOOL_NINJA;
    b->build_cmd = sdup("ninja");
    b->run_cmd   = sdup("ninja run");
    b->test_cmd  = sdup("ninja test");
  } else if (marker_exists(root, "Makefile")) {
    b->tool = UMI_TOOL_MAKE;
    b->build_cmd = sdup("make -j");
    b->run_cmd   = sdup("make run");
    b->test_cmd  = sdup("make test");
  } else if (marker_exists(root, "CMakeLists.txt") ||
             marker_exists(root, "CMakePresets.json")) {
    gchar *build_directory = find_nested_build_directory(root);
    b->tool = UMI_TOOL_CUSTOM;
    b->build_cmd = cmake_build_command(root, build_directory, NULL);
    b->run_cmd   = cmake_build_command(root, build_directory, " --target run");
    b->test_cmd  = ctest_command(root, build_directory);
    g_free(build_directory);
  } else {
    b->tool = UMI_TOOL_CUSTOM;
    /* Keep fallback commands shell-free. They fail clearly when a project has
     * no recognised build metadata instead of executing an opaque string. */
    b->build_cmd = sdup("cmake --build build");
    b->run_cmd   = sdup("./app");
    b->test_cmd  = sdup("ctest --test-dir build --output-on-failure");
  }
#endif
  return b;
}

/* Replace all three commands when a project supplies its own build policy. */
void umi_buildsys_set(UmiBuildSys *bs, const char *build, const char *run, const char *test){
  if (!bs) return;
  g_free(bs->build_cmd);
  g_free(bs->run_cmd);
  g_free(bs->test_cmd);
  bs->build_cmd = sdup(build);
  bs->run_cmd   = sdup(run);
  bs->test_cmd  = sdup(test);
}

/*-----------------------------------------------------------------------------
 * Safe argv splitter (GLib-backed).
 * - Returns a GPtrArray of g_strdup'ed strings, NULL-terminated.
 * - Always returns a valid array (never NULL).
 *---------------------------------------------------------------------------*/
static GPtrArray* split_argv_safe(const gchar *cmd) {
  if (!cmd || !*cmd) {
    GPtrArray *empty = g_ptr_array_new_with_free_func(g_free);
    g_ptr_array_add(empty, NULL);
    return empty;
  }

  int argc = 0;
  gchar **argv = NULL;
  GError *gerr = NULL;

  if (!g_shell_parse_argv(cmd, &argc, &argv, &gerr)) {
    if (gerr) g_error_free(gerr);
    GPtrArray *empty = g_ptr_array_new_with_free_func(g_free);
    g_ptr_array_add(empty, NULL);
    return empty;
  }

  GPtrArray *a = g_ptr_array_new_with_free_func(g_free);
  for (int i = 0; i < argc; i++) g_ptr_array_add(a, g_strdup(argv[i]));
  g_strfreev(argv);
  g_ptr_array_add(a, NULL);
  return a;
}

/* Expand the selected build command into arguments for shell-free execution. */
GPtrArray *umi_buildsys_build_argv(const UmiBuildSys *bs){
  return split_argv_safe(bs ? bs->build_cmd : NULL);
}
/* Expand the selected run command into arguments for shell-free execution. */
GPtrArray *umi_buildsys_run_argv(const UmiBuildSys *bs){
  return split_argv_safe(bs ? bs->run_cmd : NULL);
}
/* Expand the selected test command into arguments for shell-free execution. */
GPtrArray *umi_buildsys_test_argv(const UmiBuildSys *bs){
  return split_argv_safe(bs ? bs->test_cmd : NULL);
}

/* Release command strings and the build-system descriptor itself. */
void umi_buildsys_free(UmiBuildSys *b){
  if (!b) return;
  g_free(b->build_cmd);
  g_free(b->run_cmd);
  g_free(b->test_cmd);
  g_free(b);
}
/*---------------------------------------------------------------------------*/
/*  END OF FILE */
