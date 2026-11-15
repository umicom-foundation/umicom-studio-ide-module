/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_codeguard_native_gtk4.c
 *
 * PURPOSE:
 *   Exercise quality menu actions, copied output and native owner/context changes.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "workbench_window.h"
#include "umicom/studio/bootstrap.h"
#include "umicom/studio/settings.h"
#include "umicom/studio/workspace.h"
#include "umicom/studio/codeguard_workflow.h"
#include "umicom/ui/gtk4/automation.h"
#include "studio_test_state.h"
#include <stdlib.h>
#include <string.h>
#define CHECK(c) do { if (!(c)) { fprintf(stderr,"%d: %s\n",__LINE__,#c); \
    failed=1; goto cleanup; } } while(0)
#define REQUIRE(c) do { if (!(c)) { fprintf(stderr,"%d: %s\n",__LINE__,#c); exit(1); } } while(0)

#ifdef UMICOM_STUDIO_CODEGUARD_NATIVE_FAULTS
static int fail_poll;
static unsigned poll_failures;
UmiStatus __real_UmiStudioCodeGuardPoll(UmiStudioServices *,UmiCodeGuardReviewSnapshot *);
UmiStatus __wrap_UmiStudioCodeGuardPoll(UmiStudioServices *services,UmiCodeGuardReviewSnapshot *out)
{
    if(fail_poll) { ++poll_failures; return UMI_STATUS_IO_ERROR; }
    return __real_UmiStudioCodeGuardPoll(services,out);
}
#endif
/* A real single-worker queue keeps cancellation and duplicate admission
 * deterministic. No timing assumption about the scanner finishing is needed. */
typedef struct QueueGate {
    UmiMutex *mutex;
    UmiCondition *condition;
    UmiTask *task;
    int entered,released;
} QueueGate;
static UmiStatus HoldQueue(UmiTaskContext *context,void *data)
{
    (void)context;
    QueueGate *gate=data;
    UmiStatus status=umi_mutex_lock(gate->mutex);
    if(status!=UMI_STATUS_OK)return status;
    gate->entered=1;
    (void)umi_condition_broadcast(gate->condition);
    while(!gate->released && status==UMI_STATUS_OK)
        status=umi_condition_wait_for(gate->condition,gate->mutex,30000U);
    (void)umi_mutex_unlock(gate->mutex);
    return status;
}
static void BlockQueue(UmiTaskQueue *queue,QueueGate *gate)
{
    REQUIRE(umi_mutex_create(&gate->mutex)==UMI_STATUS_OK);
    REQUIRE(umi_condition_create(&gate->condition)==UMI_STATUS_OK);
    UmiTaskConfig config={.label="native quality test gate",.function=HoldQueue,.user_data=gate};
    REQUIRE(umi_task_create(&config,&gate->task)==UMI_STATUS_OK);
    REQUIRE(umi_task_queue_submit(queue,gate->task)==UMI_STATUS_OK);
    REQUIRE(umi_mutex_lock(gate->mutex)==UMI_STATUS_OK);
    while(!gate->entered)
        REQUIRE(umi_condition_wait_for(gate->condition,gate->mutex,10000U)==UMI_STATUS_OK);
    REQUIRE(umi_mutex_unlock(gate->mutex)==UMI_STATUS_OK);
}
static void ReleaseQueue(UmiTaskQueue *queue,QueueGate *gate)
{
    if(gate->task==NULL)return;
    REQUIRE(umi_mutex_lock(gate->mutex)==UMI_STATUS_OK);
    gate->released=1;
    REQUIRE(umi_condition_broadcast(gate->condition)==UMI_STATUS_OK);
    REQUIRE(umi_mutex_unlock(gate->mutex)==UMI_STATUS_OK);
    REQUIRE(umi_task_queue_wait_idle(queue,10000U)==UMI_STATUS_OK);
    REQUIRE(umi_task_result(gate->task)==UMI_STATUS_OK);
    umi_task_destroy(gate->task);
    umi_condition_destroy(gate->condition);
    umi_mutex_destroy(gate->mutex);
    *gate=(QueueGate){0};
}
static GtkWidget *FindCommand(GtkWidget *root,const char *command)
{
    const char *id=g_object_get_data(G_OBJECT(root),"umicom-command-id");
    if(GTK_IS_BUTTON(root) && id!=NULL && strcmp(id,command)==0 &&
        g_object_get_data(G_OBJECT(root),"umicom-contribution-id")!=NULL) return root;
    for(GtkWidget *child=gtk_widget_get_first_child(root);child!=NULL;
        child=gtk_widget_get_next_sibling(child)) {
        GtkWidget *found=FindCommand(child,command);
        if(found!=NULL)return found;
    }
    return NULL;
}
static char *OutputText(GtkTextBuffer *buffer)
{
    GtkTextIter first,last;
    gtk_text_buffer_get_bounds(buffer,&first,&last);
    return gtk_text_buffer_get_text(buffer,&first,&last,FALSE);
}
typedef struct Observer {
    UmiStudioGtkWorkbench **workbench;
    UmiStudioServices *services;
    const char *other;
    const char *mode;
    unsigned calls;
    UmiStatus status;
} Observer;
static void ChangeOwner(Observer *observer,GObject *object)
{
    if(observer->calls++!=0U || *observer->workbench==NULL)return;
    if(strstr(observer->mode,"close")!=NULL) {
        UmiStudioGtkWorkbench *owner=*observer->workbench;
        *observer->workbench=NULL;
        umi_studio_gtk_workbench_destroy(owner);
    } else if(strstr(observer->mode,"switch")!=NULL) {
        observer->status=UmiStudioServicesPrepareWorkspace(observer->services,observer->other,1);
    } else if(strstr(observer->mode,"collapse")!=NULL) {
        gtk_expander_set_expanded(GTK_EXPANDER(object),FALSE);
    } else if(strcmp(observer->mode,"output-reenter")==0) {
        observer->status=umi_studio_gtk_workbench_workspace_synchronise(*observer->workbench);
    } else observer->status=umi_studio_gtk_workbench_execute_command(
        *observer->workbench,"codeguard.scan",NULL);
}
static void Expanded(GObject *object,GParamSpec *spec,gpointer data)
{
    (void)spec;
    ChangeOwner(data,object);
}
static void TextChanged(GtkTextBuffer *buffer,gpointer data)
{
    ChangeOwner(data,G_OBJECT(buffer));
}

