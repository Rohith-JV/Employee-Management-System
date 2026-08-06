#include "test_helpers.h"

#ifdef _WIN32
#include <direct.h>
#define ems_mkdir(path) _mkdir(path)
#define ems_chdir(path) _chdir(path)
#else
#include <sys/stat.h>
#include <unistd.h>
#define ems_mkdir(path) mkdir(path, 0777)
#define ems_chdir(path) chdir(path)
#endif

/* Run the suite against a throwaway data directory so the seeded files in the
   repository data directory are never overwritten by a test run. */
int useTestSandbox(void) {
    ems_mkdir("build");
    ems_mkdir("build/test-sandbox");
    if (ems_chdir("build/test-sandbox") != 0) {
        return 0;
    }
    ems_mkdir("data");
    return 1;
}

void setupTestData(EMSData *data) {
    memset(data, 0, sizeof(*data));
    ems_mutex_init(&data->mutex);
    ems_condition_init(&data->saveRequested);

    data->roles[0].id = HR_ROLE_ID;
    copyStringSafe(data->roles[0].title, sizeof(data->roles[0].title), "HR");
    data->roles[1].id = EMPLOYEE_ROLE_ID;
    copyStringSafe(data->roles[1].title, sizeof(data->roles[1].title), "Employee");
    data->roleCount = 2;

    data->departments[0].id = 1;
    copyStringSafe(data->departments[0].name, sizeof(data->departments[0].name), "Engineering");
    data->departmentCount = 1;

    data->employees[0].id = 1;
    data->employees[0].active = 1;
    copyStringSafe(data->employees[0].name, sizeof(data->employees[0].name), "Test Employee");
    data->employeeCount = 1;
}

void teardownTestData(EMSData *data) {
    ems_mutex_destroy(&data->mutex);
    ems_condition_destroy(&data->saveRequested);
}

int testSetInput(const char *text) {
    FILE *file = fopen(".test_input.txt", "w");
    if (file == NULL) return 0;
    fputs(text, file);
    fclose(file);
    return freopen(".test_input.txt", "r", stdin) != NULL;
}
