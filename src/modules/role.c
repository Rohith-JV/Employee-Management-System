#include "ems.h"

static int nextRoleId(const EMSData *data) {
    int highest = 0;
    for (int i = 0; i < data->roleCount; ++i) {
        if (data->roles[i].id > highest) {
            highest = data->roles[i].id;
        }
    }
    return highest + 1;
}

int addRole(EMSData *data) {
    if (data->roleCount >= MAX_ROLES) {
        printf("Role storage is full.\n");
        return 0;
    }

    Role *role = &data->roles[data->roleCount];
    memset(role, 0, sizeof(*role));

    role->id = nextRoleId(data);
    readValidatedText("Role title: ", role->title, sizeof(role->title), isAlphaText, "letters and spaces, e.g. Software Engineer");
    readValidatedText("Role description: ", role->description, sizeof(role->description), isTextWithSpaces, "letters, numbers, and spaces");

    data->roleCount++;
    printf("Role added successfully.\n");
    return 1;
}

void listRoles(const EMSData *data) {
    if (data->roleCount == 0) {
        printf("No roles found.\n");
        return;
    }

    printf("\nRoles:\n");
    for (int i = 0; i < data->roleCount; ++i) {
        const Role *role = &data->roles[i];
        printf("%d. %s | %s\n", role->id, role->title, role->description);
    }
}
