#include "test_framework.h"
#include "test_helpers.h"

void testPopulate(void) {
    EMSData data;
    setupTestData(&data);

    /* Add roles: ensure HR(1) and Employee(2) exist, then add others */
    int nextRoleId = 3;
    const char *extraRoles[] = {"Trainee", "Manager", "Tester", "Developer"};
    for (size_t i = 0; i < sizeof(extraRoles)/sizeof(extraRoles[0]); ++i) {
        data.roles[data.roleCount].id = nextRoleId++;
        copyStringSafe(data.roles[data.roleCount].title, sizeof(data.roles[data.roleCount].title), extraRoles[i]);
        data.roleCount++;
    }

    /* Ensure total of 6 departments (add if missing) */
    int nextDeptId = data.departmentCount > 0 ? data.departments[data.departmentCount - 1].id + 1 : 1;
    while (data.departmentCount < 6) {
        data.departments[data.departmentCount].id = nextDeptId++;
        char name[32];
        snprintf(name, sizeof(name), "Department %d", data.departments[data.departmentCount].id);
        copyStringSafe(data.departments[data.departmentCount].name, sizeof(data.departments[data.departmentCount].name), name);
        data.departmentCount++;
    }

    /* Add 50 employees; keep first one from setupTestData and append */
    int startId = data.employeeCount > 0 ? data.employees[data.employeeCount-1].id + 1 : 1;
    int initialCount = data.employeeCount;
    for (int i = 0; i < 50 - initialCount; ++i) {
        Employee *e = &data.employees[data.employeeCount];
        memset(e, 0, sizeof(*e));
        e->id = startId + i;
        char ename[64];
        snprintf(ename, sizeof(ename), "Employee_%d", e->id);
        copyStringSafe(e->name, sizeof(e->name), ename);
        e->departmentId = (i % 6) + 1;
        /* cycle through available roles (skip index 0 if HR) */
        int roleStartIndex = 1; /* include Employee role at index 1 */
        int roleSlots = data.roleCount - roleStartIndex;
        if (roleSlots <= 0) {
            e->roleId = DEFAULT_EMPLOYEE_ROLE_ID;
        } else {
            int roleIndex = roleStartIndex + (i % roleSlots);
            e->roleId = data.roles[roleIndex].id;
        }
        e->salary = 30000 + (i * 100);
        snprintf(e->email, sizeof(e->email), "emp%d@example.com", e->id);
        snprintf(e->phone, sizeof(e->phone), "+100000000%02d", i % 100);
        copyStringSafe(e->joinDate, sizeof(e->joinDate), (i >= 45) ? "2026-08-01" : "2025-01-01");
        copyStringSafe(e->status, sizeof(e->status), (i >= 45) ? "Probation" : "Active");
        e->active = 1;
        data.employeeCount++;
    }

    /* Add 4 projects per department (24 projects) */
    int projId = 1;
    for (int d = 1; d <= 6; ++d) {
        for (int p = 1; p <= 4; ++p) {
            ProjectOrientation *proj = &data.projects[data.projectCount];
            memset(proj, 0, sizeof(*proj));
            proj->id = projId++;
            /* assign to some employee in that department */
            int assigned = -1;
            for (int ei = 0; ei < data.employeeCount; ++ei) {
                if (data.employees[ei].departmentId == d) { assigned = data.employees[ei].id; break; }
            }
            proj->employeeId = assigned > 0 ? assigned : data.employees[0].id;
            char pname[64];
            snprintf(pname, sizeof(pname), "Dept%d Project %d", d, p);
            copyStringSafe(proj->projectName, sizeof(proj->projectName), pname);
            copyStringSafe(proj->orientationDate, sizeof(proj->orientationDate), "2026-07-01");
            proj->completed = (p % 2 == 0) ? 1 : 0;
            data.projectCount++;
        }
    }

    /* Basic assertions to verify population */
    TEST_ASSERT(data.employeeCount == 50);
    TEST_ASSERT(data.departmentCount == 6);
    TEST_ASSERT(data.projectCount == 24);
    TEST_ASSERT(data.roleCount >= 5);

    /* Persist populated data to disk so runtime model looks realistic */
    saveAll(&data);

    /* Print reporting dashboard and lists for manual inspection */
    showReportingDashboard(&data);
    listEmployees(&data);
    listProjectOrientations(&data);

    teardownTestData(&data);
}
