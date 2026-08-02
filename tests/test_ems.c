#include "test_framework.h"
#include "test_helpers.h"
#include <string.h>
#include <math.h>

void testEms(void) {
    EMSData data;
    char buffer[64];
    double dval;

    setupTestData(&data);

    TEST_ASSERT(isAlphaText("John Doe"));
    TEST_ASSERT(!isAlphaText("John123"));

    TEST_ASSERT(isStatusValue("Active"));
    TEST_ASSERT(!isStatusValue("Unknown"));

    TEST_ASSERT(isTextWithSpaces("Hello, world."));
    TEST_ASSERT(!isTextWithSpaces("Hello@"));

    /* readText trims newline and surrounding spaces */
    TEST_ASSERT(testSetInput("  hello world  \n"));
    readText("Prompt: ", buffer, sizeof(buffer));
    TEST_ASSERT(strcmp(buffer, "hello world") == 0);

    /* readInt valid/invalid */
    TEST_ASSERT(testSetInput("42\n"));
    TEST_ASSERT(readInt("I: ") == 42);
    TEST_ASSERT(testSetInput("abc\n"));
    TEST_ASSERT(readInt("I: ") == 0);

    /* readValidatedInt should skip invalid then accept valid */
    TEST_ASSERT(testSetInput("bad\n7\n"));
    TEST_ASSERT(readValidatedInt("V: ", 1, 10, "1-10") == 7);

    /* readDouble and validated double */
    TEST_ASSERT(testSetInput("3.14\n"));
    dval = readDouble("D: ");
    TEST_ASSERT(fabs(dval - 3.14) < 1e-6);

    TEST_ASSERT(testSetInput("nope\n2.5\n"));
    dval = readValidatedDouble("VD: ", 0.0, 10.0, "0-10");
    TEST_ASSERT(fabs(dval - 2.5) < 1e-6);

    teardownTestData(&data);
}
