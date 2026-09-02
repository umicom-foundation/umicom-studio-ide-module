/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_developer_developer_workbench.c
 *
 * PURPOSE:
 *   Verify the test developer developer workbench behavior for
 *   Umicom Studio IDE.
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
 * File: applications/studio/tests/test_developer_developer_workbench.c
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "umicom/platform/filesystem.h"
#include "umicom/platform/path.h"
#include "umicom/studio/developer_workbench.h"

/*
 * Exercise populate and return a clear result when the behaviour no longer matches its
 * contract.
 */
static int populate(UmiStudioDeveloperWorkbench *workbench)
{
    UmiProjectWorkspace *workspace =
        umi_studio_project_centre_service(
            umi_studio_developer_workbench_projects(workbench));
    UmiProjectDescriptorSnapshot project={0};
    UmiProjectConfigurationSnapshot configuration={0};
    UmiProjectTargetSnapshot target={0};
    UmiProjectTaskSnapshot task={0};
    UmiProjectLaunchProfileSnapshot launch={0};

    project.struct_size=(uint32_t)sizeof(project);
    project.api_version=UMI_PROJECT_DESCRIPTOR_API_VERSION;
    strcpy(project.id,"studio"); strcpy(project.name,"Umicom Studio");
    strcpy(project.root_uri,"C:/Dev/umicom/umicom-studio"); project.enabled=1;
    /* Apply this branch only when its contract condition is satisfied. */
    if(umi_project_descriptor_registry_upsert(
        umi_project_workspace_descriptor(workspace),&project)!=UMI_STATUS_OK)return 1;

    configuration.struct_size=(uint32_t)sizeof(configuration);
    configuration.api_version=UMI_PROJECT_CONFIGURATION_API_VERSION;
    strcpy(configuration.id,"debug"); strcpy(configuration.project_id,"studio");
    strcpy(configuration.name,"Debug"); configuration.active=1;
    /* Apply this branch only when its contract condition is satisfied. */
    if(umi_project_configuration_registry_upsert(
        umi_project_workspace_configuration(workspace),&configuration)!=UMI_STATUS_OK)return 2;

    target.struct_size=(uint32_t)sizeof(target);
    target.api_version=UMI_PROJECT_TARGET_API_VERSION;
    strcpy(target.id,"studio-console"); strcpy(target.project_id,"studio");
    strcpy(target.name,"Studio Console"); target.enabled=1; target.default_target=1;
    /* Apply this branch only when its contract condition is satisfied. */
    if(umi_project_target_registry_upsert(
        umi_project_workspace_target(workspace),&target)!=UMI_STATUS_OK)return 3;

    task.struct_size=(uint32_t)sizeof(task);
    task.api_version=UMI_PROJECT_TASK_API_VERSION;
    strcpy(task.id,"build"); strcpy(task.project_id,"studio");
    strcpy(task.label,"Build"); strcpy(task.command,"cmake --build build");
    strcpy(task.group,"build"); task.enabled=1; task.default_task=1;
    /* Apply this branch only when its contract condition is satisfied. */
    if(umi_project_task_registry_upsert(
        umi_project_workspace_task(workspace),&task)!=UMI_STATUS_OK)return 4;

    launch.struct_size=(uint32_t)sizeof(launch);
    launch.api_version=UMI_PROJECT_LAUNCH_PROFILE_API_VERSION;
    strcpy(launch.id,"run"); strcpy(launch.project_id,"studio");
    strcpy(launch.name,"Run"); strcpy(launch.program,"umicom-studio-console");
    launch.default_profile=1;
    /* Apply this branch only when its contract condition is satisfied. */
    if(umi_project_launch_profile_registry_upsert(
        umi_project_workspace_launch_profile(workspace),&launch)!=UMI_STATUS_OK)return 5;
    return 0;
}

/*
 * Exercise test import and return a clear result when the behaviour no longer matches its
 * contract.
 */
