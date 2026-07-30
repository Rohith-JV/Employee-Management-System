#include "test_framework.h"
#include "test_helpers.h"
void testEmployee(void) { EMSData data; setupTestData(&data); TEST_ASSERT(testSetInput("Ram Kumar\n1\n2\n30000\nram@example.com\n9876543210\n2026-07-30\nActive\nram\nram2104\n")); TEST_ASSERT(addEmployee(&data)); TEST_ASSERT(data.employeeCount == 2); TEST_ASSERT(data.accountCount == 1); teardownTestData(&data); }
