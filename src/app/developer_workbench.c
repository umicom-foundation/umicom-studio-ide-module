/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/app/developer_workbench.c
 * PURPOSE: Implement ownership and lifecycle for Studio developer-platform centres.
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom/studio/developer_workbench.h"
#include "umicom/developer/context.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct UmiStudioDeveloperWorkbench{UmiStudioProjectCentre*projects;UmiStudioLanguageIntelligenceCentre*language;UmiStudioDebugCentre*debug;UmiStudioSourceControlCentre*source_control;UmiStudioTestExplorerCentre*tests;UmiUiWorkbenchServices*services;UmiDeveloperRuntime*runtime;UmiStudioDeveloperPipelineCentre*pipeline;UmiStudioDeveloperTaskCentre*task_centre;UmiStudioDeveloperRunCentre*run_centre;UmiStudioDeveloperWorkspaceStateCentre*workspace_state;UmiStudioProblemsCentre*problems;UmiStudioOutputCentre*output;UmiStudioProgressCentre*progress;UmiStudioDeveloperSession*session;uint64_t revision;};
/*
 * Initialise studio developer workbench from caller-provided values so later operations
 * receive a known state.
 */
UmiStatus umi_studio_developer_workbench_create(UmiStudioDeveloperWorkbench**out){UmiStudioDeveloperWorkbench*p;UmiDeveloperRuntimeBindings bindings;UmiStatus s=UMI_STATUS_OK;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(out==NULL)return UMI_STATUS_INVALID_ARGUMENT;*out=NULL;p=calloc(1U,sizeof(*p));/* Protect caller-owned memory by checking that required state is available before it is used. */ if(p==NULL)return UMI_STATUS_OUT_OF_MEMORY;p->revision=1U;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_studio_project_centre_create(&p->projects);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_studio_language_intelligence_centre_create(&p->language);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_studio_debug_centre_create(&p->debug);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_studio_source_control_centre_create(&p->source_control);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_studio_test_explorer_centre_create(&p->tests);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_ui_workbench_services_create(&p->services);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK){umi_developer_runtime_bindings_init(&bindings);bindings.projects=umi_studio_project_centre_service(p->projects);bindings.language=umi_studio_language_intelligence_centre_service(p->language);bindings.debug=umi_studio_debug_centre_service(p->debug);bindings.source_control=umi_studio_source_control_centre_service(p->source_control);bindings.tests=umi_studio_test_explorer_centre_service(p->tests);bindings.workbench=p->services;s=umi_developer_runtime_create_bound(&bindings,&p->runtime);}/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_studio_developer_pipeline_centre_create(p->runtime,&p->pipeline);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_studio_developer_task_centre_create(p->runtime,&p->task_centre);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_studio_developer_run_centre_create(p->runtime,&p->run_centre);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_studio_developer_workspace_state_centre_create(p->runtime,&p->workspace_state);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_studio_problems_centre_create(p->services,&p->problems);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_studio_output_centre_create(p->services,&p->output);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_studio_progress_centre_create(p->services,&p->progress);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s==UMI_STATUS_OK)s=umi_studio_developer_session_create(&p->session);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s!=UMI_STATUS_OK){umi_studio_developer_workbench_destroy(p);return s;}*out=p;return UMI_STATUS_OK;}
/*
 * Release or reset state held by studio developer workbench so the same storage can be
 * reused safely.
 */
void umi_studio_developer_workbench_destroy(UmiStudioDeveloperWorkbench*p){/* Protect caller-owned memory by checking that required state is available before it is used. */ if(p==NULL)return;umi_studio_developer_session_destroy(p->session);umi_studio_developer_workspace_state_centre_destroy(p->workspace_state);umi_studio_developer_run_centre_destroy(p->run_centre);umi_studio_developer_task_centre_destroy(p->task_centre);umi_studio_developer_pipeline_centre_destroy(p->pipeline);umi_developer_runtime_destroy(p->runtime);umi_studio_progress_centre_destroy(p->progress);umi_studio_output_centre_destroy(p->output);umi_studio_problems_centre_destroy(p->problems);umi_ui_workbench_services_destroy(p->services);umi_studio_test_explorer_centre_destroy(p->tests);umi_studio_source_control_centre_destroy(p->source_control);umi_studio_debug_centre_destroy(p->debug);umi_studio_language_intelligence_centre_destroy(p->language);umi_studio_project_centre_destroy(p->projects);free(p);}
/*
 * Provide the studio developer workbench snapshot operation used by this module and its
 * client applications.
 */
