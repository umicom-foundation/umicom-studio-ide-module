![Umicom](assets/umicom-logo.svg)

# Keep one build owner when calling Studio services

Studio's menu commands already refuse conflicting build actions while background work owns the project. Direct service callers now use the same `UmiStudioBuildBusy` observation before starting synchronous work or changing the build graph.

This applies to phase execution, execute-next, execute-all, standard-graph preparation, retry and invalidation. Profile replacement already used this boundary and remains covered. The change keeps these application entry points thin: Framework still owns process execution, graph transitions, cancellation, history and project-session state.

## Understand why the service is busy

| Observation | Why admission remains closed |
| --- | --- |
| Active project-session worker | It still owns its captured request and execution lifetime. |
| Completed results not yet collected | The frontend has not retained and acknowledged all outcomes. |
| Deferred restart pending | Current project authority must be rechecked before another submission. |
| Session or restart observation failed | The service cannot prove that the existing owner is idle. |

A terminal worker is therefore not automatically an idle service. Its results may still need collection, and a deferred restart may still be pending.

## Call from the owning thread

1. Obtain the current Studio build service from the host.
2. Perform the existing project and trust checks for the intended action.
3. Call the service entry point and inspect its returned status.
4. On `BUSY`, preserve the current profile, graph, selected operation and displayed result.
5. Continue normal completion collection or explicit cancellation before retrying admission.

~~~c
UmiStatus status = umi_studio_build_service_run(
    build, UMI_BUILD_PHASE_BUILD, result);
if (status == UMI_STATUS_BUSY) {
    /* Keep the current presentation; result was not changed by this refusal. */
} else {
    /* Interpret the actual result according to the synchronous runner contract. */
}
~~~

The busy check does not grant workspace trust or replace frontend project validation. It is also not a cross-thread mutex. These service mutations belong on the host's owning thread; the underlying Framework worker has its own synchronization.

The graph getter remains a borrowed lower-level interface. A caller that mutates that graph directly is responsible for respecting the same owner lifetime. Prefer the service operations for normal frontend commands.

## Keep Stop and observation available

Cancellation remains available while the service is busy. It requests cancellation from the engine and project session, including the session's restart handling. That request is not proof of completed teardown. Keep collecting results and observing the owner until the existing lifecycle permits another action.

Read-only presentation remains available too. A busy refusal leaves output buffers unchanged, including execute-all's caller-owned count. Do not treat an old buffer or count as the outcome of the refused call.

## Diagnose surprising refusals

If a worker completed but the service stays busy, inspect whether every completed phase has been collected and acknowledged. If restart is pending, follow the normal restart review or explicit Stop path. If state observation fails, preserve that error evidence rather than guessing idle and starting another operation.

Do not clear the project-session pointer, reset counters or replace the profile to force admission. Those actions can disconnect live owners from their results. The service-level guard protects direct clients, but native command feedback and shutdown still need separate qualification.

## Owner qualification

After integrating Framework and Studio and configuring the Applications preset, run each command separately and stop on errors:

~~~powershell
Set-Location "C:\umicom\Umicom-Applications"
cmake --build --preset windows-ucrt64-all-debug --target umicom-studio-build-service-admission-test
ctest --test-dir build/windows-ucrt64-all-debug -R "^studio[.]build_service[.]admission[.]" --output-on-failure
~~~

The expected result is 48 passing scenarios: eight actions across active, uncollected, pending-restart, snapshot-error, restart-error and idle states. The fixture compiles the actual service with private symbol aliases and controlled session observations. Real profile, graph, history, workspace and cancellation owners remain in use. Execution ports are controlled so a missing guard cannot start a compiler.

Busy cases compare service and presentation snapshots, retained results, cancellation generation and untouched caller output. Idle cases prove that the service still delegates. Cancel cases prove that Stop remains available in every observed state.

Compilation, linking, these tests, native menu feedback, actual worker collection and restart behavior remain owner-run and unverified. Static source inspection is evidence of the authored boundary, not a native workflow pass.
