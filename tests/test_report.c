#include "test_framework.h"
#include "test_helpers.h"
void testReport(void) { EMSData data; setupTestData(&data); showReportingDashboard(&data); TEST_ASSERT(data.employeeCount == 1); teardownTestData(&data); }