UmiStatus umi_studio_developer_workbench_snapshot(UmiStudioDeveloperWorkbench*p,UmiStudioDeveloperWorkbenchSnapshot*o){UmiStatus s;/* Protect caller-owned memory by checking that required state is available before it is used. */ if(p==NULL||o==NULL)return UMI_STATUS_INVALID_ARGUMENT;memset(o,0,sizeof(*o));o->struct_size=(uint32_t)sizeof(*o);o->api_version=1U;o->revision=p->revision;o->available=1;s=umi_studio_project_centre_snapshot(p->projects,&o->projects);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s!=UMI_STATUS_OK)return s;s=umi_studio_language_intelligence_centre_snapshot(p->language,&o->language);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s!=UMI_STATUS_OK)return s;s=umi_studio_debug_centre_snapshot(p->debug,&o->debug);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s!=UMI_STATUS_OK)return s;s=umi_studio_source_control_centre_snapshot(p->source_control,&o->source_control);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s!=UMI_STATUS_OK)return s;s=umi_studio_test_explorer_centre_snapshot(p->tests,&o->tests);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s!=UMI_STATUS_OK)return s;s=umi_studio_problems_centre_snapshot(p->problems,&o->problems);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s!=UMI_STATUS_OK)return s;s=umi_studio_output_centre_snapshot(p->output,&o->output);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s!=UMI_STATUS_OK)return s;s=umi_studio_progress_centre_snapshot(p->progress,&o->progress);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s!=UMI_STATUS_OK)return s;s=umi_studio_developer_session_snapshot(p->session,&o->session);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s!=UMI_STATUS_OK)return s;s=umi_studio_developer_pipeline_centre_snapshot(p->pipeline,&o->pipeline);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s!=UMI_STATUS_OK)return s;s=umi_studio_developer_task_centre_snapshot(p->task_centre,&o->tasks);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s!=UMI_STATUS_OK)return s;s=umi_studio_developer_run_centre_snapshot(p->run_centre,&o->run);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s!=UMI_STATUS_OK)return s;s=umi_studio_developer_workspace_state_centre_snapshot(p->workspace_state,&o->workspace_state);/* Protect caller-owned memory by checking that required state is available before it is used. */ if(s!=UMI_STATUS_OK)return s;o->item_count=o->projects.service.item_count+o->language.service.item_count+o->debug.service.item_count+o->source_control.service.item_count+o->tests.service.total_count+o->problems.item_count+o->output.item_count+o->progress.item_count+o->pipeline.item_count;return UMI_STATUS_OK;}
#define A(fn,type,field) type *fn(UmiStudioDeveloperWorkbench*p){return p!=NULL?p->field:NULL;}
A(umi_studio_developer_workbench_projects,UmiStudioProjectCentre,projects) A(umi_studio_developer_workbench_language,UmiStudioLanguageIntelligenceCentre,language) A(umi_studio_developer_workbench_debug,UmiStudioDebugCentre,debug) A(umi_studio_developer_workbench_source_control,UmiStudioSourceControlCentre,source_control) A(umi_studio_developer_workbench_tests,UmiStudioTestExplorerCentre,tests) A(umi_studio_developer_workbench_services,UmiUiWorkbenchServices,services) A(umi_studio_developer_workbench_runtime,UmiDeveloperRuntime,runtime) A(umi_studio_developer_workbench_pipeline,UmiStudioDeveloperPipelineCentre,pipeline) A(umi_studio_developer_workbench_task_centre,UmiStudioDeveloperTaskCentre,task_centre) A(umi_studio_developer_workbench_run_centre,UmiStudioDeveloperRunCentre,run_centre) A(umi_studio_developer_workbench_workspace_state,UmiStudioDeveloperWorkspaceStateCentre,workspace_state) A(umi_studio_developer_workbench_session,UmiStudioDeveloperSession,session)
#undef A

