#include "test_framework.h"
#include "test_helpers.h"

void testEmployeeUserAttendance(void) {
    EMSData data;
    setupTestData(&data);
    /* Ensure employee is present and assigned to a department */
    data.employees[0].departmentId = 1;
    /* Simulate date and status input */
    TEST_ASSERT(testSetInput("2026-08-02\n1\n"));
    TEST_ASSERT(markAttendance(&data, 1));
    TEST_ASSERT(data.attendanceCount == 1);
    TEST_ASSERT(data.attendance[0].employeeId == 1);
    teardownTestData(&data);
}

void testEmployeeUserLeave(void) {
    EMSData data;
    setupTestData(&data);
    data.employees[0].departmentId = 1;
    /* Simulate leave start, end, and reason */
    TEST_ASSERT(testSetInput("2026-08-05\n2026-08-06\nPersonal reason\n"));
    TEST_ASSERT(applyLeave(&data, 1));
    TEST_ASSERT(data.leaveCount == 1);
    TEST_ASSERT(data.leaves[0].employeeId == 1);
    teardownTestData(&data);
}

/* Register tests for the test runner */
void testEmployeeUser(void) {
    testEmployeeUserAttendance();
    testEmployeeUserLeave();
}
