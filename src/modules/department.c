#include "ems.h"

static int32_t nextDepartmentId(const EMSData *data) {
    int32_t highest = 0;
    for (int32_t i = 0; i < data->departmentCount; ++i) {
        if (data->departments[i].id > highest) {
            highest = data->departments[i].id;
        }
    }
    return highest + 1;
}

int addDepartment(EMSData *data) {
    if (data->departmentCount >= MAX_DEPARTMENTS) {
        printf("Department storage is full.\n");
        return 0;
    }

    Department *dept = &data->departments[data->departmentCount];
    memset(dept, 0, sizeof(*dept));

    dept->id = nextDepartmentId(data);
    readValidatedText("Department name: ", dept->name, sizeof(dept->name), isAlphaText, "letters and spaces, e.g. Engineering");
    readValidatedText("Department head: ", dept->head, sizeof(dept->head), isAlphaText, "letters and spaces, e.g. Jane Smith");

    data->departmentCount++;
    printf("Department added successfully.\n");
    return 1;
}

void listDepartments(const EMSData *data) {
    if (data->departmentCount == 0) {
        printf("No departments found.\n");
        return;
    }

    int32_t choice = 0;
    while (choice != 3) {
        printf("\nDepartments:\n");
        for (int32_t i = 0; i < data->departmentCount; ++i) {
            const Department *dept = &data->departments[i];
            printf("%d. %s | Head: %s\n", dept->id, dept->name, dept->head);
        }
        printf("\n1. Show employees in a department\n");
        printf("2. Refresh list\n");
        printf("3. Back\n");
        choice = readInt("Enter choice: ");
        if (choice == EMS_INPUT_EOF) {
            return;
        }
        if (choice == 1) {
            int32_t deptId = readValidatedInt("Department ID: ", 1, 100000, "positive integer");
            int found = 0;
            for (int32_t i = 0; i < data->departmentCount; ++i) {
                if (data->departments[i].id == deptId) {
                    printf("\nEmployees in %s:\n", data->departments[i].name);
                    for (int32_t j = 0; j < data->employeeCount; ++j) {
                        const Employee *e = &data->employees[j];
                        if (e->departmentId == deptId && e->id > 0) {
                            printf("ID: %d | Name: %s | Role: %d | Active: %s\n",
                                   e->id, e->name, e->roleId, e->active ? "Yes" : "No");
                            found = 1;
                        }
                    }
                    if (!found) printf("No employees found in this department.\n");
                    break;
                }
            }
            if (!found) {
                /* If department id wasn't matched, inform user. */
                printf("Department ID %d not found.\n", deptId);
            }
        } else if (choice == 2) {
            continue; /* loop will redisplay */
        } else if (choice == 3) {
            break;
        } else {
            printf("Invalid choice. Try again.\n");
        }
    }
}