/*
 * Provide the studio developer workbench universal model operation used by this module and
 * its client applications.
 */
UmiDeveloperUniversalModel *umi_studio_developer_workbench_universal_model(
    UmiStudioDeveloperWorkbench *workbench)
{
    return workbench != NULL
        ? umi_developer_runtime_universal_model(workbench->runtime)
        : NULL;
}

/*
 * Provide the copy selection text operation used by this module and its client
 * applications.
 */
static void copy_selection_text(char *destination,size_t capacity,const char *source)
{
    size_t length;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if(destination==NULL||capacity==0U)return;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if(source==NULL)source="";
    length=strlen(source);
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if(length>=capacity)length=capacity-1U;
    /* Keep the operation inside its valid bounds before reading, writing or adding data. */
    if(length>0U)memcpy(destination,source,length);
    destination[length]='\0';
}

/*
 * Provide the studio developer workbench activate project operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_developer_workbench_activate_project(
    UmiStudioDeveloperWorkbench *workbench,
    const UmiProjectWorkspaceSelectionRequest *request,
    UmiProjectWorkspaceSelectionSnapshot *out_selection)
{
    UmiProjectWorkspaceSelectionSnapshot selection;
    UmiDeveloperContextSnapshot context;
    UmiStudioDeveloperSessionSnapshot session;
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if(workbench==NULL)return UMI_STATUS_INVALID_ARGUMENT;

    status=umi_studio_project_centre_select(
        workbench->projects,request,&selection);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if(status!=UMI_STATUS_OK)return status;

    memset(&context,0,sizeof(context));
    context.struct_size=(uint32_t)sizeof(context);
    context.api_version=UMI_DEVELOPER_CONTEXT_API_VERSION;
    copy_selection_text(context.project_id,sizeof(context.project_id),selection.project.id);
    /* Apply this branch only when its contract condition is satisfied. */
    if(selection.has_configuration)
        copy_selection_text(context.configuration_id,sizeof(context.configuration_id),selection.configuration.id);
    /* Apply this branch only when its contract condition is satisfied. */
    if(selection.has_target)
        copy_selection_text(context.target_id,sizeof(context.target_id),selection.target.id);
    /* Apply this branch only when its contract condition is satisfied. */
    if(selection.has_launch_profile)
        copy_selection_text(context.launch_profile_id,sizeof(context.launch_profile_id),selection.launch_profile.id);
    copy_selection_text(context.workspace_directory,sizeof(context.workspace_directory),selection.project.root_uri);

    status=umi_developer_runtime_set_context(workbench->runtime,&context);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if(status!=UMI_STATUS_OK)return status;

    status=umi_studio_developer_session_snapshot(workbench->session,&session);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if(status!=UMI_STATUS_OK)return status;
    copy_selection_text(session.project_id,sizeof(session.project_id),selection.project.id);
    copy_selection_text(session.configuration_id,sizeof(session.configuration_id),
                        selection.has_configuration?selection.configuration.id:"");
    copy_selection_text(session.launch_profile_id,sizeof(session.launch_profile_id),
                        selection.has_launch_profile?selection.launch_profile.id:"");
    status=umi_studio_developer_session_set_context(workbench->session,&session);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if(status!=UMI_STATUS_OK)return status;

    workbench->revision+=1U;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if(out_selection!=NULL)*out_selection=selection;
    return UMI_STATUS_OK;
}

/*
 * Provide the studio developer workbench prepare project workflow operation used by this
 * module and its client applications.
 */
