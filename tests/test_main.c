#include "test_framework.h"

int testFailures = 0;
void testCore(void); void testEmployee(void); void testDepartment(void); void testRole(void);
void testAttendance(void); void testPayroll(void); void testLeave(void); void testAccess(void);
void testProject(void); void testReport(void);

int main(void) {
    testCore(); testEmployee(); testDepartment(); testRole(); testAttendance();
    testPayroll(); testLeave(); testAccess(); testProject(); testReport();
    if (testFailures == 0) printf("All unit tests passed.\n");
    return testFailures == 0 ? 0 : 1;
}
