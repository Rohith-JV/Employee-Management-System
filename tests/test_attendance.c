#include "test_framework.h"
#include "test_helpers.h"
void testAttendance(void) { EMSData data; setupTestData(&data); TEST_ASSERT(testSetInput("1\n2026-07-30\n1\n")); TEST_ASSERT(addAttendance(&data)); TEST_ASSERT(data.attendanceCount == 1); TEST_ASSERT(data.attendance[0].employeeId == 1); teardownTestData(&data); }