UmiStatus umi_studio_developer_workbench_prepare_project_workflow(
    UmiStudioDeveloperWorkbench *workbench,
    const UmiDeveloperProjectWorkflowRequest *request,
    UmiDeveloperProjectWorkflowSnapshot *out_workflow)
{
    UmiProjectWorkspaceSelectionRequest selection_request;
    UmiDeveloperProjectWorkflowRequest resolved_request;
    UmiProjectWorkspaceSelectionSnapshot selection;
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if(workbench==NULL||request==NULL)return UMI_STATUS_INVALID_ARGUMENT;

    memset(&selection_request,0,sizeof(selection_request));
    selection_request.struct_size=(uint32_t)sizeof(selection_request);
    selection_request.api_version=UMI_PROJECT_WORKSPACE_QUERY_API_VERSION;
    selection_request.project_id=request->project_id;
    selection_request.configuration_id=request->configuration_id;
    selection_request.target_id=request->target_id;
    selection_request.task_id=request->task_id;
    selection_request.launch_profile_id=request->launch_profile_id;
    selection_request.environment_id=request->environment_id;

    status=umi_studio_developer_workbench_activate_project(
        workbench,&selection_request,&selection);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if(status!=UMI_STATUS_OK)return status;

    resolved_request=*request;
    resolved_request.project_id=selection.project.id;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if(resolved_request.configuration_id==NULL&&selection.has_configuration)
        resolved_request.configuration_id=selection.configuration.id;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if(resolved_request.target_id==NULL&&selection.has_target)
        resolved_request.target_id=selection.target.id;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if(resolved_request.launch_profile_id==NULL&&selection.has_launch_profile)
        resolved_request.launch_profile_id=selection.launch_profile.id;
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if(resolved_request.environment_id==NULL&&selection.has_environment)
        resolved_request.environment_id=selection.environment.id;

    status=umi_studio_developer_pipeline_centre_prepare_project_workflow(
        workbench->pipeline,&resolved_request,out_workflow);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if(status==UMI_STATUS_OK)workbench->revision+=1U;
    return status;
}

/*
 * Provide the studio developer workbench validate project operation used by this module
 * and its client applications.
 */
UmiStatus umi_studio_developer_workbench_validate_project(
    UmiStudioDeveloperWorkbench *workbench,
    UmiProjectWorkspaceValidationReport *out_report)
{
    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if(workbench==NULL||out_report==NULL)return UMI_STATUS_INVALID_ARGUMENT;
    return umi_studio_project_centre_validate(workbench->projects,out_report);
}

/*
 * Provide the studio developer workbench import project operation used by this module and
 * its client applications.
 */