static int test_import(UmiStudioDeveloperWorkbench *workbench)
{
    UmiDeveloperProjectBootstrapRequest request = {0};
    UmiDeveloperProjectBootstrapSnapshot bootstrap;
    UmiStudioDeveloperSessionSnapshot session;
    UmiStudioDeveloperWorkbenchSnapshot snapshot;
    char temp_directory[UMI_PATH_CAPACITY];
    char root[UMI_PATH_CAPACITY];
    char cmake_file[UMI_PATH_CAPACITY];
    char source_file[UMI_PATH_CAPACITY];
    char git_directory[UMI_PATH_CAPACITY];

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (umi_fs_temp_directory(temp_directory, sizeof(temp_directory)) != UMI_STATUS_OK)
        return 1;
    /* Apply this branch only when its contract condition is satisfied. */
    if (umi_path_join(temp_directory, "umicom-b31-studio-workbench",
                      root, sizeof(root)) != UMI_STATUS_OK) return 2;
    (void)umi_fs_remove_tree(root);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (umi_fs_make_directories(root) != UMI_STATUS_OK) return 3;
    /* Apply this branch only when its contract condition is satisfied. */
    if (umi_path_join(root, "CMakeLists.txt", cmake_file,
                      sizeof(cmake_file)) != UMI_STATUS_OK) return 4;
    /* Apply this branch only when its contract condition is satisfied. */
    if (umi_path_join(root, "main.c", source_file,
                      sizeof(source_file)) != UMI_STATUS_OK) return 5;
    /* Apply this branch only when its contract condition is satisfied. */
    if (umi_path_join(root, ".git", git_directory,
                      sizeof(git_directory)) != UMI_STATUS_OK) return 6;
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (umi_fs_make_directories(git_directory) != UMI_STATUS_OK) return 7;
    /* Apply this branch only when its contract condition is satisfied. */
    if (umi_fs_write_text(cmake_file,
        "cmake_minimum_required(VERSION 3.24)\nproject(workbench_import C)\n") !=
        UMI_STATUS_OK) return 8;
    /* Apply this branch only when its contract condition is satisfied. */
    if (umi_fs_write_text(source_file,
        "int main(void) { return 0; }\n") != UMI_STATUS_OK) return 9;

    request.struct_size = (uint32_t)sizeof(request);
    request.api_version = UMI_DEVELOPER_PROJECT_BOOTSTRAP_API_VERSION;
    request.project.struct_size = (uint32_t)sizeof(request.project);
    request.project.api_version = UMI_PROJECT_WORKSPACE_IMPORT_API_VERSION;
    request.project.root_directory = root;
    request.project.project_id = "workbench-import";
    request.project.create_test_task = 1;
    request.preset = UMI_DEVELOPER_PROJECT_WORKFLOW_TEST;
    request.workflow_id = "workbench.import.test";
    request.prepare_workflow = 1;
    request.include_configure = 1;

    /* Apply this branch only when its contract condition is satisfied. */
    if (umi_studio_developer_workbench_import_project(
            workbench, &request, &bootstrap) != UMI_STATUS_OK) return 10;
    /* Apply this branch only when its contract condition is satisfied. */
    if (!bootstrap.workflow_prepared ||
        bootstrap.workflow.workflow.operation_count != 3U ||
        strcmp(bootstrap.context.project_id, "workbench-import") != 0 ||
        strcmp(bootstrap.context.repository_id, "workbench-import.git") != 0)
        return 11;
    /* Apply this branch only when its contract condition is satisfied. */
    if (umi_studio_developer_session_snapshot(
            umi_studio_developer_workbench_session(workbench),
            &session) != UMI_STATUS_OK) return 12;
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if (strcmp(session.project_id, "workbench-import") != 0 ||
        strcmp(session.repository_id, "workbench-import.git") != 0) return 13;
    /* Apply this branch only when its contract condition is satisfied. */
    if (umi_source_control_repository_registry_count(
            umi_source_control_service_repository(
                umi_developer_runtime_source_control(
                    umi_studio_developer_workbench_runtime(workbench)))) != 1U)
        return 14;
    /* Apply this branch only when its contract condition is satisfied. */
    if (umi_studio_developer_workbench_snapshot(
            workbench, &snapshot) != UMI_STATUS_OK ||
        !snapshot.projects.has_selection ||
        strcmp(snapshot.projects.selection.project.id, "workbench-import") != 0 ||
        !snapshot.pipeline.has_project_workflow) return 15;

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (umi_fs_remove_tree(root) != UMI_STATUS_OK) return 16;
    return 0;
}

