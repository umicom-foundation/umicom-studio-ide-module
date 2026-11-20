/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_active_translation.c
 *
 * PURPOSE:
 *   Exercise the configured editor translation actions through Studio's
 *   real command registry and verify that draft and source ownership hold.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/bootstrap.h"
#include "umicom/studio/ui.h"
#include "umicom/studio/workbench_commands.h"
#include "umicom_c/umicom_c.h"
#include "umicom_c/translate.h"
#include "studio_test_state.h"

#include <stdlib.h>
#include <string.h>

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); exit(EXIT_FAILURE); \
} } while (0)

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    const char *mode = argv[1];
    const int java = strcmp(mode, "java-to-c") == 0 || strcmp(mode, "comment-java-to-c") == 0;
    const int comments = strcmp(mode, "comment-c-to-java") == 0 || strcmp(mode, "comment-java-to-c") == 0;
    const int invalid = strcmp(mode, "invalid") == 0;
    const int large = strcmp(mode, "oversize") == 0;
    const int rollback = strcmp(mode, "view-id-rollback") == 0;
    CHECK(java || comments || invalid || large || rollback || strcmp(mode, "c-to-java") == 0);
#ifndef UMC_TRANSLATION_PRESERVES_BODY_COMMENTS
    /* An older matching SDK can still run the existing expression journeys. */
    if (comments) return 77;
#endif

    StudioTestState state = {0};
    CHECK(StudioTestStateBegin(&state));
    UmiStudioBootstrap *bootstrap = NULL;
    CHECK(umi_studio_bootstrap_create(&bootstrap) == UMI_STATUS_OK);
    CHECK(umi_studio_bootstrap_start(bootstrap) == UMI_STATUS_OK);
    UmiStudioUi *ui = umi_studio_bootstrap_ui(bootstrap);
    UmiUiWorkbench *workbench = umi_studio_ui_workbench(ui);
    UmiUiDocumentViewModel *documents = umi_ui_workbench_documents(workbench);
    UmiDocumentCoordinator *coordinator = umi_studio_ui_documents(ui);
    UmiCommandRegistry *commands = umi_studio_bootstrap_command_registry(bootstrap);

    const char *source = java
        ? "public final class UmicomAnswer { public static int answer() { return (20 + 1) * 2; } }\n"
        : "int answer(void) { return (20 + 1) * 2; }\n";
    bool usesLocals = false;
#ifdef UMC_TRANSLATION_FUNCTION_OUTPUT_CAPACITY
    /* Verify the active SDK profile against unsaved local statements. The
     * expression fixtures above still qualify earlier installed providers. */
    if (!invalid && !large && !rollback) {
        source = java
            ? "public final class UmicomAnswer { public static int answer() { int net = 21; net = net * 2; return net; } }\n"
            : "int answer(void) { int net = 21; net = net * 2; return net; }\n";
        usesLocals = true;
    }