UmiStatus umi_studio_developer_workbench_import_project(
    UmiStudioDeveloperWorkbench *workbench,
    const UmiDeveloperProjectBootstrapRequest *request,
    UmiDeveloperProjectBootstrapSnapshot *out_snapshot)
{
    UmiProjectWorkspaceSelectionRequest selection_request;
    UmiDeveloperProjectWorkflowRequest workflow_request;
    UmiStatus status;

    /*
     * Protect caller-owned memory by checking that required state is available before it is
     * used.
     */
    if (workbench == NULL || request == NULL || out_snapshot == NULL) {
        return UMI_STATUS_INVALID_ARGUMENT;
    }

#define snapshot (*out_snapshot)
    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.struct_size = (uint32_t)sizeof(snapshot);
    snapshot.api_version = UMI_DEVELOPER_PROJECT_BOOTSTRAP_API_VERSION;

    status = umi_studio_project_centre_import_directory(
        workbench->projects, &request->project, &snapshot.project);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    memset(&selection_request, 0, sizeof(selection_request));
    selection_request.struct_size = (uint32_t)sizeof(selection_request);
    selection_request.api_version = UMI_PROJECT_WORKSPACE_QUERY_API_VERSION;
    selection_request.project_id = snapshot.project.project_id;
    selection_request.configuration_id = snapshot.project.configuration_id;
    selection_request.target_id = snapshot.project.target_id;
    selection_request.environment_id = snapshot.project.environment_id;
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (snapshot.project.build_task_id[0] != '\0')
        selection_request.task_id = snapshot.project.build_task_id;
    /* Apply this branch only when its contract condition is satisfied. */
    if (snapshot.project.has_launch_profile)
        selection_request.launch_profile_id = snapshot.project.launch_profile_id;

    status = umi_studio_developer_workbench_activate_project(
        workbench, &selection_request, &snapshot.project.selection);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    /* Apply this branch only when its contract condition is satisfied. */
    if (snapshot.project.has_git) {
        UmiSourceControlRepositorySnapshot repository;
        UmiDeveloperContextSnapshot context;
        UmiStudioDeveloperSessionSnapshot session;
        char repository_id[128];
        int written = snprintf(repository_id, sizeof(repository_id),
                               "%s.git", snapshot.project.project_id);
        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (written < 0 || (size_t)written >= sizeof(repository_id))
            return UMI_STATUS_CAPACITY_EXCEEDED;

        memset(&repository, 0, sizeof(repository));
        repository.struct_size = (uint32_t)sizeof(repository);
        repository.api_version = UMI_SOURCE_CONTROL_REPOSITORY_API_VERSION;
        copy_selection_text(repository.id, sizeof(repository.id), repository_id);
        copy_selection_text(repository.root_uri, sizeof(repository.root_uri),
                            snapshot.project.root_directory);
        copy_selection_text(repository.provider, sizeof(repository.provider), "git");
        status = umi_source_control_repository_registry_upsert(
            umi_source_control_service_repository(
                umi_developer_runtime_source_control(workbench->runtime)),
            &repository);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;

        status = umi_developer_context_snapshot(
            umi_developer_runtime_context(workbench->runtime), &context);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;
        copy_selection_text(context.repository_id, sizeof(context.repository_id),
                            repository_id);
        status = umi_developer_runtime_set_context(workbench->runtime, &context);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;

        status = umi_studio_developer_session_snapshot(workbench->session, &session);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;
        copy_selection_text(session.repository_id, sizeof(session.repository_id),
                            repository_id);
        status = umi_studio_developer_session_set_context(workbench->session, &session);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;
    }

    /* Apply this branch only when its contract condition is satisfied. */
    if (request->prepare_workflow != 0) {
        memset(&workflow_request, 0, sizeof(workflow_request));
        workflow_request.struct_size = (uint32_t)sizeof(workflow_request);
        workflow_request.api_version = UMI_DEVELOPER_PROJECT_WORKFLOW_API_VERSION;
        workflow_request.preset = request->preset;
        workflow_request.workflow_id = request->workflow_id;
        workflow_request.project_id = snapshot.project.project_id;
        workflow_request.configuration_id = snapshot.project.configuration_id;
        workflow_request.target_id = snapshot.project.target_id;
        workflow_request.environment_id = snapshot.project.environment_id;
        workflow_request.timeout_ms = request->timeout_ms;
        workflow_request.max_attempts = request->max_attempts;
        workflow_request.include_configure = request->include_configure;
        /* Apply this branch only when its contract condition is satisfied. */
        if (snapshot.project.has_launch_profile)
            workflow_request.launch_profile_id = snapshot.project.launch_profile_id;

        status = umi_studio_developer_pipeline_centre_prepare_project_workflow(
            workbench->pipeline, &workflow_request, &snapshot.workflow);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;
        snapshot.workflow_prepared = 1;
    }


    /*
     * Preparing a project workflow refreshes the reusable Developer Context.
     * Reapply the repository identity discovered during import so source
     * control, workspace-state capture and the returned bootstrap snapshot all
     * observe one coherent project/repository session.
     */
    if (snapshot.project.has_git) {
        UmiDeveloperContextPatch patch = {0};
        char repository_id[128];
        int written = snprintf(repository_id, sizeof(repository_id),
                               "%s.git", snapshot.project.project_id);

        /* Keep the operation inside its valid bounds before reading, writing or adding data. */
        if (written < 0 || (size_t)written >= sizeof(repository_id))
            return UMI_STATUS_CAPACITY_EXCEEDED;

        patch.struct_size = (uint32_t)sizeof(patch);
        patch.api_version = UMI_DEVELOPER_CONTEXT_PATCH_API_VERSION;
        patch.field_mask = UMI_DEVELOPER_CONTEXT_PATCH_REPOSITORY;
        patch.repository_id = repository_id;
        status = umi_developer_context_patch(
            umi_developer_runtime_context(workbench->runtime), &patch, NULL);
        /* Preserve the original failure result so the caller can respond to the correct cause. */
        if (status != UMI_STATUS_OK) return status;
    }

    status = umi_studio_developer_workspace_state_centre_capture_context(
        workbench->workspace_state);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;
    status = umi_developer_context_snapshot(
        umi_developer_runtime_context(workbench->runtime), &snapshot.context);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (status != UMI_STATUS_OK) return status;

    workbench->revision += 1U;
#undef snapshot
    return UMI_STATUS_OK;
}
