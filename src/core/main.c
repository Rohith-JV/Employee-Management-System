#include "ems.h"

int main(void) {
    EMSData data;
    initializeData(&data);

    int choice = 0;
    while (choice != 13) {
        printMenu();
        scanf("%d", &choice);
        while (getchar() != '\n') {
        }

        switch (choice) {
            case 1:
                showModuleMenu(&data, "Employee onboarding", addEmployee, listEmployees);
                break;
            case 2:
                showModuleMenu(&data, "Employee records", addEmployee, listEmployees);
                break;
            case 3:
                showModuleMenu(&data, "Department management", addDepartment, listDepartments);
                break;
            case 4:
                showModuleMenu(&data, "Attendance management", addAttendance, listAttendance);
                break;
            case 5:
                showModuleMenu(&data, "Leave management", addLeaveRequest, listLeaveRequests);
                break;
            case 6:
                employeeLogin(&data);
                break;
            case 7:
                hrLogin(&data);
                break;
            case 8:
                showModuleMenu(&data, "Login & access management", addAccessAccount, listAccessAccounts);
                break;
            case 9:
                showModuleMenu(&data, "Role management", addRole, listRoles);
                break;
            case 10:
                showModuleMenu(&data, "Payroll", addPayroll, listPayroll);
                break;
            case 11:
                showModuleMenu(&data, "Project orientation", addProjectOrientation, listProjectOrientations);
                break;
            case 12:
                showReportingDashboard(&data);
                break;
            case 13:
                saveAll(&data);
                printf("Data saved successfully. Exiting.\n");
                break;
            default:
                printf("Invalid choice. Try again.\n");
                break;
        }
    }

    return 0;
}
