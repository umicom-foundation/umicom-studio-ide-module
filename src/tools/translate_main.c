/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/tools/translate_main.c
 *
 * PURPOSE:
 *   Present checked Java and C translation through the native Studio toolset.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/toolchain/cvm_file_translation.h"

#include <stdio.h>
#include <string.h>

static void PrintUsage(FILE *stream)
{
    (void)fputs("Umicom Studio source translation\n"
        "Usage: umicom-studio-translate c-to-java|java-to-c ABSOLUTE_INPUT ABSOLUTE_NEW_OUTPUT\n"
        "Translates one checked integer answer function. Existing files are preserved.\n"
        "Exit 0: written. 2: source refused. 1: file failure. 64: usage.\n", stream);
}

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        PrintUsage(stdout);
        return fflush(stdout) == EOF || ferror(stdout) != 0 ? 1 : 0;
    }
    if (argc != 4) {
        PrintUsage(stderr);
        return 64;
    }

    UmcTranslationDirection direction;
    if (strcmp(argv[1], "c-to-java") == 0) {
        direction = UMC_TRANSLATE_C_TO_JAVA;
    } else if (strcmp(argv[1], "java-to-c") == 0) {
        direction = UMC_TRANSLATE_JAVA_TO_C;
    } else {
        PrintUsage(stderr);
        return 64;
    }

    /* Framework owns file selection and writing; the IDE tool only presents
     * the result supplied by the shared compiler service. */
    UmcDiagnostic diagnostic;
    const UmiStatus status = UmiCvmTranslateFile(direction, argv[2], argv[3], &diagnostic);
    if (status == UMI_STATUS_OK) {
        return 0;
    }
    if (status == UMI_STATUS_PARSE_ERROR && diagnostic.message != NULL) {
        (void)fprintf(stderr, "%s:%zu:%zu: %s\n", argv[2], diagnostic.line,
                      diagnostic.column, diagnostic.message);
        return 2;
    }
    (void)fprintf(stderr, "Translation could not complete: %s.\n", umi_status_text(status));
    return 1;
}
