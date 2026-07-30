#include "test_framework.h"
#include "test_helpers.h"
void testRole(void) { EMSData data; setupTestData(&data); TEST_ASSERT(testSetInput("Developer\nWrites code\n")); TEST_ASSERT(addRole(&data)); TEST_ASSERT(data.roleCount == 3); TEST_ASSERT(roleExists(&data, 3)); teardownTestData(&data); }
