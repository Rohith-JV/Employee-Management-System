#include "test_framework.h"
#include "test_helpers.h"

void testCore(void) {
    EMSData data;
    char buffer[5];
    setupTestData(&data);
    TEST_ASSERT(copyStringSafe(buffer, sizeof(buffer), "abcd"));
    TEST_ASSERT(!copyStringSafe(buffer, sizeof(buffer), "abcde"));
    TEST_ASSERT(isEmail("user@example.com"));
    TEST_ASSERT(!isEmail("not-an-email"));
    TEST_ASSERT(isPhone("+919876543210"));
    TEST_ASSERT(isDate("2026-07-30"));
    TEST_ASSERT(isMonth("2026-07"));
    TEST_ASSERT(isUsername("ram_01"));
    TEST_ASSERT(isPassword("ram2104"));
    TEST_ASSERT(employeeExists(&data, 1));
    TEST_ASSERT(departmentExists(&data, 1));
    TEST_ASSERT(roleExists(&data, EMPLOYEE_ROLE_ID));
    teardownTestData(&data);
}