int main(int argc,char **argv)
{
    if(argc!=2)return 2;
    const char *mode=argv[1];
    const char *cases[]={"menu","direct","output-button","untrusted","closed","argument","repeat","busy","cancel-menu","cancel-direct","cancel-closed","cancel-idle","retained-scan","retained-cancel","observer-close","observer-switch","observer-reenter","observer-collapse","output-close","output-switch","output-reenter","paused-context"
#ifdef UMICOM_STUDIO_CODEGUARD_NATIVE_FAULTS
,"poll-failure","poll-recovery","poll-context-failure"
#endif
};
    int known=0;
    for(size_t i=0U;i<sizeof cases/sizeof cases[0];++i)
        if(strcmp(mode,cases[i])==0)known=1;
    if(!known)return 2;
    if(!gtk_init_check())return 77;
    int failed=0;
    StudioTestState state={0};
    UmiSettings *settings=NULL;
    UmiStudioBootstrap *bootstrap=NULL;
    UmiStudioGtkWorkbench *workbench=NULL;
    GtkApplication *application=NULL;
    GtkWidget *retained=NULL,*observed=NULL;
    GtkTextBuffer *buffer=NULL;
    gulong observer_id=0U;
    char *project=NULL,*other=NULL,*source=NULL,*text=NULL;
    UmiStudioServices *services=NULL;
    UmiTaskQueue *queue=NULL;
    QueueGate gate={0};
    Observer observer={&workbench,NULL,NULL,mode,0U,UMI_STATUS_OK};
    CHECK(StudioTestStateBegin(&state));
    project=g_build_filename(state.directory,"project",NULL);
    other=g_build_filename(state.directory,"other",NULL);
    source=g_build_filename(project,"sample.c",NULL);
    CHECK(g_mkdir(project,0700)==0 && g_mkdir(other,0700)==0);
    CHECK(g_file_set_contents(source,"void sample(void) {\n    malloc(4);\n}\n",-1,NULL));
    CHECK(umi_studio_settings_create(&settings)==UMI_STATUS_OK);
    CHECK(umi_settings_set_integer(settings,UMI_STUDIO_SETTING_BUILD_PARALLEL_JOBS,1)==UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings,UMI_STUDIO_SETTING_AI_ALLOW_REMOTE,0)==UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings,UMI_STUDIO_SETTING_AI_PERSIST_SESSIONS,0)==UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings,UMI_STUDIO_SETTING_WORKSPACE_RESTORE_SESSION,0)==UMI_STATUS_OK);
    CHECK(umi_settings_set_boolean(settings,UMI_STUDIO_SETTING_AUTO_SAVE,0)==UMI_STATUS_OK);
    CHECK(umi_studio_settings_save(settings,umi_studio_settings_default_path())==UMI_STATUS_OK);
    UmiStudioServicesOptions service_options={0};
    CHECK(umi_studio_bootstrap_create_with_options(&service_options,&bootstrap)==UMI_STATUS_OK);
    UmiStudioUi *ui=umi_studio_bootstrap_ui(bootstrap);
    services=umi_studio_ui_services(ui);
    queue=umi_studio_services_task_queue(services);
    observer.services=services; observer.other=other;
    if(strcmp(mode,"closed")!=0)
        CHECK(UmiStudioServicesPrepareWorkspace(services,project,strcmp(mode,"untrusted")!=0)==UMI_STATUS_OK);
    application=gtk_application_new("org.umicom.studio.codeguard-native-test",G_APPLICATION_NON_UNIQUE);
    CHECK(g_application_register(G_APPLICATION(application),NULL,NULL));
    UmiStudioGtkWorkbenchOptions options={0};
    CHECK(umi_studio_gtk_workbench_create_with_options(application,ui,
        umi_studio_bootstrap_desktop_shell(bootstrap),&options,&workbench)==UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_open_surface(workbench,UMI_STUDIO_SURFACE_OUTPUT,NULL)==UMI_STATUS_OK);
    CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench)==UMI_STATUS_OK);
    GtkWidget *window=GTK_WIDGET(umi_studio_gtk_workbench_window(workbench));
    GtkWidget *menu=umi_gtk4_automation_find_tagged_widget(window,"umicom.shell.tools-menu.root");
    CHECK(GTK_IS_MENU_BUTTON(menu));
    GtkWidget *popover=GTK_WIDGET(gtk_menu_button_get_popover(GTK_MENU_BUTTON(menu)));
    GtkWidget *scan=FindCommand(popover,"codeguard.scan"),*cancel=FindCommand(popover,"codeguard.cancel");
    CHECK(GTK_IS_BUTTON(scan) && GTK_IS_BUTTON(cancel));
    GtkWidget *section=umi_gtk4_automation_find_tagged_widget(window,"studio.codeguard.section");
    GtkWidget *view=umi_gtk4_automation_find_tagged_widget(window,"codeguard.output.text");
    GtkWidget *follow=umi_gtk4_automation_find_tagged_widget(window,"codeguard.output.follow");
    CHECK(GTK_IS_EXPANDER(section) && GTK_IS_TEXT_VIEW(view) && GTK_IS_CHECK_BUTTON(follow));
    CHECK(umi_gtk4_automation_find_tagged_widget(window,"studio.build.output")!=NULL);
    buffer=g_object_ref(gtk_text_view_get_buffer(GTK_TEXT_VIEW(view)));
    CHECK(!gtk_widget_get_sensitive(cancel));
    gtk_expander_set_expanded(GTK_EXPANDER(section),FALSE);
    uint64_t submitted=umi_task_queue_stats(queue).submitted;
    UmiCodeGuardReviewSnapshot snapshot;
    if(strcmp(mode,"closed")==0 || strcmp(mode,"cancel-idle")==0) {
        const char *id=strcmp(mode,"closed")==0?"codeguard.scan":"codeguard.cancel";
        CHECK(umi_studio_gtk_workbench_execute_command(workbench,id,NULL)!=UMI_STATUS_OK);
        CHECK(umi_task_queue_stats(queue).submitted==submitted);
        goto cleanup;
    }
    CHECK(gtk_widget_get_sensitive(scan));
    if(strcmp(mode,"argument")==0) {
        CHECK(umi_studio_gtk_workbench_execute_command(workbench,"codeguard.scan","ignored-root")==UMI_STATUS_INVALID_ARGUMENT);
        CHECK(umi_task_queue_stats(queue).submitted==submitted);
        CHECK(!gtk_expander_get_expanded(GTK_EXPANDER(section)));
        goto cleanup;
    }
    if(strncmp(mode,"retained-",9U)==0) {
        retained=g_object_ref(strcmp(mode,"retained-scan")==0?scan:cancel);
        umi_studio_gtk_workbench_destroy(workbench);workbench=NULL;
        g_signal_emit_by_name(retained,"clicked");
        CHECK(umi_task_queue_stats(queue).submitted==submitted);
        goto cleanup;
    }
    if(strncmp(mode,"observer-",9U)==0) {
        observed=g_object_ref(section);
        observer_id=g_signal_connect(observed,"notify::expanded",G_CALLBACK(Expanded),&observer);
        UmiStatus status=umi_studio_gtk_workbench_execute_command(workbench,"codeguard.scan",NULL);
        CHECK(observer.calls!=0U);
        if(strcmp(mode,"observer-reenter")==0) {
            CHECK(observer.status!=UMI_STATUS_OK && status==UMI_STATUS_OK);
            CHECK(umi_task_queue_stats(queue).submitted==submitted+1U);
        } else {
            CHECK(status==UMI_STATUS_CANCELLED);
            CHECK(umi_task_queue_stats(queue).submitted==submitted);
            if(strcmp(mode,"observer-switch")==0)CHECK(observer.status==UMI_STATUS_OK);
        }
        goto cleanup;
    }
    int held=strcmp(mode,"busy")==0 || strncmp(mode,"cancel-",7U)==0;
    /* Output observers must see publication after they are attached, even on
     * a very fast machine. Keep the initial command's poll deterministically queued. */
    int observe_output=strncmp(mode,"output-",7U)==0 && strcmp(mode,"output-button")!=0;
    if(held || observe_output)BlockQueue(queue,&gate);
    if(strcmp(mode,"menu")==0)g_signal_emit_by_name(scan,"clicked");
    else if(strcmp(mode,"output-button")==0) {
        GtkWidget *button=umi_gtk4_automation_find_tagged_widget(section,"codeguard.scan");
        CHECK(GTK_IS_BUTTON(button));
        g_signal_emit_by_name(button,"clicked");
    } else CHECK(umi_studio_gtk_workbench_execute_command(workbench,"codeguard.scan",NULL)==UMI_STATUS_OK);
    CHECK(gtk_expander_get_expanded(GTK_EXPANDER(section)));
    CHECK(UmiStudioCodeGuardSnapshot(services,&snapshot)==UMI_STATUS_OK);
    CHECK(snapshot.request_id==1U);
    if(held) {
        CHECK(snapshot.pending);
        CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench)==UMI_STATUS_OK);
        CHECK(!gtk_widget_get_sensitive(scan) && gtk_widget_get_sensitive(cancel));
        if(strcmp(mode,"busy")==0) {
            uint64_t before=umi_task_queue_stats(queue).submitted;
            CHECK(umi_studio_gtk_workbench_execute_command(workbench,"codeguard.scan",NULL)!=UMI_STATUS_OK);
            CHECK(umi_task_queue_stats(queue).submitted==before);
            CHECK(umi_studio_gtk_workbench_execute_command(workbench,"codeguard.cancel",NULL)==UMI_STATUS_OK);
        } else {
            if(strcmp(mode,"cancel-closed")==0) {
                CHECK(umi_studio_workspace_close(services)==UMI_STATUS_OK);
                /* Dispatch queries the closed graph directly. Do not poll first:
                 * a stale queued job may already retire during that observation. */
                CHECK(gtk_widget_get_sensitive(cancel));
            }
            if(strcmp(mode,"cancel-menu")==0)g_signal_emit_by_name(cancel,"clicked");
            else CHECK(umi_studio_gtk_workbench_execute_command(workbench,"codeguard.cancel",NULL)==UMI_STATUS_OK);
        }
        CHECK(UmiStudioCodeGuardSnapshot(services,&snapshot)==UMI_STATUS_OK && snapshot.cancel_requested);
        ReleaseQueue(queue,&gate);
        CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench)==UMI_STATUS_OK);
        CHECK(UmiStudioCodeGuardSnapshot(services,&snapshot)==UMI_STATUS_OK);
        CHECK(!snapshot.pending && !snapshot.has_report && snapshot.status!=UMI_STATUS_OK);
        goto cleanup;
    }
    if(observe_output)ReleaseQueue(queue,&gate);
    CHECK(umi_task_queue_wait_idle(queue,10000U)==UMI_STATUS_OK);
    if(strncmp(mode,"output-",7U)==0 && strcmp(mode,"output-button")!=0) {
        /* Attach before the completed report is published. Opening the section
         * earlier does not count as testing native text callback ownership. */
        observed=g_object_ref(view);
        observer_id=g_signal_connect(buffer,"changed",G_CALLBACK(TextChanged),&observer);
    }
    UmiStatus synced=umi_studio_gtk_workbench_workspace_synchronise(workbench);
    if(strcmp(mode,"output-close")==0) {
        CHECK(observer.calls!=0U && workbench==NULL && synced==UMI_STATUS_CANCELLED);
        goto cleanup;
    }
    CHECK(synced==UMI_STATUS_OK);
    if(strcmp(mode,"output-switch")==0) {
        CHECK(observer.calls!=0U && observer.status==UMI_STATUS_OK);
        text=OutputText(buffer);CHECK(text[0]=='\0');
        CHECK(UmiStudioCodeGuardResult(services)==NULL);
        goto cleanup;
    }
    if(strcmp(mode,"output-reenter")==0) CHECK(observer.calls!=0U && observer.status==UMI_STATUS_OK);
    CHECK(UmiStudioCodeGuardSnapshot(services,&snapshot)==UMI_STATUS_OK);
    CHECK(!snapshot.pending && snapshot.has_report && snapshot.summary.total>0U);
    text=OutputText(buffer);CHECK(text!=NULL && strstr(text,"sample.c")!=NULL);
    g_clear_pointer(&text,g_free);
    if(strcmp(mode,"repeat")==0) {
        CHECK(umi_studio_gtk_workbench_execute_command(workbench,"codeguard.scan",NULL)==UMI_STATUS_OK);
        CHECK(UmiStudioCodeGuardSnapshot(services,&snapshot)==UMI_STATUS_OK && snapshot.request_id==2U);
    } else if(strcmp(mode,"paused-context")==0) {
        gtk_check_button_set_active(GTK_CHECK_BUTTON(follow),FALSE);
        CHECK(UmiStudioServicesPrepareWorkspace(services,other,1)==UMI_STATUS_OK);
        CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench)==UMI_STATUS_OK);
        text=OutputText(buffer);CHECK(text[0]=='\0');
        CHECK(!gtk_check_button_get_active(GTK_CHECK_BUTTON(follow)));
    }
