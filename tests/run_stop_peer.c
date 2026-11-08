/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/run_stop_peer.c
 *
 * PURPOSE:
 *   Hold one isolated launch open after publishing a readiness marker so native cancellation can be observed.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#if !defined(_WIN32)
#define _POSIX_C_SOURCE 200809L
#endif
#include <stdio.h>
#include <glib/gstdio.h>
#if defined(_WIN32)
#include <windows.h>
#else
#include <time.h>
#endif

int main(int argc, char **argv)
{
    if (argc != 2) return 2;
    /* The test supplies a marker inside its unique temporary directory. This
     * process has no repository, network or interactive desktop responsibilities. */
    /* GLib preserves the UTF-8 fixture path on Windows too. The narrow C
     * runtime spelling is retained for review; it can lose non-ASCII names. */
#if 0
    FILE *marker = fopen(argv[1], "wb");
#endif
    FILE *marker = g_fopen(argv[1], "wb");
    if (marker == NULL) return 3;
    if (fputs("ready\n", marker) == EOF) { fclose(marker); return 4; }
    if (fclose(marker) != 0) return 4;
    puts("Studio cancellation peer ready");
    if (fflush(stdout) != 0) return 5;
    /* Bound the fixture independently of the parent's timeout. A natural exit
     * is not a cancellation pass; the parent checks the session's final state. */
    for (unsigned i = 0U; i < 300U; ++i) {
#if defined(_WIN32)
        Sleep(100U);
#else
        struct timespec delay = {0, 100000000L};
        (void)nanosleep(&delay, NULL);
#endif
    }
    return 0;
}
