#include "ems.h"

static int findEmployeeIndexById(const EMSData *data, int employeeId) {
    for (int i = 0; i < data->employeeCount; ++i) {
        if (data->employees[i].id == employeeId) {
            return i;
        }
    }
    return -1;
}

static int caseInsensitiveEquals(const char *left, const char *right) {
    while (*left != '\0' && *right != '\0') {
        unsigned char leftChar = (unsigned char)*left;
        unsigned char rightChar = (unsigned char)*right;
        if (tolower(leftChar) != tolower(rightChar)) {
            return 0;
        }
        ++left;
        ++right;
    }
    return *left == *right;
}

static int findEmployeeIndexByName(const EMSData *data, const char *name) {
    for (int i = 0; i < data->employeeCount; ++i) {
        if (caseInsensitiveEquals(data->employees[i].name, name)) {
            return i;
        }
    }
    return -1;
}

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

void findEmployeeByNameOrId(const EMSData *data) {
    char input[64];
    char *end = NULL;
    long idValue = 0;

    if (data->employeeCount == 0) {
        printf("No employees found.\n");
        return;
    }

    readText("Enter employee name or ID: ", input, sizeof(input));

    idValue = strtol(input, &end, 10);
    if (end != input && *end == '\0') {
        int index = findEmployeeIndexById(data, (int)idValue);
        if (index < 0) {
            printf("No employee found with ID %ld.\n", idValue);
            return;
        }

        const Employee *emp = &data->employees[index];
        printf("\nEmployee found:\n");
        printf("ID: %d\nName: %s\nDepartment ID: %d\nRole ID: %d\nSalary: %d\nEmail: %s\nPhone: %s\nJoin Date: %s\nStatus: %s\nActive: %s\n",
               emp->id,
               emp->name,
               emp->departmentId,
               emp->roleId,
               emp->salary,
               emp->email,
               emp->phone,
               emp->joinDate,
               emp->status,
               emp->active ? "Yes" : "No");
        return;
    }

    int index = findEmployeeIndexByName(data, input);
    if (index < 0) {
        printf("No employee found with name '%s'.\n", input);
        return;
    }

    const Employee *emp = &data->employees[index];
    printf("\nEmployee found:\n");
    printf("ID: %d\nName: %s\nDepartment ID: %d\nRole ID: %d\nSalary: %d\nEmail: %s\nPhone: %s\nJoin Date: %s\nStatus: %s\nActive: %s\n",
           emp->id,
           emp->name,
           emp->departmentId,
           emp->roleId,
           emp->salary,
           emp->email,
           emp->phone,
           emp->joinDate,
           emp->status,
           emp->active ? "Yes" : "No");
}
