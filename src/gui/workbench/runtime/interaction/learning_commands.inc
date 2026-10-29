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
} RuntimeLearningProductGuide;
/* Only reviewed product identities belong here. Framework owns pack admission,
 * verified reads and history; a guide command cannot adopt or trust a project. */
static const RuntimeLearningProductGuide runtime_learning_product_guides[] = {
    {"Read the editing and saving guide", "editing", "editing-files",
     "studio.learning.editing-guide", "help.learning.editing"},
    {"Read the compiler error guide", "build-errors", "reading-build-errors",
     "studio.learning.errors-guide", "help.learning.errors"},
    {"Read the testing guide", "testing", "running-tests",
     "studio.learning.tests-guide", "help.learning.tests"},
    {"Read the recent projects guide", "recent-projects", "recent-projects",
     "studio.learning.recent-guide", "help.learning.recent"}
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
