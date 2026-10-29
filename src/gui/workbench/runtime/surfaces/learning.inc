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

#ifdef UMICOM_EDUCATION_WORKSPACE_GTK4
#include "umicom/ui/gtk4/education_workspace.h"
#endif

#include "learning_location.inc"

#ifdef UMICOM_STUDIO_HAS_LEARNING_CATALOGUE
#include "umicom/ui/gtk4/learning_catalogue.h"
#include "umicom/ui/gtk4/learning_help_binding.h"
/* Studio supplies a weak owner and the shared Framework compatibility version.
 * Catalogue storage, verification, searching and reading remain Framework
 * responsibilities; this action neither changes the project nor grants trust. */
/* The original shared-window launcher is retained for review. The replacement
 * adds a copied contextual-help policy while retaining catalogue reuse. */
#if 0
static void RuntimeLearningCatalogue(GtkButton *button, gpointer context)
{
    (void)button;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(context, &window);
    if (runtime != NULL) {
        UmiVersion version = {
            UMICOM_FRAMEWORK_VERSION_MAJOR,
            UMICOM_FRAMEWORK_VERSION_MINOR,
            UMICOM_FRAMEWORK_VERSION_PATCH
        };
        /* Reopening the learning surface retains this Studio window's shelf
         * and reading history. Framework owns the shared window lifetime. */
#if 0
        (void)UmiLearningCatalogueGtk4Present(runtime->owner->window, version);
#endif
        (void)UmiLearningCatalogueGtk4PresentShared(runtime->owner->window, version);
    }
    g_clear_object(&window);
}
#endif

/* Framework owns the shared catalogue, copied help route and keyboard binding.
 * Studio supplies only its reviewed guide identity. Pack admission remains an
 * explicit user choice, independent of the active project and working folder. */
/* Preserve the single-guide launcher for review. Explicit product routes now
 * reuse its shared window and keep its original F1 binding unchanged. */
