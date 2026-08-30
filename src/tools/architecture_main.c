/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: applications/studio/src/tools/architecture_main.c
 *
 * PURPOSE:
 *   Run the Studio architecture guard across reference products.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* BEGINNER NOTE:
 * This command exposes one compatibility check in a form that is easy to run from PowerShell, CI, or the future Studio workbench UI.
 */
#include <stdio.h>
#include "umicom/studio/architecture_guard.h"
int main(void){UmiStudioArchitectureGuard r;(void)umi_studio_architecture_guard_run(&r);printf("Profiles: %zu\nInvalid: %zu\nIncompatible: %zu\nArchitecture guard: %s\n",r.profiles_checked,r.invalid_profiles,r.incompatible_profiles,r.passed?"PASS":"FAIL");return r.passed?0:2;}
