#ifndef EMS_TEST_FRAMEWORK_H
#define EMS_TEST_FRAMEWORK_H

#include <stdio.h>

extern int testFailures;

#define TEST_ASSERT(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        ++testFailures; \
    } \
} while (0)

#endif
