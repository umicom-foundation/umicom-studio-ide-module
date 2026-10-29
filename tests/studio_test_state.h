/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/studio_test_state.h
 *
 * PURPOSE:
 *   Keep service-test preferences in a uniquely owned temporary directory.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_TEST_STATE_H
#define UMICOM_STUDIO_TEST_STATE_H
#include <glib.h>
#include <glib/gstdio.h>
#include <stdio.h>
typedef struct StudioTestState {
    char *cwd, *data, *directory;
    int changed;
} StudioTestState;

/* Test processes own their environment and cwd. Retain the fixture for review
 * rather than recursively deleting a directory chosen by the launch context. */
static inline int StudioTestStateEnd(StudioTestState *state)
{
    int ok=1;
    if(state->changed) {
        if(g_chdir(state->cwd)!=0)ok=0;
        if(state->data!=NULL) { if(!g_setenv("UMICOM_STUDIO_DATA_PATH",state->data,TRUE))ok=0; }
        else g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    }
    if(state->directory!=NULL)printf("Retained isolated Studio fixture: %s\n",state->directory);
    g_free(state->cwd);g_free(state->data);g_free(state->directory);
    *state=(StudioTestState){0};
    return ok;
}
static inline int StudioTestStateBegin(StudioTestState *state)
{
    *state=(StudioTestState){0};
    state->cwd=g_get_current_dir();
    state->data=g_strdup(g_getenv("UMICOM_STUDIO_DATA_PATH"));
    state->directory=g_dir_make_tmp("umicom-studio-services-XXXXXX",NULL);
    if(state->cwd==NULL || state->directory==NULL ||
        (g_getenv("UMICOM_STUDIO_DATA_PATH")!=NULL && state->data==NULL) || g_chdir(state->directory)!=0) {
        (void)StudioTestStateEnd(state);return 0;
    }
    state->changed=1;
    g_unsetenv("UMICOM_STUDIO_DATA_PATH");
    if(g_mkdir_with_parents("config",0700)!=0 || g_mkdir_with_parents(".umicom",0700)!=0) {
        (void)StudioTestStateEnd(state);return 0;
    }
    return 1;
}
#endif
