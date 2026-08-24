/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/runtime_helpers.inc
 *
 * PURPOSE:
 *   Small bounded GTK/runtime helpers shared by the private Studio host.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

static void runtime_clear_box(GtkWidget *box)
{
    GtkWidget *child;

    if (box == NULL) return;

    child = gtk_widget_get_first_child(box);
    while (child != NULL) {
        GtkWidget *next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(box), child);
        child = next;
    }
}

static void runtime_clear_list_box(GtkWidget *list)
{
    GtkWidget *child;

    if (list == NULL) return;

    child = gtk_widget_get_first_child(list);
    while (child != NULL) {
        GtkWidget *next = gtk_widget_get_next_sibling(child);
        gtk_list_box_remove(GTK_LIST_BOX(list), child);
        child = next;
    }
}

static GtkWidget *runtime_label(const char *text, float xalign)
{
    GtkWidget *label = gtk_label_new(text != NULL ? text : "");
    if (label != NULL) {
        gtk_label_set_xalign(GTK_LABEL(label), xalign);
        gtk_label_set_wrap(GTK_LABEL(label), TRUE);
        gtk_widget_set_hexpand(label, TRUE);
    }
    return label;
}

static GtkWidget *runtime_section_title(const char *title)
{
    GtkWidget *label = runtime_label(title, 0.0F);

    if (label != NULL) {
        gtk_widget_add_css_class(label, "umicom-runtime-section-title");
    }
    return label;
}

static GtkWidget *runtime_card_new(const char *title, const char *body)
{
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    GtkWidget *heading = runtime_section_title(title);
    GtkWidget *detail = runtime_label(body, 0.0F);

    if (box == NULL || heading == NULL || detail == NULL) return box;

    gtk_widget_add_css_class(box, "umicom-runtime-card");
    gtk_box_append(GTK_BOX(box), heading);
    gtk_box_append(GTK_BOX(box), detail);
    return box;
}

static GtkWidget *runtime_scroll(GtkWidget *child)
{
    GtkWidget *scroll = gtk_scrolled_window_new();

    if (scroll == NULL) return NULL;

    gtk_scrolled_window_set_policy(
        GTK_SCROLLED_WINDOW(scroll),
        GTK_POLICY_AUTOMATIC,
        GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), child);
    gtk_widget_set_hexpand(scroll, TRUE);
    gtk_widget_set_vexpand(scroll, TRUE);
    return scroll;
}

static int runtime_command_enabled(
    UmiStudioGtkRuntimeChrome *runtime,
    const char *command_id)
{
    UmiStudioRuntimeBindings *bindings;

    if (runtime == NULL ||
        runtime->runtime_bootstrap == NULL ||
        runtime->runtime_bootstrap->platform == NULL ||
        command_id == NULL ||
        command_id[0] == '\0') {
        return 0;
    }

    bindings = umi_studio_runtime_platform_bindings(
        runtime->runtime_bootstrap->platform);
    if (bindings == NULL || bindings->commands == NULL) return 0;

    return umi_command_registry_contains(bindings->commands, command_id) &&
        umi_command_registry_is_enabled(bindings->commands, command_id, NULL);
}

static const char *runtime_safe(const char *text)
{
    return text != NULL ? text : "";
}

static uint32_t runtime_bounded_badge(size_t value)
{
    return value > UINT32_MAX ? UINT32_MAX : (uint32_t)value;
}
