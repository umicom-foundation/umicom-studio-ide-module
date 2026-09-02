/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/compiler_templates.h
 *
 * PURPOSE:
 *   Publish Studio's public compiler templates contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_COMPILER_TEMPLATES_H
#define UMICOM_STUDIO_COMPILER_TEMPLATES_H
#include "umicom/compiler/compiler.h"
/**
 * Represent the studio compiler template data shared with callers of this public contract.
 */
typedef struct UmiStudioCompilerTemplate { const char *template_id; const char *name; const char *description; const char *relative_path; uint32_t language_mask; bool polyglot; bool requires_umicc; } UmiStudioCompilerTemplate;
/**
 * Return the number of records represented by studio compiler templates without changing
 * their state.
 */
size_t umi_studio_compiler_templates_count(void);
/**
 * Find studio compiler templates while leaving the underlying catalogue or model owned by
 * this module.
 */
const UmiStudioCompilerTemplate *umi_studio_compiler_templates_at(size_t index);
/**
 * Find studio compiler templates while leaving the underlying catalogue or model owned by
 * this module.
 */
const UmiStudioCompilerTemplate *umi_studio_compiler_templates_find(const char *template_id);
#endif
