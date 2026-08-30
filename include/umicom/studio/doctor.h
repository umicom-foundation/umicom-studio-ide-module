/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: include/umicom/studio/doctor.h
 *
 * PURPOSE:
 *   Publish Studio's public doctor contract over reusable Framework services.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_STUDIO_DOCTOR_H
#define UMICOM_STUDIO_DOCTOR_H

#include <stddef.h>

#include "umicom/diagnostics/diagnostic.h"
#include "umicom/base/status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UmiStudioDoctorReport {
    size_t checks_passed;
    size_t checks_failed;
} UmiStudioDoctorReport;

UmiStatus umi_studio_doctor_run(const char *repository_root,
                                UmiDiagnosticSink sink,
                                void *user_data,
                                UmiStudioDoctorReport *out_report);

#ifdef __cplusplus
}
#endif

#endif
