/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/gui/workbench/runtime/surfaces/learning.inc
 *
 * PURPOSE:
 *   Render the Framework-owned C and Umicom curriculum as a sequence of
 *   dockable, readable lesson cards.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* Open a lesson resource in the shared documentation surface. */
static void runtime_on_learning_resource(
    GtkButton *button,
    gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime =
        (UmiStudioGtkRuntimeChrome *)user_data;
    const char *resource = (const char *)g_object_get_data(
        G_OBJECT(button), "umicom-learning-resource");
    if (runtime == NULL || resource == NULL || resource[0] == '\0') return;
    (void)runtime_documentation_display_location(runtime, resource, 1);
    runtime_show_surface(runtime, UMI_STUDIO_SURFACE_DOCUMENTATION);
}

/* Compose one lesson card while keeping curriculum data in Framework. */
static GtkWidget *runtime_learning_lesson_card(
    UmiStudioGtkRuntimeChrome *runtime,
    const UmiStudioLearningLessonSnapshot *lesson)
{
    GtkWidget *expander;
    GtkWidget *content;
    GtkWidget *summary;
    GtkWidget *exercise_title;
    GtkWidget *exercise;
    GtkWidget *requirements;
    GtkWidget *resource_button;
    char heading[512];
    char requirement_text[512];

    if (runtime == NULL || lesson == NULL) return NULL;
    (void)snprintf(
        heading,
        sizeof(heading),
        "%u. %s · %u minutes",
        lesson->sequence,
        lesson->title,
        lesson->estimated_minutes);
    expander = gtk_expander_new(heading);
    content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    summary = runtime_label(lesson->summary, 0.0F);
    exercise_title = runtime_section_title("Practice");
    exercise = runtime_label(lesson->exercise, 0.0F);
    (void)snprintf(
        requirement_text,
        sizeof(requirement_text),
        "Stage: %s · Pass score: %u · Compiler: %s · Repository: %s · Account: %s",
        lesson->stage,
        lesson->required_score,
        lesson->requires_compiler ? "needed" : "not needed",
        lesson->requires_repository ? "needed" : "not needed",
        lesson->requires_github_account ? "needed" : "not needed");
    requirements = runtime_label(requirement_text, 0.0F);
    resource_button = lesson->resource_path[0] != '\0'
        ? gtk_button_new_with_label("Open lesson resource")
        : NULL;

    if (expander == NULL || content == NULL) return expander;
    if (summary != NULL) gtk_box_append(GTK_BOX(content), summary);
    if (exercise_title != NULL) {
        gtk_box_append(GTK_BOX(content), exercise_title);
    }
    if (exercise != NULL) gtk_box_append(GTK_BOX(content), exercise);
    if (requirements != NULL) {
        gtk_widget_add_css_class(requirements, "umicom-runtime-muted");
        gtk_box_append(GTK_BOX(content), requirements);
    }
    if (resource_button != NULL) {
        g_object_set_data_full(
            G_OBJECT(resource_button),
            "umicom-learning-resource",
            g_strdup(lesson->resource_path),
            g_free);
        g_signal_connect(
            resource_button,
            "clicked",
            G_CALLBACK(runtime_on_learning_resource),
            runtime);
        gtk_widget_set_halign(resource_button, GTK_ALIGN_START);
        gtk_box_append(GTK_BOX(content), resource_button);
    }
    gtk_expander_set_child(GTK_EXPANDER(expander), content);
    return expander;
}

/* Build the Studio projection of the shared lessons catalogue. */
static GtkWidget *runtime_build_surface_learning(
    UmiStudioGtkRuntimeChrome *runtime)
{
    GtkWidget *panel = runtime_panel_new("Guided Learning");
    UmiStudioLearningCentreSnapshot centre;
    UmiStudioLearningLessonSnapshot c_lesson;
    GtkWidget *reference_button = NULL;
    size_t index;
    char summary[512];

    if (panel == NULL || runtime == NULL) return panel;
    if (umi_studio_learning_centre_snapshot(&centre) != UMI_STATUS_OK) {
        gtk_box_append(
            GTK_BOX(panel),
            runtime_card_new(
                "Lessons unavailable",
                "The Framework curriculum could not be read."));
        return runtime_scroll(panel);
    }
    (void)snprintf(
        summary,
        sizeof(summary),
        "%s\n%zu lessons · about %u minutes",
        centre.summary,
        centre.lesson_count,
        centre.estimated_minutes);
    runtime->learning_summary_label = runtime_label(summary, 0.0F);
    if (runtime->learning_summary_label != NULL) {
        gtk_widget_add_css_class(
            runtime->learning_summary_label, "umicom-runtime-muted");
        gtk_box_append(GTK_BOX(panel), runtime->learning_summary_label);
    }

    /* Give the shared C wiki one stable entry point above the ordered lessons. */
    if (umi_studio_learning_centre_find_lesson(
            "foundations.c-hello", &c_lesson) == UMI_STATUS_OK &&
        c_lesson.resource_path[0] != '\0') {
        reference_button = gtk_button_new_with_label(
            "Open C language and safer programming reference");
    }
    if (reference_button != NULL) {
        g_object_set_data_full(
            G_OBJECT(reference_button),
            "umicom-learning-resource",
            g_strdup(c_lesson.resource_path),
            g_free);
        g_signal_connect(
            reference_button,
            "clicked",
            G_CALLBACK(runtime_on_learning_resource),
            runtime);
        gtk_widget_set_halign(reference_button, GTK_ALIGN_START);
        gtk_box_append(GTK_BOX(panel), reference_button);
    }

    for (index = 0U; index < centre.lesson_count; ++index) {
        UmiStudioLearningLessonSnapshot lesson;
        GtkWidget *card;
        if (umi_studio_learning_centre_lesson_at(index, &lesson) !=
            UMI_STATUS_OK) {
            continue;
        }
        card = runtime_learning_lesson_card(runtime, &lesson);
        if (card != NULL) gtk_box_append(GTK_BOX(panel), card);
    }
    return runtime_scroll(panel);
}
