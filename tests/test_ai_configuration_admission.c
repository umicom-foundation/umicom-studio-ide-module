/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_ai_configuration_admission.c
 *
 * PURPOSE:
 *   Verify bounded Studio AI configuration, optional remote metadata and finite fitness before service construction.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/


#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "umicom/studio/ai_platform.h"

typedef struct TextCase { size_t offset, capacity; int required; } TextCase;
#define FIELD(name, required) {offsetof(UmiStudioAiPlatformConfig, name), \
    sizeof(((UmiStudioAiPlatformConfig *)0)->name), required}
static const TextCase fields[] = {
    FIELD(authorengine_executable, 1), FIELD(workspace, 1), FIELD(preferred_runtime_id, 1),
    FIELD(remote_provider_id, 0), FIELD(remote_endpoint, 0), FIELD(remote_model_id, 0),
    FIELD(remote_secret_reference, 0), FIELD(knowledge_archive_path, 1)
};
#undef FIELD

/* Invalid settings must fail before constructor ownership begins. The output
 * sentinel is allocated storage, never a fabricated or dereferenced pointer. */
static void check(UmiStudioAiPlatformConfig *config, UmiStatus expected)
{
    UmiStudioAiPlatformConfig before;
    void *sentinel = malloc(1U);
    UmiStudioAiPlatform *out = sentinel;
    assert(sentinel != NULL);
    memcpy(&before, config, sizeof before);
    assert(UmiStudioAiPlatformConfigValidate(config) == expected);
    assert(memcmp(&before, config, sizeof before) == 0);
    if (expected != UMI_STATUS_OK) {
        assert(umi_studio_ai_platform_create_configured(config, &out) == expected);
        assert(out == sentinel);
        assert(memcmp(&before, config, sizeof before) == 0);
    }
    free(sentinel);
}

