#include "test_framework.h"
#include "test_helpers.h"
void testAccess(void) { EMSData data; setupTestData(&data); 
    TEST_ASSERT(createEmployeeAccessAccount(&data, 1, "ram", "ram2104")); 
    TEST_ASSERT(data.accountCount == 1); 
    TEST_ASSERT(data.accounts[0].passwordChangeRequired == 1); 
    teardownTestData(&data); }
