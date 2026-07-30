#include "test_framework.h"
#include "test_helpers.h"
void testProject(void) { EMSData data; setupTestData(&data); TEST_ASSERT(testSetInput("1\nOnboarding\n2026-07-30\n1\n")); TEST_ASSERT(addProjectOrientation(&data)); TEST_ASSERT(data.projectCount == 1); TEST_ASSERT(data.projects[0].completed == 1); teardownTestData(&data); }