int main(int argc, char **argv)
{
    static const char *const modes[] = {
        "defaults", "unterminated", "required-empty", "optional-empty", "last-terminators",
        "remote-provider", "remote-model", "remote-secret", "remote-scheme",
        "remote-disabled", "remote-unconfigured", "remote-complete", "flags",
        "fitness-nan", "fitness-pos-inf", "fitness-neg-inf", "fitness-low", "fitness-high",
        "fitness-zero", "fitness-one", "numeric-bounds", "null-config", "null-output"
    };
    UmiStudioAiPlatformConfig config = umi_studio_ai_platform_config_default();
    int known = 0;
    assert(argc == 2);
    for (size_t i = 0U; i < sizeof modes / sizeof modes[0]; ++i)
        if (strcmp(argv[1], modes[i]) == 0) known = 1;
    assert(known);
#define CASE(name) (strcmp(argv[1], name) == 0)
    if (CASE("null-config")) {
        UmiStudioAiPlatform *out = NULL;
        assert(UmiStudioAiPlatformConfigValidate(NULL) == UMI_STATUS_INVALID_ARGUMENT);
        assert(umi_studio_ai_platform_create_configured(NULL, &out) == UMI_STATUS_INVALID_ARGUMENT);
        assert(out == NULL);
    } else if (CASE("null-output")) {
        assert(umi_studio_ai_platform_create_configured(&config, NULL) == UMI_STATUS_INVALID_ARGUMENT);
    } else if (CASE("unterminated") || CASE("required-empty") ||
               CASE("optional-empty") || CASE("last-terminators")) {
        for (size_t i = 0U; i < sizeof fields / sizeof fields[0]; ++i) {
            char *field;
            config = umi_studio_ai_platform_config_default();
            field = (char *)&config + fields[i].offset;
            if (CASE("unterminated")) {
                memset(field, 'x', fields[i].capacity);
                check(&config, UMI_STATUS_INVALID_ARGUMENT);
            } else if (CASE("last-terminators")) {
                memset(field, 'x', fields[i].capacity);
                field[fields[i].capacity - 1U] = '\0';
                /* Remote fields remain inert while the remote policy is off. */
                config.allow_remote = 0;
                check(&config, UMI_STATUS_OK);
            } else if ((CASE("required-empty") && fields[i].required) ||
                       (CASE("optional-empty") && !fields[i].required)) {
                field[0] = '\0';
                check(&config, fields[i].required ? UMI_STATUS_INVALID_ARGUMENT : UMI_STATUS_OK);
            }
        }
    } else if (CASE("numeric-bounds")) {
        for (unsigned i = 0U; i < 13U; ++i) {
            config = umi_studio_ai_platform_config_default();
            switch (i) {
                case 0: config.context_tokens = 0U; break;
                case 1: config.reserved_output_tokens = config.context_tokens; break;
                case 2: config.coding_context_tokens = 0U; break;
                case 3: config.maximum_patch_lines = 0U; break;
                case 4: config.maximum_patch_files = 0U; break;
                case 5: config.maximum_patch_files = UMI_AI_CODING_PATCH_FILE_MAX + 1U; break;
                case 6: config.knowledge_source_capacity = 0U; break;
                case 7: config.knowledge_vector_capacity = 0U; break;
                case 8: config.knowledge_chunk_bytes = 0U; break;
                case 9: config.knowledge_chunk_bytes = UMI_KNOWLEDGE_TEXT_CAPACITY; break;
                case 10: config.knowledge_overlap_bytes = config.knowledge_chunk_bytes; break;
                case 11: config.knowledge_result_limit = 0U; break;
                default: config.helix_maximum_attempts = 0U; break;
            }
            check(&config, UMI_STATUS_INVALID_ARGUMENT);
        }
        config = umi_studio_ai_platform_config_default();
        config.knowledge_result_limit = UMI_KNOWLEDGE_QUERY_RESULT_MAX + 1U;
        check(&config, UMI_STATUS_INVALID_ARGUMENT);
    } else {
        UmiStatus expected = UMI_STATUS_OK;
        if (strncmp(argv[1], "remote-", 7U) == 0) {
            config.allow_remote = 1;
            strcpy(config.remote_provider_id, "provider.remote");
            strcpy(config.remote_endpoint, "https://example.invalid/api");
            strcpy(config.remote_model_id, "model.remote");
            strcpy(config.remote_secret_reference, "secret://example");
            if (CASE("remote-provider")) config.remote_provider_id[0] = '\0';
            if (CASE("remote-model")) config.remote_model_id[0] = '\0';
            if (CASE("remote-secret")) config.remote_secret_reference[0] = '\0';
            if (CASE("remote-scheme")) strcpy(config.remote_secret_reference, "no-scheme");
            if (CASE("remote-disabled")) {
                config.allow_remote = 0;
                config.remote_provider_id[0] = '\0'; config.remote_secret_reference[0] = '\0';
            }
            if (CASE("remote-unconfigured")) config.remote_endpoint[0] = '\0';
            if (CASE("remote-provider") || CASE("remote-model") ||
                CASE("remote-secret") || CASE("remote-scheme")) expected = UMI_STATUS_INVALID_ARGUMENT;
        }
        if (CASE("flags")) {
            config.allow_remote = -1; config.remote_endpoint[0] = '\0';
            config.persist_sessions = -1; config.rag_enabled = 2; config.stream_responses = -1;
        }
        if (CASE("fitness-nan")) config.helix_minimum_fitness = NAN;
        if (CASE("fitness-pos-inf")) config.helix_minimum_fitness = INFINITY;
        if (CASE("fitness-neg-inf")) config.helix_minimum_fitness = -INFINITY;
        if (CASE("fitness-low")) config.helix_minimum_fitness = -0.001;
        if (CASE("fitness-high")) config.helix_minimum_fitness = 1.001;
        if (CASE("fitness-zero")) config.helix_minimum_fitness = 0.0;
        if (CASE("fitness-one")) config.helix_minimum_fitness = 1.0;
        if (CASE("fitness-nan") || CASE("fitness-pos-inf") || CASE("fitness-neg-inf") ||
            CASE("fitness-low") || CASE("fitness-high")) expected = UMI_STATUS_INVALID_ARGUMENT;
        check(&config, expected);
    }
    return 0;
#undef CASE
}