#ifdef UMICOM_STUDIO_CODEGUARD_NATIVE_FAULTS
    if(strncmp(mode,"poll-",5U)==0) {
        if(strcmp(mode,"poll-context-failure")==0)
            CHECK(UmiStudioServicesPrepareWorkspace(services,other,1)==UMI_STATUS_OK);
        fail_poll=1;
        CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench)==UMI_STATUS_IO_ERROR);
        CHECK(poll_failures!=0U);
        text=OutputText(buffer);CHECK(text[0]=='\0');
        g_clear_pointer(&text,g_free);
        fail_poll=0;
        if(strcmp(mode,"poll-recovery")==0) {
            CHECK(umi_studio_gtk_workbench_workspace_synchronise(workbench)==UMI_STATUS_OK);
            text=OutputText(buffer);CHECK(strstr(text,"sample.c")!=NULL);
        }
    }
#endif
cleanup:
#ifdef UMICOM_STUDIO_CODEGUARD_NATIVE_FAULTS
    fail_poll=0;
#endif
    if(observer_id!=0U) {
        if(strncmp(mode,"output-",7U)==0)g_signal_handler_disconnect(buffer,observer_id);
        else g_signal_handler_disconnect(observed,observer_id);
    }
    if(queue!=NULL) {
        ReleaseQueue(queue,&gate);
        (void)umi_task_queue_wait_idle(queue,10000U);
    }
    if(workbench!=NULL)umi_studio_gtk_workbench_destroy(workbench);
    g_clear_object(&retained);
    g_clear_object(&observed);
    g_clear_object(&buffer);
    if(bootstrap!=NULL)umi_studio_bootstrap_destroy(bootstrap);
    if(settings!=NULL)umi_settings_destroy(settings);
    g_clear_object(&application);
    g_free(text);g_free(source);g_free(other);g_free(project);
    if(!StudioTestStateEnd(&state))failed=1;
    return failed;
}