#if 0
static void RuntimeLearningCatalogueAction(gpointer context, int open_help)
{
    GObject *owner_window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(context, &owner_window);
    if (runtime != NULL) {
        UmiVersion version = { UMICOM_FRAMEWORK_VERSION_MAJOR,
            UMICOM_FRAMEWORK_VERSION_MINOR, UMICOM_FRAMEWORK_VERSION_PATCH };
        GtkWindow *window = UmiLearningCatalogueGtk4PresentShared(runtime->owner->window, version);
        if (window != NULL) {
            g_object_ref(window);
            GtkWidget *panel = gtk_window_get_child(window);
            if (panel != NULL) {
                g_object_ref(panel);
                UmiLearningHelpRoute route = {0};
                (void)snprintf(route.request.product, sizeof(route.request.product), "studio");
                (void)snprintf(route.request.locale, sizeof(route.request.locale), "en");
                route.request.kind = UMI_LEARNING_HELP_TOPIC;
                (void)snprintf(route.request.key, sizeof(route.request.key), "learning");
                (void)snprintf(route.pack_id, sizeof(route.pack_id), "studio.learning-guide");
                (void)snprintf(route.resource_id, sizeof(route.resource_id), "learning-in-studio");
                UmiStatus status = UmiLearningHelpGtk4Bind(panel, panel, &route);
                if (open_help && (status == UMI_STATUS_OK || status == UMI_STATUS_ALREADY_EXISTS))
                    (void)UmiLearningHelpGtk4Open(panel);
                g_object_unref(panel);
            }
            g_object_unref(window);
        }
    }
    g_clear_object(&owner_window);
}
#endif
static void RuntimeLearningCatalogueRoute(gpointer context, int open_help,
    const char *topic, const char *resource)
{
    GObject *owner_window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(context, &owner_window);
    if (runtime != NULL) {
        UmiVersion version = { UMICOM_FRAMEWORK_VERSION_MAJOR,
            UMICOM_FRAMEWORK_VERSION_MINOR, UMICOM_FRAMEWORK_VERSION_PATCH };
        GtkWindow *window = UmiLearningCatalogueGtk4PresentShared(runtime->owner->window, version);
        if (window != NULL) {
            g_object_ref(window);
            GtkWidget *panel = gtk_window_get_child(window);
            if (panel != NULL) {
                g_object_ref(panel);
                UmiLearningHelpRoute route = {0};
                (void)snprintf(route.request.product, sizeof(route.request.product), "studio");
                (void)snprintf(route.request.locale, sizeof(route.request.locale), "en");
                route.request.kind = UMI_LEARNING_HELP_TOPIC;
                (void)snprintf(route.request.key, sizeof(route.request.key), "learning");
                (void)snprintf(route.pack_id, sizeof(route.pack_id), "studio.learning-guide");
                (void)snprintf(route.resource_id, sizeof(route.resource_id), "learning-in-studio");
                UmiStatus status = UmiLearningHelpGtk4Bind(panel, panel, &route);
                if (open_help && (status == UMI_STATUS_OK || status == UMI_STATUS_ALREADY_EXISTS)) {
                    if (topic == NULL || resource == NULL)
                        (void)UmiLearningHelpGtk4Open(panel);
                    else {
                        /* Product buttons select explicit identities without
                         * replacing the catalogue's existing F1 help policy. */
                        (void)snprintf(route.request.key,sizeof(route.request.key),"%s",topic);
                        (void)snprintf(route.resource_id,sizeof(route.resource_id),"%s",resource);
                        (void)UmiLearningHelpGtk4OpenRoute(panel,&route);
                    }
                }
                g_object_unref(panel);
            }
            g_object_unref(window);
        }
    }
    g_clear_object(&owner_window);
}
static void RuntimeLearningCatalogueAction(gpointer context,int open_help)
{
    RuntimeLearningCatalogueRoute(context,open_help,NULL,NULL);
}
typedef struct RuntimeLearningProductGuide {
    const char *title,*topic,*resource,*tag;
} RuntimeLearningProductGuide;
static const RuntimeLearningProductGuide runtime_learning_product_guides[]={
    {"Read the editing and saving guide","editing","editing-files","studio.learning.editing-guide"},
    {"Read the compiler error guide","build-errors","reading-build-errors","studio.learning.errors-guide"},
    {"Read the testing guide","testing","running-tests","studio.learning.tests-guide"}
};
static void RuntimeLearningOpenProductGuide(GtkButton *button,gpointer context)
{
    guint choice=GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(button),"studio-learning-guide-choice"));
    if(choice==0U || choice>sizeof(runtime_learning_product_guides)/sizeof(runtime_learning_product_guides[0]))return;
    /* These compile-time route strings remain valid through window callbacks;
     * no caller document text or transient widget data becomes guide policy. */
    const RuntimeLearningProductGuide *guide=&runtime_learning_product_guides[choice-1U];
    RuntimeLearningCatalogueRoute(context,1,guide->topic,guide->resource);
}

static void RuntimeLearningCatalogue(GtkButton *button, gpointer context)
{
    (void)button;
    RuntimeLearningCatalogueAction(context, 0);
}
static void RuntimeLearningGuide(GtkButton *button, gpointer context)
{
    (void)button;
    RuntimeLearningCatalogueAction(context, 1);
}

#endif

#ifdef UMICOM_EDUCATION_WORKSPACE_GTK4
/* Education can remain open after Studio closes. Resolve a weak owner at the
 * explicit adoption action; the practicum never keeps a raw runtime pointer. */
