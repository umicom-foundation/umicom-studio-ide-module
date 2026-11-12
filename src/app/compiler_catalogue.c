/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/app/compiler_catalogue.c
 *
 * PURPOSE:
 *   Implement the compiler catalogue behavior for
 *   Umicom Studio IDE.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/compiler_catalogue.h"
#include "umicom/compiler/language.h"
#include "umicom/base/text.h"
#include <stdlib.h>
#include <string.h>
/* Unchecked catalogue projection and lookup are superseded by full source/view admission and staged presentation publication. Retain the original rows and language formatting logic for review. The previous implementation is retained for review. */
#if 0
/* Provide the append language operation used by this module and its client applications. */
static void append_language(char *output,size_t capacity,UmiCompilerLanguage language,bool *first)
{ size_t length = strlen(output); const char *name = umi_compiler_language_name(language); /* Keep the operation inside its valid bounds before reading, writing or adding data. */ if (length >= capacity) return; (void)snprintf(output + length,capacity - length,"%s%s",*first ? "" : ", ",name); *first = false; }
/*
 * Provide the studio compiler catalogue project operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_compiler_catalogue_project(const UmiCompilerRegistry *registry,const char *selected_provider,UmiStudioCompilerCatalogue *out_catalogue)
{
    size_t index; /* Protect caller-owned memory by checking that required state is available before it is used. */ if (registry == NULL || out_catalogue == NULL) return UMI_STATUS_INVALID_ARGUMENT; (void)memset(out_catalogue,0,sizeof(*out_catalogue));
    /* Visit each bounded item once so every record receives the same rule. */
    for (index = 0U; index < registry->count && index < UMI_STUDIO_COMPILER_CATALOGUE_MAX; ++index) { const UmiCompilerProvider *provider = &registry->items[index]; UmiStudioCompilerCatalogueRow *row = &out_catalogue->rows[out_catalogue->count++]; UmiCompilerLanguage language; bool first = true; (void)snprintf(row->provider_id,sizeof(row->provider_id),"%s",provider->descriptor.provider_id); (void)snprintf(row->name,sizeof(row->name),"%s",provider->descriptor.display_name); (void)snprintf(row->family,sizeof(row->family),"%s",umi_compiler_family_name(provider->descriptor.family)); (void)snprintf(row->executable,sizeof(row->executable),"%s",provider->descriptor.executable); /* Visit each bounded item once so every record receives the same rule. */ for (language = UMI_COMPILER_LANGUAGE_C; language <= UMI_COMPILER_LANGUAGE_UAI; language = (UmiCompilerLanguage)(language + 1)) /* Protect caller-owned memory by checking that required state is available before it is used. */ if (umi_compiler_provider_supports(provider,language)) append_language(row->languages,sizeof(row->languages),language,&first); row->available = provider->descriptor.available; row->selected = selected_provider != NULL && strcmp(selected_provider,row->provider_id) == 0; }
    out_catalogue->revision = registry->revision; return UMI_STATUS_OK;
}
/*
 * Find studio compiler catalogue while leaving the underlying catalogue or model owned by
 * this module.
 */
const UmiStudioCompilerCatalogueRow *umi_studio_compiler_catalogue_find(const UmiStudioCompilerCatalogue *catalogue,const char *provider_id)
{ size_t index; /* Protect caller-owned memory by checking that required state is available before it is used. */ if (catalogue == NULL || provider_id == NULL) return NULL; /* Visit each bounded item once so every record receives the same rule. */ for (index = 0U; index < catalogue->count; ++index) /* Protect caller-owned memory by checking that required state is available before it is used. */ if (strcmp(catalogue->rows[index].provider_id,provider_id) == 0) return &catalogue->rows[index]; return NULL; }

#endif
/* Studio owns presentation rows; Framework supplies registry admission, language
 * names and complete text operations. Validate the entire source before projection
 * and publish a staged view so an error never exposes a partially filled panel. */
