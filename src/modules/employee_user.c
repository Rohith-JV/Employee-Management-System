#include "ems.h"

static const Employee *findEmployeeById(const EMSData *data, int32_t employeeId) {
    for (int32_t i = 0; i < data->employeeCount; ++i) {
        if (data->employees[i].id == employeeId) {
            return &data->employees[i];
        }
    }
    return NULL;
}

static int32_t nextEmployeeAttendanceId(const EMSData *data) {
    int32_t highest = 0;
    for (int32_t i = 0; i < data->attendanceCount; ++i) {
        if (data->attendance[i].id > highest) {
            highest = data->attendance[i].id;
        }
    }
    return highest + 1;
}

static int32_t nextEmployeeLeaveId(const EMSData *data) {
    int32_t highest = 0;
    for (int32_t i = 0; i < data->leaveCount; ++i) {
        if (data->leaves[i].id > highest) {
            highest = data->leaves[i].id;
        }
    }
    return highest + 1;
}

void showOwnProfile(EMSData *data, int32_t employeeId) {
    const Employee *employee = findEmployeeById(data, employeeId);
    if (!employee) {
        printf("Profile not found.\n");
        return;
    }

    printf("\n=== My Profile ===\n");
    printf("ID: %d\nName: %s\nDepartment ID: %d\nRole ID: %d\nSalary: %d\nEmail: %s\nPhone: %s\nJoin Date: %s\nStatus: %s\n",
           employee->id, employee->name, employee->departmentId, employee->roleId,
           employee->salary, employee->email, employee->phone, employee->joinDate, employee->status);
}

int markAttendance(EMSData *data, int32_t employeeId) {
    if (data->attendanceCount >= MAX_ATTENDANCE) {
        printf("Attendance storage is full.\n");
        return 0;
    }

    AttendanceRecord *record = &data->attendance[data->attendanceCount];
    memset(record, 0, sizeof(*record));

    record->id = nextEmployeeAttendanceId(data);
    record->employeeId = employeeId;
    readValidatedText("Date (YYYY-MM-DD): ", record->date, sizeof(record->date), isDate, "YYYY-MM-DD");
    record->status = readValidatedInt("Status (1=Present, 0=Absent): ", 0, 1, "0 or 1");

    data->attendanceCount++;
    markDataDirty(data);
    printf("Attendance recorded for employee %d.\n", employeeId);
    return 1;
}

void viewMyAttendance(const EMSData *data, int32_t employeeId) {
    int found = 0;
    printf("\n=== My Attendance ===\n");
    for (int32_t i = 0; i < data->attendanceCount; ++i) {
        if (data->attendance[i].employeeId == employeeId) {
            printf("%s | %s\n", data->attendance[i].date,
                   data->attendance[i].status ? "Present" : "Absent");
            found = 1;
        }
    }
    if (!found) {
        printf("No attendance records found.\n");
    }
}

void showEmployeeAttendanceMenu(EMSData *data, int32_t employeeId) {
    int32_t choice = 0;
    while (choice != 3) {
        printf("\n=== Attendance Menu ===\n");
        printf("1. Mark attendance\n");
        printf("2. View my attendance\n");
        printf("3. Back\n");
        choice = readInt("Enter choice: ");

        switch (choice) {
            case 1:
                lockData(data);
                markAttendance(data, employeeId);
                unlockData(data);
                break;
            case 2:
                viewMyAttendance(data, employeeId);
                break;
            case 3:
                break;
            default:
                printf("Invalid choice. Try again.\n");
                break;
        }
    }
}

int applyLeave(EMSData *data, int32_t employeeId) {
    if (data->leaveCount >= MAX_LEAVES) {
        printf("Leave storage is full.\n");
        return 0;
    }

    LeaveRequest *leave = &data->leaves[data->leaveCount];
    memset(leave, 0, sizeof(*leave));

    leave->id = nextEmployeeLeaveId(data);
    leave->employeeId = employeeId;
    readValidatedText("Start date (YYYY-MM-DD): ", leave->startDate, sizeof(leave->startDate), isDate, "YYYY-MM-DD");
    readValidatedText("End date (YYYY-MM-DD): ", leave->endDate, sizeof(leave->endDate), isDate, "YYYY-MM-DD");
    readValidatedText("Reason: ", leave->reason, sizeof(leave->reason), isTextWithSpaces, "letters, numbers, and spaces");
    leave->status = 0; /* pending */

    data->leaveCount++;
    markDataDirty(data);
    printf("Leave request submitted and is pending approval.\n");
    return 1;
}

