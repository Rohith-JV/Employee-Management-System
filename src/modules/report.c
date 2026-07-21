#include "ems.h"

void showReportingDashboard(const EMSData *data) {
    int presentCount = 0;
    int activeAccounts = 0;
    int completedOrientations = 0;

    for (int i = 0; i < data->attendanceCount; ++i) {
        if (data->attendance[i].status == 1) {
            presentCount++;
        }
    }

    for (int i = 0; i < data->accountCount; ++i) {
        if (data->accounts[i].active) {
            activeAccounts++;
        }
    }

    for (int i = 0; i < data->projectCount; ++i) {
        if (data->projects[i].completed) {
            completedOrientations++;
        }
    }

    printf("\n=== Reporting Dashboard ===\n");
    printf("Employees: %d\n", data->employeeCount);
    printf("Departments: %d\n", data->departmentCount);
    printf("Roles: %d\n", data->roleCount);
    printf("Attendance records: %d\n", data->attendanceCount);
    printf("Present today: %d\n", presentCount);
    printf("Leave requests: %d\n", data->leaveCount);
    printf("Active accounts: %d\n", activeAccounts);
    printf("Project orientations: %d\n", data->projectCount);
    printf("Completed orientations: %d\n", completedOrientations);
    printf("Payroll records: %d\n", data->payrollCount);
}