#endif
    if (comments) {
        /* The real editor command must preserve both raw draft text and the
         * compiler's comment-preserving output in the newly owned draft. */
#define COMMENTED_BODY "\r\n    /* Reason; } */\r\n    int net = 21;\r\n" \
                       "    net = net * 2; // Keep adjustment.\r\n    return net;\r\n"
        source = java ? "public final class UmicomAnswer { public static int answer() {" COMMENTED_BODY "} }\r\n"
            : "int answer(void) {" COMMENTED_BODY "}\r\n";
#undef COMMENTED_BODY
    }
    if (invalid) source = "int answer(void) { return 1 / 0; }\n";
    size_t length = strlen(source);
    char *oversized = NULL;
    if (large) {
        length = UMC_SOURCE_LIMIT + 1U;
        oversized = malloc(length);
        CHECK(oversized != NULL);
        memset(oversized, 'a', length);
        source = oversized;
    }
    char originalView[UMI_UI_ID_CAPACITY];
    CHECK(UmiDocumentCoordinatorNewWithText(coordinator, java ? "Original.java" : "original.c",
        source, length, originalView, sizeof originalView) == UMI_STATUS_OK);
    const size_t beforeCount = umi_ui_document_view_model_count(documents);
    const char *command = java ? UMI_STUDIO_COMMAND_EDITOR_TRANSLATE_JAVA_TO_C
        : UMI_STUDIO_COMMAND_EDITOR_TRANSLATE_C_TO_JAVA;
    UmiUiActionSnapshot action;
    CHECK(umi_ui_action_model_find(umi_ui_workbench_actions(workbench),
        java ? "studio.action.editor.translate-java-to-c" : "studio.action.editor.translate-c-to-java",
        &action) == UMI_STATUS_OK);
    CHECK(strcmp(action.command_id, command) == 0 && action.argument_kind == UMI_UI_ACTION_ARGUMENT_NONE);

    char message[512] = "";
    UmiStatus status;
    if (rollback) {
        /* A late view-ID refusal cannot consume a document or editor slot. */
        char shortId[1] = {'x'};
        status = UmiDocumentCoordinatorNewWithText(coordinator, "refused.c", source, length,
            shortId, sizeof shortId);
        CHECK(shortId[0] == '\0');
    } else {
        status = umi_command_registry_execute(commands, command, NULL, message, sizeof message);
    }
    UmiUiWorkbenchSnapshot current;
    CHECK(umi_ui_workbench_snapshot(workbench, &current) == UMI_STATUS_OK);
    if (invalid || large || rollback) {
        CHECK(status == (invalid ? UMI_STATUS_PARSE_ERROR : UMI_STATUS_CAPACITY_EXCEEDED));
        CHECK(umi_ui_document_view_model_count(documents) == beforeCount);
        CHECK(strcmp(current.active_document_view, originalView) == 0);
        if (invalid) CHECK(strstr(message, "Translation refused") != NULL);
    } else {
        CHECK(status == UMI_STATUS_OK && strstr(message, "unsaved translation draft") != NULL);
        CHECK(umi_ui_document_view_model_count(documents) == beforeCount + 1U);
        CHECK(strcmp(current.active_document_view, originalView) != 0);
        UmiDocumentWorkingCopySnapshot draft;
        CHECK(umi_document_coordinator_active_snapshot(coordinator, &draft) == UMI_STATUS_OK);
        CHECK(!draft.has_path && draft.dirty);
        CHECK(strcmp(draft.display_name, java ? "answer.c" : "UmicomAnswer.java") == 0);
        char *translated = NULL;
        size_t translatedBytes = 0U;
        CHECK(UmiUiDocumentViewModelCopyText(documents, current.active_document_view,
            &translated, &translatedBytes) == UMI_STATUS_OK);
        CHECK(translatedBytes > 0U && strstr(translated, java ? "int answer(void)" : "public static int answer()") != NULL);
        if (usesLocals) CHECK(strstr(translated, "int net = 21;") != NULL &&
            strstr(translated, "net = net * 2;") != NULL && strstr(translated, "return net;") != NULL);
        if (comments) CHECK(strstr(translated, "\r\n    /* Reason; } */\r\n") != NULL &&
            strstr(translated, "// Keep adjustment.\r\n") != NULL);
        UmiUiDocumentViewModelFreeText(translated);
    }

    char *retained = NULL;
    size_t retainedBytes = 0U;
    CHECK(UmiUiDocumentViewModelCopyText(documents, originalView, &retained, &retainedBytes) == UMI_STATUS_OK);
    CHECK(retainedBytes == length && memcmp(retained, source, length) == 0);
    UmiUiDocumentViewModelFreeText(retained);
    free(oversized);
    CHECK(umi_studio_bootstrap_stop(bootstrap) == UMI_STATUS_OK);
    umi_studio_bootstrap_destroy(bootstrap);
    CHECK(StudioTestStateEnd(&state));
    return EXIT_SUCCESS;
}
