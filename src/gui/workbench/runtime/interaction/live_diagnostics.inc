/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: src/gui/workbench/runtime/interaction/live_diagnostics.inc
 * PURPOSE: Bind the current named source and trusted workspace to Framework live diagnostics.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

#include "umicom/document/source_navigation.h"
#include "umicom/document/source_request.h"
#include "umicom/language_runtime/builtin_profiles.h"
#include "umicom/ui/gtk4/live_diagnostics.h"

typedef struct StudioDiagnosticBinding
{
    UmiStudioFileDialogRequest *owner;
    UmiDocumentCoordinator *documents;
    UmiDocumentId document_id;
    uint64_t workspace_revision;
    char root[UMI_PATH_CAPACITY], uri[UMI_DOCUMENT_URI_CAPACITY],
        language[UMI_DOCUMENT_LANGUAGE_CAPACITY];
} StudioDiagnosticBinding;
static void RuntimeLiveDiagnosticBindingFree(gpointer data)
{
    StudioDiagnosticBinding *binding = data;
    runtime_file_dialog_request_free(binding->owner);
    g_free(binding);
}
static bool RuntimeLiveDiagnosticAuthorized(void *data)
{
    StudioDiagnosticBinding *binding = data;
    GObject *window = NULL;
    UmiStudioGtkRuntimeChrome *runtime = runtime_file_dialog_owner(binding->owner, &window);
    bool allowed = false;
    if (runtime != NULL && UmiGtk4WindowIsOpen(GTK_WINDOW(window)) &&
        umi_studio_ui_documents(runtime->owner->ui) == binding->documents)
    {
        UmiStudioWorkspaceSnapshot workspace;
        allowed = umi_studio_workspace_snapshot(umi_studio_ui_services(runtime->owner->ui),
                                                &workspace) == UMI_STATUS_OK &&
                  workspace.graph.open && workspace.graph.trusted &&
                  workspace.graph.revision == binding->workspace_revision &&
                  umi_path_equal(workspace.graph.root, binding->root);
    }
    g_clear_object(&window);
    return allowed;
}
static UmiStatus RuntimeLiveDiagnosticCapture(char *out, size_t capacity, size_t *out_bytes,
                                              void *data)
{
    StudioDiagnosticBinding *binding = data;
    if (!RuntimeLiveDiagnosticAuthorized(binding))
        return UMI_STATUS_PERMISSION_DENIED;
    UmiDocumentSourceRequest *capture = NULL;
    UmiStatus status = UmiDocumentSourceRequestCreateInspection(binding->documents,
                                                                binding->document_id, &capture);
    UmiDocumentSourceRequestSummary summary;
    const char *text = NULL;
    size_t bytes = 0U;
    if (status == UMI_STATUS_OK)
        status = UmiDocumentSourceRequestInspect(capture, &summary);
    if (status == UMI_STATUS_OK && (strcmp(summary.uri, binding->uri) != 0 ||
                                    strcmp(summary.language_id, binding->language) != 0))
        status = UMI_STATUS_INVALID_STATE;
    if (status == UMI_STATUS_OK)
        status = UmiDocumentSourceRequestRead(capture, &text, &bytes);
    if (status == UMI_STATUS_OK && bytes >= capacity)
        status = UMI_STATUS_CAPACITY_EXCEEDED;
    /* The original document stays selected by identity when another tab becomes
     * active. Closing or renaming it requires a fresh explicit diagnostics owner. */
    if (status == UMI_STATUS_OK)
    {
        memcpy(out, text, bytes);
        out[bytes] = '\0';
        *out_bytes = bytes;
    }
    UmiDocumentSourceRequestDestroy(capture);
    return status;
}
static UmiStatus RuntimeLiveDiagnosticNavigate(const char *uri, const char *source, size_t bytes,
                                               UmiEditorTextPosition start,
                                               UmiEditorTextPosition end, void *data)
{
    StudioDiagnosticBinding *binding = data;
    if (!RuntimeLiveDiagnosticAuthorized(binding))
        return UMI_STATUS_PERMISSION_DENIED;
    if (strcmp(uri, binding->uri) != 0)
        return UMI_STATUS_INVALID_STATE;
    /* Framework validates the complete current draft and records navigation.
     * This also works when the original tab is no longer the active tab. */
    return UmiDocumentCoordinatorSelectCapturedSourceRange(
        binding->documents, binding->document_id, uri, source, bytes, start, end, NULL, NULL);
}
static void RuntimeLiveDiagnostics(GtkButton *button, gpointer data)
{
    (void)button;
    UmiStudioGtkRuntimeChrome *runtime = data;
    UmiStudioFileDialogRequest *request = runtime_file_dialog_request_new(runtime, 0U);
    if (request == NULL)
        return;
    GObject *owner_window = NULL;
    runtime = runtime_file_dialog_owner(request, &owner_window);
    if (runtime == NULL || !UmiGtk4WindowIsOpen(GTK_WINDOW(owner_window)))
        goto done;
    GWeakRef *existing = g_object_get_data(owner_window, "umicom-studio-live-diagnostics-window");
    GtkWindow *old = existing != NULL ? g_weak_ref_get(existing) : NULL;
    if (old != NULL && UmiGtk4WindowIsOpen(old))
    {
        gtk_window_present(old);
        g_object_unref(old);
        goto done;
    }
    g_clear_object(&old);
    GtkWidget *old_panel = g_object_get_data(owner_window, "umicom-studio-live-diagnostics-panel");
    if (old_panel != NULL && UmiGtk4LiveDiagnosticPanelPending(old_panel))
    {
        runtime_show_notification(runtime, "The previous language server is still stopping.");
        goto done;
    }
    UmiStudioServices *services = umi_studio_ui_services(runtime->owner->ui);
    UmiStudioWorkspaceSnapshot workspace;
    UmiDocumentWorkingCopySnapshot active;
    UmiStatus status = umi_studio_workspace_snapshot(services, &workspace);
    if (status == UMI_STATUS_OK && (!workspace.graph.open || !workspace.graph.trusted))
        status = UMI_STATUS_PERMISSION_DENIED;
    UmiDocumentCoordinator *documents = umi_studio_ui_documents(runtime->owner->ui);
    if (status == UMI_STATUS_OK)
        status = umi_document_coordinator_active_snapshot(documents, &active);
    if (status == UMI_STATUS_OK && !active.has_path)
        status = UMI_STATUS_INVALID_STATE;
    if (status != UMI_STATUS_OK)
    {
        runtime_show_notification(runtime, "Open and trust the project, then open a named source "
                                           "file. Use Save As first for an unnamed draft.");
        goto done;
    }
    StudioDiagnosticBinding *binding = g_try_new0(StudioDiagnosticBinding, 1U);
    char *source = g_try_malloc(UMI_LANGUAGE_RUNTIME_JSON_CAPACITY);
    if (binding == NULL || source == NULL)
    {
        g_free(binding);
        g_free(source);
        goto done;
    }
    binding->owner = runtime_file_dialog_request_new(runtime, 0U);
    if (binding->owner == NULL)
    {
        g_free(binding);
        g_free(source);
        goto done;
    }
    binding->documents = documents;
    binding->document_id = active.document_id;
    binding->workspace_revision = workspace.graph.revision;
    strcpy(binding->root, workspace.graph.root);
    strcpy(binding->uri, active.uri);
    strcpy(binding->language, active.language_id);
    size_t bytes = 0U;
    status =
        RuntimeLiveDiagnosticCapture(source, UMI_LANGUAGE_RUNTIME_JSON_CAPACITY, &bytes, binding);
    char *root_uri = status == UMI_STATUS_OK ? g_filename_to_uri(binding->root, NULL, NULL) : NULL;
    if (status == UMI_STATUS_OK && root_uri == NULL)
        status = UMI_STATUS_INVALID_ARGUMENT;
    UmiLanguageDiagnosticMonitorConfig config = {0};
    const UmiLanguageServerProfile *builtin =
        umi_language_runtime_builtin_profile_for_language(binding->language);
    if (builtin != NULL)
        config.profile = *builtin;
    else
    {
        strcpy(config.profile.id, "selected-language-server");
        strcpy(config.profile.display_name, "Selected language server");
    }
    config.profile.enabled = 1;
    config.working_directory = binding->root;
    config.tool_directory =
        umi_studio_build_service_profile(umi_studio_services_build(services))->tool_directory;
    config.document = (UmiLanguageDiagnosticRequest){root_uri, binding->uri, binding->language,
                                                     source,   bytes,        10000U};
    UmiGtk4LiveDiagnosticCallbacks callbacks = {RuntimeLiveDiagnosticCapture,
                                                RuntimeLiveDiagnosticAuthorized, binding,
                                                RuntimeLiveDiagnosticBindingFree};
    GtkWidget *panel = NULL;
    if (status == UMI_STATUS_OK)
        status = UmiGtk4LiveDiagnosticPanelCreate(&config, &callbacks, &panel);
    g_free(root_uri);
    g_free(source);
    if (status != UMI_STATUS_OK)
    {
        RuntimeLiveDiagnosticBindingFree(binding);
        runtime_show_notification(
            runtime, "The source or workspace exceeds the live language-session limits.");
        goto done;
    }
    (void)UmiGtk4LiveDiagnosticPanelSetNavigator(panel, RuntimeLiveDiagnosticNavigate);
    GtkWindow *window = GTK_WINDOW(gtk_window_new());
    gtk_window_set_title(window, "Studio — Live Source Diagnostics");
    gtk_window_set_default_size(window, 920, 650);
    gtk_window_set_transient_for(window, GTK_WINDOW(owner_window));
    gtk_window_set_destroy_with_parent(window, TRUE);
    gtk_window_set_child(window, panel);
    g_object_set_data_full(owner_window, "umicom-studio-live-diagnostics-panel",
                           g_object_ref(panel), g_object_unref);
    GWeakRef *weak = g_new0(GWeakRef, 1U);
    g_weak_ref_init(weak, window);
    g_object_set_data_full(owner_window, "umicom-studio-live-diagnostics-window", weak,
                           RuntimeProgramConsoleWeakFree);
    gtk_window_present(window);
done:
    g_clear_object(&owner_window);
    runtime_file_dialog_request_free(request);
}