static void RuntimeLearningOpenExported(const UmiEducationProjectWorkflow *project, void *context)
{
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(context, &window);
    if (runtime != NULL)
    {
        UmiStatus status = RuntimeProjectCreated(&project->project, &project->build, runtime);
        /* Adoption can deliver host notifications. Resolve the binding again
         * before presenting any additional message through the runtime. */
        GObject *currentWindow = NULL;
        runtime = runtime_file_dialog_owner(context, &currentWindow);
        if (runtime != NULL)
        {
            if (status != UMI_STATUS_OK)
                runtime_show_notification(runtime, status == UMI_STATUS_BUSY
                    ? "The lesson files are exported. Stop the active build or debugger, then choose Open last exported project again."
                    : "The lesson files remain in the selected folder. Studio could not finish opening or saving their settings; review the project and try again.");
            else if (project->requires_framework_sdk)
                runtime_show_notification(runtime,
                    "Framework lesson opened. Select your installed Framework data SDK in CMake settings before configuring. Review project trust separately.");
        }
        g_clear_object(&currentWindow);
    }
    g_clear_object(&window);
}
static void RuntimeLearningPracticum(GtkButton *button, gpointer context)
{
    (void)button;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(context, &window);
    if (runtime != NULL)
    {
        UmiStudioFileDialogRequest *binding = runtime_file_dialog_request_new(runtime, 0U);
        if (binding != NULL)
        {
            UmiStatus status = UmiEducationGtkOpenWithProjectOpener(runtime->owner->window,
                RuntimeLearningOpenExported, binding, RuntimeLearningActionFree);
            if (status != UMI_STATUS_OK) runtime_file_dialog_request_free(binding);
        }
    }
    g_clear_object(&window);
}
#endif


/* Open a lesson resource in the shared documentation surface. */
/* Lesson resources were resolved against the current project, which can be private application storage at startup. The selected Framework library now owns resolution; the former route remains for review. The previous implementation is retained for engineering review. */
#if 0
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
#endif
/* Lesson launchers use the same weak workbench binding as the location controls, retaining the earlier callback for review. The previous implementation is retained for engineering review. */
#if 0
static void runtime_on_learning_resource(GtkButton *button, gpointer user_data)
{
    UmiStudioGtkRuntimeChrome *runtime = user_data;
    const char *resource = g_object_get_data(G_OBJECT(button), "umicom-learning-resource");
    if (runtime == NULL || resource == NULL || resource[0] == '\0') return;
    /* A catalogue-relative path is meaningful only inside a chosen library.
     * Asking here also makes the first lesson usable without a setup detour. */
    if (runtime->learning_directory[0] == '\0') RuntimeLearningChoose(runtime, resource);
    else (void)RuntimeLearningOpen(runtime, resource);
}
#endif
static void runtime_on_learning_resource(GtkButton *button, gpointer user_data)
{
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(user_data, &window);
    const char *resource = g_object_get_data(G_OBJECT(button), "umicom-learning-resource");
    /* Closing Studio invalidates this binding even when a caller retains a
     * lesson button. A lesson path cannot recreate its former application. */
    if (runtime != NULL && resource != NULL && resource[0] != '\0') {
        if (runtime->learning_directory[0] == '\0') RuntimeLearningChoose(runtime, resource);
        else (void)RuntimeLearningOpen(runtime, resource);
    }
    g_clear_object(&window);
}

/* Compose one lesson card while keeping curriculum data in Framework. */
/* Bind lesson-resource actions to a weak owner so a retained control cannot dereference a released Studio runtime. The previous implementation is retained for engineering review. */
#if 0
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
#endif
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
        RuntimeLearningBind(resource_button, G_CALLBACK(runtime_on_learning_resource), runtime);
        gtk_widget_set_halign(resource_button, GTK_ALIGN_START);
        gtk_box_append(GTK_BOX(content), resource_button);
    }
    gtk_expander_set_child(GTK_EXPANDER(expander), content);
    return expander;
}

/* Build the Studio projection of the shared lessons catalogue. */
/* Bind lesson-resource actions to a weak owner so a retained control cannot dereference a released Studio runtime. The previous implementation is retained for engineering review. */
#if 0
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
    RuntimeLearningLocations(runtime, panel);
#ifdef UMICOM_EDUCATION_WORKSPACE_GTK4
    /* Framework owns saved learning records and the practicum. This launcher
     * adds that capability without replacing the existing lesson cards, wiki,
     * documentation navigation or Studio's current learning summary. */
    gtk_box_append(GTK_BOX(panel), UmiEducationGtkLauncher());
#endif
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
#endif
/* Give the shared reference action a stable semantic identity without depending on its translated label or widget position. The previous implementation is retained for engineering review. */
#if 0
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
    RuntimeLearningLocations(runtime, panel);
