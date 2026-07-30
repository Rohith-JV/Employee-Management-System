#ifndef EMS_TEST_HELPERS_H
#define EMS_TEST_HELPERS_H

#include "../src/ems.h"

void setupTestData(EMSData *data);
void teardownTestData(EMSData *data);
int testSetInput(const char *text);

#endif
