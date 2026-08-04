#include "test_framework.h"
#include "test_helpers.h"
#include <string.h>

void testIntegration(void) {
    EMSData data;
    EMSData reloaded;

    setupTestData(&data);
    memset(&reloaded, 0, sizeof(reloaded));

    TEST_ASSERT(testSetInput("Engineering\nJane Doe\n"));
    TEST_ASSERT(addDepartment(&data));

    TEST_ASSERT(testSetInput("Software Engineer\nBuilds and maintains systems\n"));
    TEST_ASSERT(addRole(&data));

    TEST_ASSERT(testSetInput("Alice Johnson\n1\n2\n50000\nalice@example.com\n+1234567890\n2025-01-15\nActive\nalice.smith\nTemp123!\n"));
    TEST_ASSERT(addEmployee(&data));

    TEST_ASSERT(testSetInput("1\n2025-01-16\n1\n"));
    TEST_ASSERT(addAttendance(&data));

    TEST_ASSERT(testSetInput("1\n2025-02\n50000\n2000\n"));
    TEST_ASSERT(addPayroll(&data));

    TEST_ASSERT(testSetInput("1\n2025-02-01\n2025-02-05\nMedical appointment\n1\n"));
    TEST_ASSERT(addLeaveRequest(&data));

    TEST_ASSERT(data.departmentCount == 2);
    TEST_ASSERT(data.roleCount == 3);
    TEST_ASSERT(data.employeeCount == 2);
    TEST_ASSERT(data.attendanceCount == 1);
    TEST_ASSERT(data.payrollCount == 1);
    TEST_ASSERT(data.leaveCount == 1);

    saveAll(&data);
    initializeData(&reloaded);

    TEST_ASSERT(reloaded.departmentCount == data.departmentCount);
    TEST_ASSERT(reloaded.roleCount == data.roleCount);
    TEST_ASSERT(reloaded.employeeCount == data.employeeCount);
    TEST_ASSERT(reloaded.attendanceCount == data.attendanceCount);
    TEST_ASSERT(reloaded.payrollCount == data.payrollCount);
    TEST_ASSERT(reloaded.leaveCount == data.leaveCount);
    TEST_ASSERT(reloaded.accountCount >= 2);

    teardownTestData(&data);
    teardownTestData(&reloaded);
}
