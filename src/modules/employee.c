#include "ems.h"

static int32_t findEmployeeIndexById(const EMSData *data, int32_t employeeId) {
    for (int32_t i = 0; i < data->employeeCount; ++i) {
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
    for (int32_t i = 0; i < data->employeeCount; ++i) {
        if (caseInsensitiveEquals(data->employees[i].name, name)) {
            return i;
        }
    }
    return -1;
}

static int32_t nextEmployeeId(const EMSData *data) {
    int32_t highest = 0;
    for (int32_t i = 0; i < data->employeeCount; ++i) {
        if (data->employees[i].id > highest) {
            highest = data->employees[i].id;
        }
    }
    return highest + 1;
}

static void printEmployeeSummary(const Employee *emp) {
    printf("ID: %d | Name: %s | Dept: %d | Role: %d | Salary: %d | Email: %s | Status: %s | Active: %s\n",
           emp->id,
           emp->name,
           emp->departmentId,
           emp->roleId,
           emp->salary,
           emp->email,
           emp->status,
           emp->active ? "Yes" : "No");
}

static void printEmployeeDetails(const Employee *emp) {
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

static void removeEmployeeAtIndex(EMSData *data, int32_t index) {
    if (index < 0 || index >= data->employeeCount) {
        return;
    }

    for (int32_t i = index; i < data->employeeCount - 1; ++i) {
        data->employees[i] = data->employees[i + 1];
    }

    memset(&data->employees[data->employeeCount - 1], 0, sizeof(data->employees[data->employeeCount - 1]));
    data->employeeCount--;
}

static void deactivateAccountsForEmployee(EMSData *data, int32_t employeeId) {
    for (int32_t i = 0; i < data->accountCount; ++i) {
        if (data->accounts[i].employeeId == employeeId) {
            data->accounts[i].active = 0;
            data->accounts[i].employeeId = 0;
        }
    }
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
    if (!departmentExists(data, emp->departmentId) || !roleExists(data, emp->roleId)) {
        printf("Department ID or role ID does not exist.\n");
        return 0;
    }
    emp->salary = readValidatedInt("Salary: ", 1, 100000000, "positive integer");
    readValidatedText("Email: ", emp->email, sizeof(emp->email), isEmail, "name@domain.com");
    readValidatedText("Phone: ", emp->phone, sizeof(emp->phone), isPhone, "+1234567890");
    readValidatedText("Join date (YYYY-MM-DD): ", emp->joinDate, sizeof(emp->joinDate), isDate, "YYYY-MM-DD");
    readValidatedText("Status (Active/Probation/Inactive): ", emp->status, sizeof(emp->status), isStatusValue, "Active, Probation, or Inactive");
    emp->active = 1;

    data->employeeCount++;

    {
        char username[MAX_USERNAME_LENGTH];
        char tempPassword[MAX_PASSWORD_LENGTH];
        int32_t accountCreated = 0;

        printf("Create employee login credentials.\n");
        while (!accountCreated) {
            readValidatedText("Username: ", username, sizeof(username), isUsername, "3-20 letters, numbers, dot, underscore, or hyphen");
            readValidatedText("Temporary password: ", tempPassword, sizeof(tempPassword), isPassword, "4-32 characters");

            if (createEmployeeAccessAccount(data, emp->id, username, tempPassword)) {
                accountCreated = 1;
                printf("Temporary password issued. The employee must change it at first login.\n");
            } else {
                printf("Please try again with a different username.\n");
            }
        }
    }

    printf("Employee onboarding completed successfully.\n");
    return 1;
}

void listEmployees(const EMSData *data) {
    if (data->employeeCount == 0) {
        printf("No employees found.\n");
        return;
    }

    printf("\nEmployees:\n");
    for (int32_t i = 0; i < data->employeeCount; ++i) {
        printEmployeeSummary(&data->employees[i]);
    }
}

void searchEmployees(const EMSData *data) {
    int32_t choice = 0;
    char name[MAX_NAME_LENGTH];
    int32_t idValue = 0;
    int32_t departmentId = 0;
    int32_t roleId = 0;
    char statusValue[MAX_STATUS_LENGTH];
    int32_t found = 0;

    if (data->employeeCount == 0) {
        printf("No employees found.\n");
        return;
    }

    printf("\nSearch employees by:\n");
    printf("1. Employee ID\n");
    printf("2. Name\n");
    printf("3. Department ID\n");
    printf("4. Role ID\n");
    printf("5. Status\n");
    printf("6. Back\n");
    choice = readInt("Enter choice: ");

    switch (choice) {
        case 1:
            idValue = readValidatedInt("Employee ID: ", 1, 100000, "positive integer");
            for (int32_t i = 0; i < data->employeeCount; ++i) {
                if (data->employees[i].id == idValue) {
                    printEmployeeDetails(&data->employees[i]);
                    found = 1;
                }
            }
            break;
        case 2:
            readText("Employee name: ", name, sizeof(name));
            for (int32_t i = 0; i < data->employeeCount; ++i) {
                if (caseInsensitiveEquals(data->employees[i].name, name)) {
                    printEmployeeDetails(&data->employees[i]);
                    found = 1;
                }
            }
            break;
        case 3:
            departmentId = readValidatedInt("Department ID: ", 1, 100000, "positive integer");
            for (int32_t i = 0; i < data->employeeCount; ++i) {
                if (data->employees[i].departmentId == departmentId) {
                    printEmployeeSummary(&data->employees[i]);
                    found = 1;
                }
            }
            break;
        case 4:
            roleId = readValidatedInt("Role ID: ", 1, 100000, "positive integer");
            for (int32_t i = 0; i < data->employeeCount; ++i) {
                if (data->employees[i].roleId == roleId) {
                    printEmployeeSummary(&data->employees[i]);
                    found = 1;
                }
            }
            break;
        case 5:
            readValidatedText("Status: ", statusValue, sizeof(statusValue), isStatusValue, "Active, Probation, or Inactive");
            for (int32_t i = 0; i < data->employeeCount; ++i) {
                if (strcmp(data->employees[i].status, statusValue) == 0) {
                    printEmployeeSummary(&data->employees[i]);
                    found = 1;
                }
            }
            break;
        default:
            printf("Invalid choice.\n");
            return;
    }

    if (!found) {
        printf("No matching employees found.\n");
    }
}

void updateEmployee(EMSData *data) {
    int32_t employeeId = 0;
    int32_t index = -1;
    int32_t choice = 0;

    if (data->employeeCount == 0) {
        printf("No employees found.\n");
        return;
    }

    employeeId = readValidatedInt("Employee ID to update: ", 1, 100000, "positive integer");
    index = findEmployeeIndexById(data, employeeId);
    if (index < 0) {
        printf("No employee found with ID %d.\n", employeeId);
        return;
    }

    Employee *emp = &data->employees[index];
    printEmployeeDetails(emp);

    while (choice != 9) {
        printf("\nUpdate employee fields:\n");
        printf("1. Name\n");
        printf("2. Department ID\n");
        printf("3. Role ID\n");
        printf("4. Salary\n");
        printf("5. Email\n");
        printf("6. Phone\n");
        printf("7. Status\n");
        printf("8. Active flag\n");
        printf("9. Done\n");
        choice = readInt("Enter choice: ");

        switch (choice) {
            case 1:
                readValidatedText("New name: ", emp->name, sizeof(emp->name), isAlphaText, "letters and spaces, e.g. John Doe");
                break;
            case 2:
                {
                    int32_t value = readValidatedInt("New department ID: ", 1, 100000, "positive integer");
                    if (departmentExists(data, value)) emp->departmentId = value;
                    else printf("Department ID does not exist.\n");
                }
                break;
            case 3:
                {
                    int32_t value = readValidatedInt("New role ID: ", 1, 100000, "positive integer");
                    if (roleExists(data, value)) emp->roleId = value;
                    else printf("Role ID does not exist.\n");
                }
                break;
            case 4:
                emp->salary = readValidatedInt("New salary: ", 1, 100000000, "positive integer");
                break;
            case 5:
                readValidatedText("New email: ", emp->email, sizeof(emp->email), isEmail, "name@domain.com");
                break;
            case 6:
                readValidatedText("New phone: ", emp->phone, sizeof(emp->phone), isPhone, "+1234567890");
                break;
            case 7:
                readValidatedText("New status: ", emp->status, sizeof(emp->status), isStatusValue, "Active, Probation, or Inactive");
                break;
            case 8:
                emp->active = (uint8_t)readValidatedInt("Active (1/0): ", 0, 1, "0 or 1");
                break;
            case 9:
                break;
            default:
                printf("Invalid choice.\n");
                break;
        }
    }

    printf("Employee updated successfully.\n");
}

void deleteEmployee(EMSData *data) {
    int32_t employeeId = 0;
    int32_t index = -1;
    char confirm[8];

    if (data->employeeCount == 0) {
        printf("No employees found.\n");
        return;
    }

    employeeId = readValidatedInt("Employee ID to delete: ", 1, 100000, "positive integer");
    index = findEmployeeIndexById(data, employeeId);
    if (index < 0) {
        printf("No employee found with ID %d.\n", employeeId);
        return;
    }

    printEmployeeDetails(&data->employees[index]);
    readText("Confirm delete (y/n): ", confirm, sizeof(confirm));
    if (confirm[0] != 'y' && confirm[0] != 'Y') {
        printf("Deletion cancelled.\n");
        return;
    }

    deactivateAccountsForEmployee(data, employeeId);
    removeEmployeeAtIndex(data, index);
    markDataDirty(data);
    printf("Employee deleted successfully.\n");
}

void showEmployeeRecordsMenu(EMSData *data) {
    int32_t choice = 0;

    while (choice != 6) {
        printf("\n=== Employee Records ===\n");
        printf("1. Add employee\n");
        printf("2. List employees\n");
        printf("3. Search employees\n");
        printf("4. Update employee\n");
        printf("5. Delete employee\n");
        printf("6. Back\n");
        choice = readInt("Enter choice: ");
        if (choice == EMS_INPUT_EOF) {
            return;
        }

        switch (choice) {
            case 1:
                addEmployee(data);
                break;
            case 2:
                listEmployees(data);
                break;
            case 3:
                searchEmployees(data);
                break;
            case 4:
                updateEmployee(data);
                break;
            case 5:
                deleteEmployee(data);
                break;
            case 6:
                break;
            default:
                printf("Invalid choice.\n");
                break;
        }
    }
}

void findEmployeeByNameOrId(const EMSData *data) {
    char input[64];
    char *end = NULL;
    int32_t idValue = 0;

    if (data->employeeCount == 0) {
        printf("No employees found.\n");
        return;
    }

    readText("Enter employee name or ID: ", input, sizeof(input));

    idValue = (int32_t)strtol(input, &end, 10);
    if (end != input && *end == '\0') {
        int32_t index = findEmployeeIndexById(data, idValue);
        if (index < 0) {
            printf("No employee found with ID %d.\n", idValue);
            return;
        }

        const Employee *emp = &data->employees[index];
        printEmployeeDetails(emp);
        return;
    }

    int32_t index = findEmployeeIndexByName(data, input);
    if (index < 0) {
        printf("No employee found with name '%s'.\n", input);
        return;
    }

    const Employee *emp = &data->employees[index];
    printEmployeeDetails(emp);
}