/*
 * Start this command or application, report setup failures, and return a process exit code
 * to the operating system.
 */
int main(void)
{
    UmiStudioDeveloperWorkbench *workbench=NULL;
    UmiStudioDeveloperWorkbenchSnapshot snapshot;
    UmiProjectWorkspaceSelectionRequest request={0};
    UmiProjectWorkspaceSelectionSnapshot selection;
    UmiProjectWorkspaceValidationReport validation;
    UmiDeveloperContextSnapshot context;
    UmiStudioDeveloperSessionSnapshot session;
    UmiDeveloperProjectWorkflowRequest workflow_request={0};
    UmiDeveloperProjectWorkflowSnapshot workflow;
    int result;

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if(umi_studio_developer_workbench_create(&workbench)!=UMI_STATUS_OK)return 1;
    result=populate(workbench);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if(result!=0)return 10+result;

    request.struct_size=(uint32_t)sizeof(request);
    request.api_version=UMI_PROJECT_WORKSPACE_QUERY_API_VERSION;
    request.project_id="studio";
    /* Apply this branch only when its contract condition is satisfied. */
    if(umi_studio_developer_workbench_activate_project(
        workbench,&request,&selection)!=UMI_STATUS_OK)return 20;
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if(strcmp(selection.project.id,"studio")!=0)return 21;

    /* Apply this branch only when its contract condition is satisfied. */
    if(umi_developer_context_snapshot(
        umi_developer_runtime_context(
            umi_studio_developer_workbench_runtime(workbench)),
        &context)!=UMI_STATUS_OK)return 22;
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if(strcmp(context.project_id,"studio")!=0||
       strcmp(context.configuration_id,"debug")!=0||
       strcmp(context.target_id,"studio-console")!=0||
       strcmp(context.launch_profile_id,"run")!=0)return 23;

    /* Apply this branch only when its contract condition is satisfied. */
    if(umi_studio_developer_session_snapshot(
        umi_studio_developer_workbench_session(workbench),
        &session)!=UMI_STATUS_OK)return 24;
    /* Use the stable identifier comparison to choose the matching record or policy. */
    if(strcmp(session.project_id,"studio")!=0||
       strcmp(session.configuration_id,"debug")!=0||
       strcmp(session.launch_profile_id,"run")!=0)return 25;

    /* Apply this operation only while the related capability or state is available. */
    if(umi_studio_developer_workbench_validate_project(
        workbench,&validation)!=UMI_STATUS_OK||validation.valid==0)return 26;

    workflow_request.struct_size=(uint32_t)sizeof(workflow_request);
    workflow_request.api_version=UMI_DEVELOPER_PROJECT_WORKFLOW_API_VERSION;
    workflow_request.preset=UMI_DEVELOPER_PROJECT_WORKFLOW_BUILD;
    workflow_request.project_id="studio";
    /* Apply this branch only when its contract condition is satisfied. */
    if(umi_studio_developer_workbench_prepare_project_workflow(
        workbench,&workflow_request,&workflow)!=UMI_STATUS_OK)return 27;
    /* Apply this branch only when its contract condition is satisfied. */
    if(workflow.workflow.operation_count!=1U)return 28;

    /* Apply this branch only when its contract condition is satisfied. */
    if(umi_studio_developer_workbench_snapshot(
        workbench,&snapshot)!=UMI_STATUS_OK||!snapshot.available||
       !snapshot.projects.has_selection||
       !snapshot.pipeline.has_project_workflow)return 29;

    umi_studio_developer_workbench_destroy(workbench);

    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if(umi_studio_developer_workbench_create(&workbench)!=UMI_STATUS_OK)return 30;
    result = test_import(workbench);
    umi_studio_developer_workbench_destroy(workbench);
    /* Preserve the original failure result so the caller can respond to the correct cause. */
    if (result != 0) return 100 + result;
    return 0;
}
