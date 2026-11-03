/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/learning_commands.inc
 *
 * PURPOSE:
 *   Share local guide command identities between welcome controls and learning routes.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifdef UMICOM_STUDIO_HAS_LEARNING_CATALOGUE
typedef struct RuntimeLearningProductGuide {
    const char *title, *topic, *resource, *tag, *command;
    const char *product, *pack;
} RuntimeLearningProductGuide;
/* Only reviewed product identities belong here. Framework owns pack admission,
 * verified reads and history; a guide command cannot adopt or trust a project. */
/* Retain the Studio-only guide table for review. Explicit product and pack
 * identities let the same shared reader route to an independently chosen
 * course library without embedding a machine-specific folder in Studio. */
#if 0
static const RuntimeLearningProductGuide runtime_learning_product_guides[] = {
    {"Read the Studio user guide", "studio-workflows", "studio-user-guide",
     "studio.learning.user-guide", "help.learning.user-guide"},
    {"Read the editing and saving guide", "editing", "editing-files",
     "studio.learning.editing-guide", "help.learning.editing"},
    {"Read the compiler error guide", "build-errors", "reading-build-errors",
     "studio.learning.errors-guide", "help.learning.errors"},
    {"Read the testing guide", "testing", "running-tests",
     "studio.learning.tests-guide", "help.learning.tests"},
    {"Read the recent projects guide", "recent-projects", "recent-projects",
     "studio.learning.recent-guide", "help.learning.recent"}
};
#endif
static const RuntimeLearningProductGuide runtime_learning_product_guides[] = {
    {"Read the Studio user guide", "studio-workflows", "studio-user-guide",
     "studio.learning.user-guide", "help.learning.user-guide", "studio", "studio.learning-guide"},
    {"Read the editing and saving guide", "editing", "editing-files",
     "studio.learning.editing-guide", "help.learning.editing", "studio", "studio.learning-guide"},
    {"Read the compiler error guide", "build-errors", "reading-build-errors",
     "studio.learning.errors-guide", "help.learning.errors", "studio", "studio.learning-guide"},
    {"Read the testing guide", "testing", "running-tests",
     "studio.learning.tests-guide", "help.learning.tests", "studio", "studio.learning-guide"},
    {"Read the recent projects guide", "recent-projects", "recent-projects",
     "studio.learning.recent-guide", "help.learning.recent", "studio", "studio.learning-guide"},
    {"Follow Bits2Banking courses", "getting-started", "index",
     "studio.learning.bits2banking", "help.learning.bits2banking",
     "bits2banking", "bits2banking.framework-finance"},
    {"Open the Framework learning wiki", "framework-wiki", "index",
     "studio.learning.framework-wiki", "help.learning.framework-wiki",
     "framework", "framework.studio-wiki"}
};
static const RuntimeLearningProductGuide *RuntimeLearningGuideForCommand(const char *command)
{
    if (command == NULL) return NULL;
    for (size_t i = 0U; i < sizeof(runtime_learning_product_guides) /
        sizeof(runtime_learning_product_guides[0]); ++i)
        if (strcmp(command, runtime_learning_product_guides[i].command) == 0)
            return &runtime_learning_product_guides[i];
    return NULL;
}
static int RuntimeLearningCommandKnown(const char *command)
{
    return command != NULL && (strcmp(command, "help.learning.catalogue") == 0 ||
        strcmp(command, "help.learning.guide") == 0 || RuntimeLearningGuideForCommand(command) != NULL);
}
/* The definition follows the weak window-binding helpers in the learning
 * surface. Declaring it here preserves the established unity include order. */
static UmiStatus RuntimeLearningDispatch(UmiStudioGtkRuntimeChrome *runtime, const char *command);
#endif
