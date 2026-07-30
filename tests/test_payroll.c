#include "test_framework.h"
#include "test_helpers.h"
void testPayroll(void) { EMSData data; setupTestData(&data); TEST_ASSERT(testSetInput("1\n2026-07\n50000\n2000\n")); TEST_ASSERT(addPayroll(&data)); TEST_ASSERT(data.payrollCount == 1); TEST_ASSERT(data.payroll[0].netPay == 48000.0); teardownTestData(&data); }
