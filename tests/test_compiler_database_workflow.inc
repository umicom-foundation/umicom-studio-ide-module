/*-----------------------------------------------------------------------------
 * Umicom Studio IDE
 * File: tests/test_compiler_database_workflow.inc
 * PURPOSE: Verify that a Studio-configured C23 project exports the compiler command used for its source.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/

/* This runs inside the real project workflow after Configure/Build/Run. Reading
 * the generated database never invokes the compiler recorded inside its rows. */
static int VerifyCompilerDatabaseWorkflow(const UmiBuildProfile *profile, const char *source)
{
    char root[UMI_BUILD_PATH_CAPACITY], build[UMI_BUILD_PATH_CAPACITY];
    CHECK(UmiBuildProfileSourceDirectory(profile, root, sizeof root) == UMI_STATUS_OK);
    CHECK(umi_path_absolute(profile->build_directory, root, build, sizeof build) == UMI_STATUS_OK);
    UmiCompilationDatabase *database = NULL;
    CHECK(UmiCompilationDatabaseRead(build, NULL, &database) == UMI_STATUS_OK);
    CHECK(UmiCompilationDatabaseCount(database) != 0U);
    size_t row_index = 0U;
    CHECK(UmiCompilationDatabaseFind(database, source, 0U, &row_index) == UMI_STATUS_OK);
    UmiCompilationCommand row;
    CHECK(UmiCompilationDatabaseAt(database, row_index, &row) == UMI_STATUS_OK);
    CHECK(umi_path_equal(row.source_file, source));
    CHECK(row.has_arguments || row.has_command);
    char server_arguments[2048] = "--background-index";
    CHECK(UmiCompilationDatabaseClangdArguments(server_arguments, build,
        server_arguments, sizeof server_arguments) == UMI_STATUS_OK);
    UmiArguments *arguments = malloc(sizeof *arguments);
    CHECK(arguments != NULL);
    CHECK(UmiArgumentsParse(server_arguments, arguments) == UMI_STATUS_OK);
    CHECK(arguments->count == 2U);
    CHECK(strncmp(arguments->values[1], "--compile-commands-dir=", 23U) == 0);
    CHECK(umi_path_equal(arguments->values[1] + 23U, build));
    free(arguments);
    UmiCompilationDatabaseDestroy(database);
    return EXIT_SUCCESS;
}
