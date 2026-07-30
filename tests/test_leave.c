#include "test_framework.h"
#include "test_helpers.h"
void testLeave(void) { EMSData data; setupTestData(&data); TEST_ASSERT(testSetInput("1\n2026-08-01\n2026-08-03\nFamily event\n1\n")); TEST_ASSERT(addLeaveRequest(&data)); TEST_ASSERT(data.leaveCount == 1); TEST_ASSERT(data.leaves[0].employeeId == 1); teardownTestData(&data); }
