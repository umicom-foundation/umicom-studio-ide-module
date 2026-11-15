/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_codeguard_jobs.c
 *
 * PURPOSE:
 *   Qualify thin Studio job preparation and retained-result adoption through Framework.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

#include "umicom/studio/codeguard.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"%d: %s\n",__LINE__,#x); exit(1); } } while (0)
int main(int argc,char **argv)
{
    if (argc != 3) return 2;
    const char *name=argv[1];
    const char *cases[]={"arguments","success","empty","busy","cancelled","budget",
        "foreign-workspace","facade-destroyed","repeat"};
    size_t selected=0U;
    while(selected<sizeof cases/sizeof cases[0] && strcmp(name,cases[selected])) ++selected;
    if(selected==sizeof cases/sizeof cases[0]) return 2;
    char root[1024],other_root[1024];
    int written=snprintf(root,sizeof root,"%s/workspace",argv[2]);
    CHECK(written>0 && (size_t)written<sizeof root);
    written=snprintf(other_root,sizeof other_root,"%s/other",argv[2]);
    CHECK(written>0 && (size_t)written<sizeof other_root);
    UmiStudioCodeGuard *guard=NULL,*other=NULL;
    CHECK(umi_studio_codeguard_create(root,&guard)==UMI_STATUS_OK);
    UmiCodeGuardProfile profile=umi_codeguard_profile_default();
    profile.scan_architecture=0;
    profile.scan_source_names=0;
    profile.scan_duplicates=0;
    profile.max_line_length=SIZE_MAX;
    CHECK(umi_studio_codeguard_scan(guard,&profile)==UMI_STATUS_OK);
    const UmiCodeGuardResult *retained=umi_studio_codeguard_result(guard);
    const UmiCodeGuardFinding *borrow=umi_codeguard_result_at(retained,0U);
    size_t count=umi_codeguard_result_count(retained);
    CHECK(count>0U && borrow!=NULL);
    if(!strcmp(name,"empty")) profile.scan_c=0;
    if(!strcmp(name,"budget")) profile.max_file_bytes=1U;
    UmiCodeGuardJob *job=NULL;
    CHECK(umi_studio_codeguard_create_job(guard,&profile,&job)==UMI_STATUS_OK);
    if(!strcmp(name,"arguments")) {
        UmiCodeGuardJob *output=job;
        CHECK(umi_studio_codeguard_create_job(NULL,&profile,&output)==UMI_STATUS_INVALID_ARGUMENT && output==NULL);
        output=job;
        CHECK(umi_studio_codeguard_create_job(guard,NULL,&output)==UMI_STATUS_INVALID_ARGUMENT && output==NULL);
        CHECK(umi_studio_codeguard_create_job(guard,&profile,NULL)==UMI_STATUS_INVALID_ARGUMENT);
        CHECK(umi_studio_codeguard_adopt_job(NULL,job)==UMI_STATUS_INVALID_ARGUMENT);
        CHECK(umi_studio_codeguard_adopt_job(guard,NULL)==UMI_STATUS_INVALID_ARGUMENT);
    }
    /* Releasing the facade before scheduling proves the worker borrows neither
     * the Studio wrapper nor its retained Framework service. */
    if(!strcmp(name,"facade-destroyed")) {
        umi_studio_codeguard_destroy(guard);
        guard=NULL;
    }
    UmiTaskQueue *queue=NULL;
    UmiStatus expected=UMI_STATUS_OK;
    if(!strcmp(name,"busy") || !strcmp(name,"arguments")) expected=UMI_STATUS_BUSY;
    else if(!strcmp(name,"cancelled")) {
        CHECK(UmiCodeGuardJobCancel(job)==UMI_STATUS_OK);
        expected=UMI_STATUS_CANCELLED;
    } else {
        UmiTaskQueueConfig config={.worker_count=1U,.capacity=2U};
        CHECK(umi_task_queue_create(&config,&queue)==UMI_STATUS_OK);
        CHECK(UmiCodeGuardJobSubmit(job,queue)==UMI_STATUS_OK);
        CHECK(umi_task_queue_wait_idle(queue,10000U)==UMI_STATUS_OK);
    }
    if(!strcmp(name,"budget")) expected=UMI_STATUS_CAPACITY_EXCEEDED;
    if(guard==NULL) {
        UmiCodeGuardResult *result=NULL;
        CHECK(UmiCodeGuardJobTake(job,&result)==UMI_STATUS_OK);
        CHECK(umi_codeguard_result_count(result)==count);
        umi_codeguard_result_destroy(result);
    } else {
        if(!strcmp(name,"foreign-workspace")) {
            CHECK(umi_studio_codeguard_create(other_root,&other)==UMI_STATUS_OK);
            CHECK(umi_studio_codeguard_adopt_job(other,job)==UMI_STATUS_INVALID_STATE);
            CHECK(umi_studio_codeguard_summary(other).total==0U);
            UmiCodeGuardJobSnapshot snapshot;
            CHECK(UmiCodeGuardJobRead(job,&snapshot)==UMI_STATUS_OK && snapshot.result_available);
        }
        CHECK(umi_studio_codeguard_adopt_job(guard,job)==expected);
        CHECK(umi_studio_codeguard_result(guard)==retained);
        if(expected!=UMI_STATUS_OK) {
            CHECK(umi_codeguard_result_at(retained,0U)==borrow);
            CHECK(umi_codeguard_result_count(retained)==count);
        } else {
            CHECK(umi_codeguard_result_count(retained)==(!strcmp(name,"empty") ? 0U : count));
            CHECK(umi_studio_codeguard_summary(guard).total==umi_codeguard_result_count(retained));
            if(!strcmp(name,"repeat")) {
                const UmiCodeGuardFinding *published=umi_codeguard_result_at(retained,0U);
                CHECK(umi_studio_codeguard_adopt_job(guard,job)==UMI_STATUS_NOT_FOUND);
                CHECK(umi_codeguard_result_at(retained,0U)==published);
            }
        }
    }
    CHECK(UmiCodeGuardJobDestroy(&job)==UMI_STATUS_OK && job==NULL);
    umi_studio_codeguard_destroy(other);
    umi_studio_codeguard_destroy(guard);
    if(queue!=NULL) {
        CHECK(umi_task_queue_shutdown(queue,0)==UMI_STATUS_OK);
        umi_task_queue_destroy(queue);
    }
    return 0;
}
