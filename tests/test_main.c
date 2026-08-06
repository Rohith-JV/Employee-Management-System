#include "test_framework.h"
#include "test_helpers.h"

int testFailures = 0;
void testCore(void); void testEmployee(void); void testDepartment(void); void testRole(void);
void testAttendance(void); void testPayroll(void); void testLeave(void); void testAccess(void);
void testProject(void); void testReport(void);
void testEmployeeUser(void);
void testEms(void);
void testPopulate(void);
void testIntegration(void);

int main(void) {
    if (!useTestSandbox()) {
        fprintf(stderr, "Could not create the test sandbox directory.\n");
        return 1;
    }
    testCore(); testEmployee(); testDepartment(); testRole(); testAttendance();
    testPayroll(); testLeave(); testAccess(); testProject(); testReport();
    testEms();
    testPopulate();
    testIntegration();
    testEmployeeUser();
    if (testFailures == 0) printf("All unit tests passed.\n");
    return testFailures == 0 ? 0 : 1;
}
