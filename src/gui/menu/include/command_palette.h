/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/menu/include/command_palette.h
 *
 * PURPOSE:
 *   Declare the command palette contract shared by Framework services and thin
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
 * File: src/gui/menu/include/command_palette.h
 * PURPOSE: Command palette dialog with filterable list of commands (GTK4, C)
 * Created by: Umicom Foundation | Author: Sammy Hegab | Date: 2025-10-01 | MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_COMMAND_PALETTE_H
#define UMICOM_COMMAND_PALETTE_H

#include <gtk/gtk.h>

/**
 * Provide the void operation used by this module and its client applications.
 */
G_BEGIN_DECLS

typedef void (*UmiCommandFn)(gpointer user);

typedef struct {
  const char *name;     /* Button label / action name       */
  const char *hint;     /* Tooltip / subtitle (optional)    */
  UmiCommandFn cb;      /* Callback invoked on activation   */
} UmiCommand;

/**
 * Represent the palette data shared with callers of this public contract.
 */
typedef struct _UmiPalette UmiPalette;

/**
 * Provide the palette new operation used by this module and its client applications.
 */
UmiPalette *umi_palette_new (const UmiCommand *table, gpointer user);
/**
 * Provide the palette open operation used by this module and its client applications.
 */
void        umi_palette_open(UmiPalette *p, GtkWindow *parent);
/**
 * Provide the palette free operation used by this module and its client applications.
 */
void        umi_palette_free(UmiPalette *p);

G_END_DECLS
#endif /* UMICOM_COMMAND_PALETTE_H */
