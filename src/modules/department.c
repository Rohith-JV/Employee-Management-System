#include "ems.h"

static int nextDepartmentId(const EMSData *data) {
    int highest = 0;
    for (int i = 0; i < data->departmentCount; ++i) {
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

    printf("\nDepartments:\n");
    for (int i = 0; i < data->departmentCount; ++i) {
        const Department *dept = &data->departments[i];
        printf("%d. %s | Head: %s\n", dept->id, dept->name, dept->head);
    }
}
