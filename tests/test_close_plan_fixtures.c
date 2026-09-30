/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/tests/test_close_plan_fixtures.c
 *
 * PURPOSE:
 *   Verify every externally inspectable boolean close state from the 256-case
 *   STU-02 fixture matrix. No directory enumeration API is required.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/close_plan.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef UMI_STUDIO_CLOSE_FIXTURE_DIR
#define UMI_STUDIO_CLOSE_FIXTURE_DIR "tests/fixtures/close_lifecycle"
#endif

typedef struct Fixture {
    UmiStudioClosePlanInput input;
    int expected_decision;
    unsigned expected_actions;
    int expected_second_close;
} Fixture;

static int ReadInteger(const char *line, const char *name, int *out)
{
    size_t length = strlen(name);
    char *end = NULL;
    long value;

    if (strncmp(line, name, length) != 0 || line[length] != '=') return 0;
    value = strtol(line + length + 1U, &end, 10);
    if (end == line + length + 1U) return -1;
    while (*end == '\r' || *end == '\n') ++end;
    if (*end != '\0') return -1;
    *out = (int)value;
    return 1;
}

static int LoadFixture(const char *path, Fixture *fixture)
{
    FILE *file;
    char line[128];

    if (path == NULL || fixture == NULL) return 0;
    (void)memset(fixture, 0, sizeof(*fixture));
    file = fopen(path, "rb");
    if (file == NULL) return 0;

    while (fgets(line, sizeof line, file) != NULL) {
        int value;
        int matched = 0;
#define READ(NAME, TARGET) do { \
        int r = ReadInteger(line, NAME, &value); \
        if (r < 0) { fclose(file); return 0; } \
        if (r > 0) { TARGET = value; matched = 1; } \
    } while (0)
        READ("services_ready", fixture->input.services_ready);
        READ("snapshot_ready", fixture->input.snapshot_ready);
        READ("test_run_pending", fixture->input.test_run_pending);
        READ("test_discovery_pending", fixture->input.test_discovery_pending);
        READ("build_busy", fixture->input.build_busy);
        READ("debugger_busy", fixture->input.debugger_busy);
        READ("dirty_documents", fixture->input.dirty_documents);
        READ("discard_confirmed", fixture->input.discard_confirmed);
        READ("expect_decision", fixture->expected_decision);
        READ("expect_actions", value);
        if (matched && strncmp(line, "expect_actions=", 15U) == 0)
            fixture->expected_actions = (unsigned)value;
        READ("expect_second_close", fixture->expected_second_close);
#undef READ
    }

    return fclose(file) == 0;
}

int main(void)
{
    unsigned mask;

    for (mask = 0U; mask < 256U; ++mask) {
        char path[512];
        Fixture fixture;
        UmiStudioClosePlan plan;

        (void)snprintf(path, sizeof path,
            "%s/state-%03u.case", UMI_STUDIO_CLOSE_FIXTURE_DIR, mask);
        if (!LoadFixture(path, &fixture)) {
            fprintf(stderr, "cannot read fixture %s\n", path);
            return 1;
        }
        if (UmiStudioClosePlanEvaluate(&fixture.input, &plan) !=
            UMI_STATUS_OK) {
            fprintf(stderr, "planner rejected fixture %u\n", mask);
            return 1;
        }
        if ((int)plan.decision != fixture.expected_decision ||
            plan.actions != fixture.expected_actions ||
            plan.requires_second_close != fixture.expected_second_close) {
            fprintf(stderr,
                "fixture %u mismatch: decision %d/%d actions %u/%u second %d/%d\n",
                mask, (int)plan.decision, fixture.expected_decision,
                (unsigned)plan.actions, fixture.expected_actions,
                plan.requires_second_close, fixture.expected_second_close);
            return 1;
        }
    }

    puts("All 256 Studio close-lifecycle fixtures passed.");
    return 0;
}
