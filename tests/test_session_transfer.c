/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_session_transfer.c
 * PURPOSE: Exercise the product session exchange boundary through the public API.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/workspace_commands.h"
#include "umicom/application/experience_catalogue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Refused archives must keep the active product usable. Checks remain active
 * in Release builds and call the linked product library rather than a stub. */
#define CHECK(expression) do { if (!(expression)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expression); \
    free(bytes); return 1; \
} } while (0)
int main(void)
{
    const UmiApplicationExperienceDefinition *experience =
        umi_application_experience_catalogue_find("org.umicom.studio");
    UmiApplicationSession session, preview;
    unsigned char *bytes = NULL;
    size_t size = 0U;
    CHECK(umi_application_session_init(experience, &session) == UMI_STATUS_OK);
    CHECK(umi_studio_workspace_session_capture(&session, NULL, 0U, &size) == UMI_STATUS_OK);
    bytes = malloc(size);
    CHECK(bytes != NULL);
    CHECK(umi_studio_workspace_session_capture(&session, bytes, size, &size) == UMI_STATUS_OK);
    CHECK(umi_studio_workspace_session_preview(bytes, size, &preview) == UMI_STATUS_OK);
    CHECK(preview.experience == experience && preview.active_panel_count == session.active_panel_count);
    uint64_t observed = session.revision;
    CHECK(umi_studio_workspace_session_apply(&session, observed, bytes, size) == UMI_STATUS_OK);
    CHECK(session.revision == observed + 1U);
    unsigned char before[sizeof(session)]; memcpy(before, &session, sizeof(session));
    CHECK(umi_studio_workspace_session_apply(&session, observed, bytes, size) == UMI_STATUS_INVALID_STATE);
    CHECK(memcmp(before, &session, sizeof(session)) == 0);
    bytes[size - 1U] ^= 1U;
    CHECK(umi_studio_workspace_session_apply(&session, session.revision, bytes, size) == UMI_STATUS_PARSE_ERROR);
    CHECK(memcmp(before, &session, sizeof(session)) == 0);
    bytes[size - 1U] ^= 1U;
    const UmiApplicationExperienceDefinition *foreign =
        umi_application_experience_catalogue_find("org.umicom.trader");
    UmiApplicationSession foreign_session;
    CHECK(umi_application_session_init(foreign, &foreign_session) == UMI_STATUS_OK);
    size_t unchanged = 77U;
    CHECK(umi_studio_workspace_session_capture(&foreign_session, NULL, 0U, &unchanged) == UMI_STATUS_INVALID_ARGUMENT);
    CHECK(unchanged == 77U);
    CHECK(umi_studio_workspace_session_apply(&foreign_session, foreign_session.revision, bytes, size) == UMI_STATUS_INVALID_ARGUMENT);
    free(bytes);
    return 0;
}