#ifdef UMICOM_EDUCATION_WORKSPACE_GTK4
    /* Framework owns saved learning records and the practicum. This launcher
     * adds that capability without replacing the existing lesson cards, wiki,
     * documentation navigation or Studio's current learning summary. */
    gtk_box_append(GTK_BOX(panel), UmiEducationGtkLauncher());
#endif
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
        RuntimeLearningBind(reference_button, G_CALLBACK(runtime_on_learning_resource), runtime);
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
#endif
/* The practicum now receives a weak Studio project opener so a completed export can be adopted with its real build metadata. Keep the previous standalone-only launcher composition for review. The previous implementation is retained for engineering review. */
#if 0
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
    RuntimeLearningLocations(runtime, panel);
#ifdef UMICOM_EDUCATION_WORKSPACE_GTK4
    /* Framework owns saved learning records and the practicum. This launcher
     * adds that capability without replacing the existing lesson cards, wiki,
     * documentation navigation or Studio's current learning summary. */
    gtk_box_append(GTK_BOX(panel), UmiEducationGtkLauncher());
#endif
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
        /* Keep a semantic identity for keyboard and retained-control coverage. */
        (void)umi_gtk4_automation_tag_widget(reference_button, "studio.learning.reference");
        g_object_set_data_full(
            G_OBJECT(reference_button),
            "umicom-learning-resource",
            g_strdup(c_lesson.resource_path),
            g_free);
        RuntimeLearningBind(reference_button, G_CALLBACK(runtime_on_learning_resource), runtime);
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
#endif
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
    RuntimeLearningLocations(runtime, panel);
#ifdef UMICOM_STUDIO_HAS_LEARNING_CATALOGUE
    /* Keep existing curriculum and practicum controls while exposing portable
     * verified packs through the common native reader. */
    GtkWidget *catalogue = gtk_button_new_with_label("Open local learning catalogue");
    (void)umi_gtk4_automation_tag_widget(catalogue, "studio.learning.catalogue");
    RuntimeLearningBind(catalogue, G_CALLBACK(RuntimeLearningCatalogue), runtime);
    gtk_box_append(GTK_BOX(panel), catalogue);
    GtkWidget *guide = gtk_button_new_with_label("Read Studio learning guide");
    (void)umi_gtk4_automation_tag_widget(guide, "studio.learning.guide");
    RuntimeLearningBind(guide, G_CALLBACK(RuntimeLearningGuide), runtime);
    gtk_box_append(GTK_BOX(panel), guide);
    for(guint i=0U;i<sizeof(runtime_learning_product_guides)/sizeof(runtime_learning_product_guides[0]);++i) {
        const RuntimeLearningProductGuide *entry=&runtime_learning_product_guides[i];
        GtkWidget *product_guide=gtk_button_new_with_label(entry->title);
        (void)umi_gtk4_automation_tag_widget(product_guide,entry->tag);
        g_object_set_data(G_OBJECT(product_guide),"studio-learning-guide-choice",GUINT_TO_POINTER(i+1U));
        RuntimeLearningBind(product_guide,G_CALLBACK(RuntimeLearningOpenProductGuide),runtime);
        gtk_box_append(GTK_BOX(panel),product_guide);
    }
#endif
#ifdef UMICOM_EDUCATION_WORKSPACE_GTK4
    /* Framework owns saved learning records and the practicum. This launcher
     * adds that capability without replacing the existing lesson cards, wiki,
     * documentation navigation or Studio's current learning summary. */
    GtkWidget *practicum = gtk_button_new_with_label("Open learning practicum and saved progress");
    (void)umi_gtk4_automation_tag_widget(practicum, "education.launcher");
    RuntimeLearningBind(practicum, G_CALLBACK(RuntimeLearningPracticum), runtime);
    gtk_box_append(GTK_BOX(panel), practicum);
#endif
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
        /* Keep a semantic identity for keyboard and retained-control coverage. */
        (void)umi_gtk4_automation_tag_widget(reference_button, "studio.learning.reference");
        g_object_set_data_full(
            G_OBJECT(reference_button),
            "umicom-learning-resource",
            g_strdup(c_lesson.resource_path),
            g_free);
        RuntimeLearningBind(reference_button, G_CALLBACK(runtime_on_learning_resource), runtime);
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