void viewMyLeaves(const EMSData *data, int32_t employeeId) {
    static const char *labels[] = {"Pending", "Approved", "Rejected"};
    int found = 0;

    printf("\n=== My Leave Requests ===\n");
    for (int32_t i = 0; i < data->leaveCount; ++i) {
        if (data->leaves[i].employeeId == employeeId) {
            const LeaveRequest *leave = &data->leaves[i];
            printf("%s to %s | %s | Status: %s\n",
                   leave->startDate,
                   leave->endDate,
                   leave->reason,
                   (leave->status >= 0 && leave->status <= 2) ? labels[leave->status] : "Unknown");
            found = 1;
        }
    }

    if (!found) {
        printf("No leave requests found.\n");
    }
}

void showEmployeeLeaveMenu(EMSData *data, int32_t employeeId) {
    int32_t choice = 0;
    while (choice != 3) {
        printf("\n=== Leave Menu ===\n");
        printf("1. Request leave\n");
        printf("2. View my leave requests\n");
        printf("3. Back\n");
        choice = readInt("Enter choice: ");

        switch (choice) {
            case 1:
                lockData(data);
                applyLeave(data, employeeId);
                unlockData(data);
                break;
            case 2:
                viewMyLeaves(data, employeeId);
                break;
            case 3:
                break;
            default:
                printf("Invalid choice. Try again.\n");
                break;
        }
    }
}

void viewMyPayroll(const EMSData *data, int32_t employeeId) {
    int found = 0;

    printf("\n=== My Payroll ===\n");
    for (int32_t i = 0; i < data->payrollCount; ++i) {
        if (data->payroll[i].employeeId == employeeId) {
            const PayrollRecord *pay = &data->payroll[i];
            printf("%s | Gross: %.2f | Deductions: %.2f | Net: %.2f | Credited: %s\n",
                   pay->month,
                   pay->salary,
                   pay->deductions,
                   pay->netPay,
                   pay->credited ? "Yes" : "No");
            found = 1;
        }
    }

    if (!found) {
        printf("No payroll records found.\n");
    }
}

void viewMyProjects(const EMSData *data, int32_t employeeId) {
    int found = 0;

    printf("\n=== My Project Orientations ===\n");
    for (int32_t i = 0; i < data->projectCount; ++i) {
        if (data->projects[i].employeeId == employeeId) {
            const ProjectOrientation *project = &data->projects[i];
            printf("%s | %s | Completed: %s\n",
                   project->projectName,
                   project->orientationDate,
                   project->completed ? "Yes" : "No");
            found = 1;
        }
    }

    if (!found) {
        printf("No project orientation records found.\n");
    }
}

void viewProjectProgress(const EMSData *data, int32_t employeeId) {
    int total = 0;
    int completed = 0;

    for (int32_t i = 0; i < data->projectCount; ++i) {
        if (data->projects[i].employeeId == employeeId) {
            total++;
            if (data->projects[i].completed) {
                completed++;
            }
        }
    }

    printf("\n=== My Project Progress ===\n");
    if (total == 0) {
        printf("No project records found.\n");
        return;
    }
    printf("Total projects: %d\n", total);
    printf("Completed: %d\n", completed);
    printf("Pending: %d\n", total - completed);
}

void showEmployeeProjectMenu(EMSData *data, int32_t employeeId) {
    int32_t choice = 0;
    while (choice != 3) {
        printf("\n=== Project Orientation Menu ===\n");
        printf("1. View my projects\n");
        printf("2. View project progress\n");
        printf("3. Back\n");
        choice = readInt("Enter choice: ");

        switch (choice) {
            case 1:
                viewMyProjects(data, employeeId);
                break;
            case 2:
                viewProjectProgress(data, employeeId);
                break;
            case 3:
                break;
            default:
                printf("Invalid choice. Try again.\n");
                break;
        }
    }
}
