/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/quality_fixture/checked_source.c
 *
 * PURPOSE:
 *   Supply a bounded, documented source file for Studio quality-service integration checks.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/

/* Return a stable value so the fixture has one small, documented function.
 * There are no allocations, external commands or mutable global state. */
int studio_quality_fixture_value(void)
{
    return 17;
}
