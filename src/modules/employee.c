#include "ems.h"

static int nextEmployeeId(const EMSData *data) {
    int highest = 0;
    for (int i = 0; i < data->employeeCount; ++i) {
        if (data->employees[i].id > highest) {
            highest = data->employees[i].id;
        }
    }
    return highest + 1;
}

int addEmployee(EMSData *data) {
    if (data->employeeCount >= MAX_EMPLOYEES) {
        printf("Employee storage is full.\n");
        return 0;
    }

    Employee *emp = &data->employees[data->employeeCount];
    memset(emp, 0, sizeof(*emp));

    emp->id = nextEmployeeId(data);
    readValidatedText("Name: ", emp->name, sizeof(emp->name), isAlphaText, "letters and spaces, e.g. John Doe");
    emp->departmentId = readValidatedInt("Department ID: ", 1, 100000, "positive integer");
    emp->roleId = readValidatedInt("Role ID: ", 1, 100000, "positive integer");
    emp->salary = readValidatedInt("Salary: ", 1, 100000000, "positive integer");
    readValidatedText("Email: ", emp->email, sizeof(emp->email), isEmail, "name@domain.com");
    readValidatedText("Phone: ", emp->phone, sizeof(emp->phone), isPhone, "+1234567890");
    readValidatedText("Join date (YYYY-MM-DD): ", emp->joinDate, sizeof(emp->joinDate), isDate, "YYYY-MM-DD");
    readValidatedText("Status (Active/Probation/Inactive): ", emp->status, sizeof(emp->status), isStatusValue, "Active, Probation, or Inactive");
    emp->active = 1;

    data->employeeCount++;
    printf("Employee onboarding completed successfully.\n");
    return 1;
}

void listEmployees(const EMSData *data) {
    if (data->employeeCount == 0) {
        printf("No employees found.\n");
        return;
    }

    printf("\nEmployees:\n");
    for (int i = 0; i < data->employeeCount; ++i) {
        const Employee *emp = &data->employees[i];
        printf("%d. %s | Dept: %d | Role: %d | Salary: %d | Email: %s | Status: %s | Active: %s\n",
               emp->id,
               emp->name,
               emp->departmentId,
               emp->roleId,
               emp->salary,
               emp->email,
               emp->status,
               emp->active ? "Yes" : "No");
    }
}
