#include "test_framework.h"
#include "test_helpers.h"
void testDepartment(void) { EMSData data; setupTestData(&data); TEST_ASSERT(testSetInput("Finance\nRavi Kumar\n")); TEST_ASSERT(addDepartment(&data)); TEST_ASSERT(data.departmentCount == 2); TEST_ASSERT(departmentExists(&data, 2)); teardownTestData(&data); }
