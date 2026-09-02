/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/core/include/proj_templates.h
 *
 * PURPOSE:
 *   Declare the proj templates contract shared by Framework services and thin
 *   applications.
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
 * File: src/proj_templates.h
 * PURPOSE: Generate minimal project templates in a directory
 * Created by: Umicom Foundation | Author: Sammy Hegab | Date: 2025-10-01 | MIT
 *---------------------------------------------------------------------------*/

#ifndef UMICOM_PROJ_TEMPLATES_H
#define UMICOM_PROJ_TEMPLATES_H

#include <glib.h>

typedef enum { UMI_TPL_C, UMI_TPL_CPP, UMI_TPL_PY } UmiTemplateKind;

/**
 * Write tpl in its stable representation and report capacity or input failures to the
 * caller.
 */
gboolean umi_tpl_write(const char *dir, UmiTemplateKind kind, GError **err);

#endif /* UMICOM_PROJ_TEMPLATES_H */