UmiStatus umi_studio_compiler_catalogue_project(const UmiCompilerRegistry *registry,
    const char *selected_provider,UmiStudioCompilerCatalogue *out_catalogue)
{
    if (registry == NULL || out_catalogue == NULL) return UMI_STATUS_INVALID_ARGUMENT;
    const uintptr_t source = (uintptr_t)(const void *)registry;
    const uintptr_t output = (uintptr_t)(void *)out_catalogue;
    if (output >= source ? output - source < sizeof(*registry) : source - output < sizeof(*out_catalogue))
        return UMI_STATUS_INVALID_ARGUMENT;
    char selected[UMI_COMPILER_ID_CAPACITY] = {0};
    UmiStatus status = UMI_STATUS_OK;
    if (selected_provider != NULL) status = umi_text_replace(selected,sizeof(selected),selected_provider);
    if (status != UMI_STATUS_OK) return status;
    if (!umi_compiler_registry_valid(registry)) return UMI_STATUS_INVALID_STATE;
    if (registry->count > UMI_STUDIO_COMPILER_CATALOGUE_MAX) return UMI_STATUS_CAPACITY_EXCEEDED;
    UmiStudioCompilerCatalogue *candidate = calloc(1U,sizeof(*candidate));
    if (candidate == NULL) return UMI_STATUS_OUT_OF_MEMORY;
    for (size_t index = 0U; index < registry->count; ++index) {
        const UmiCompilerProvider *provider = &registry->items[index];
        UmiStudioCompilerCatalogueRow *row = &candidate->rows[index];
        status = umi_text_replace(row->provider_id,sizeof(row->provider_id),provider->descriptor.provider_id);
        if (status == UMI_STATUS_OK) status = umi_text_replace(row->name,sizeof(row->name),provider->descriptor.display_name);
        if (status == UMI_STATUS_OK) status = umi_text_replace(row->family,sizeof(row->family),umi_compiler_family_name(provider->descriptor.family));
        if (status == UMI_STATUS_OK) status = umi_text_replace(row->executable,sizeof(row->executable),provider->descriptor.executable);
        bool first = true;
        for (UmiCompilerLanguage language = UMI_COMPILER_LANGUAGE_C;
             status == UMI_STATUS_OK && language <= UMI_COMPILER_LANGUAGE_UAI;
             language = (UmiCompilerLanguage)(language+1)) {
            if (!umi_compiler_provider_supports(provider,language)) continue;
            if (!first) status = umi_text_append(row->languages,sizeof(row->languages),", ");
            if (status == UMI_STATUS_OK)
                status = umi_text_append(row->languages,sizeof(row->languages),umi_compiler_language_name(language));
            first = false;
        }
        if (status != UMI_STATUS_OK) break;
        row->available = provider->descriptor.available;
        row->selected = selected[0] != '\0' && strcmp(selected,row->provider_id) == 0;
        candidate->count += 1U;
    }
    if (status == UMI_STATUS_OK) {
        candidate->revision = registry->revision;
        (void)memcpy(out_catalogue,candidate,sizeof(*candidate));
    }
    free(candidate);
    return status;
}

/* Catalogue rows can be retained by UI clients. Admit every active row, not just
 * the first matching identifier, before returning a borrowed display record. */
bool umi_studio_compiler_catalogue_valid(const UmiStudioCompilerCatalogue *catalogue)
{
    if (catalogue == NULL || catalogue->count > UMI_STUDIO_COMPILER_CATALOGUE_MAX) return false;
    for (size_t index = 0U; index < catalogue->count; ++index) {
        const UmiStudioCompilerCatalogueRow *row = &catalogue->rows[index];
        if (row->provider_id[0] == '\0' ||
            memchr(row->provider_id,'\0',sizeof(row->provider_id)) == NULL ||
            memchr(row->name,'\0',sizeof(row->name)) == NULL ||
            memchr(row->family,'\0',sizeof(row->family)) == NULL ||
            memchr(row->executable,'\0',sizeof(row->executable)) == NULL ||
            memchr(row->languages,'\0',sizeof(row->languages)) == NULL) return false;
    }
    return true;
}
const UmiStudioCompilerCatalogueRow *umi_studio_compiler_catalogue_find(
    const UmiStudioCompilerCatalogue *catalogue,const char *provider_id)
{
    char identity[UMI_COMPILER_ID_CAPACITY] = {0};
    if (provider_id == NULL || provider_id[0] == '\0' ||
        umi_text_replace(identity,sizeof(identity),provider_id) != UMI_STATUS_OK ||
        !umi_studio_compiler_catalogue_valid(catalogue)) return NULL;
    for (size_t index = 0U; index < catalogue->count; ++index)
        if (strcmp(catalogue->rows[index].provider_id,identity) == 0) return &catalogue->rows[index];
    return NULL;
}
