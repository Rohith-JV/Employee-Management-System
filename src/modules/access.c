#include "ems.h"

static int32_t findAccountIndexByUsername(const EMSData *data, const char *username);

static int32_t findEmployeeIdByName(const EMSData *data, const char *name) {
    for (int32_t i = 0; i < data->employeeCount; ++i) {
        const char *left = data->employees[i].name;
        const char *right = name;
        while (*left != '\0' && *right != '\0' && tolower((unsigned char)*left) == tolower((unsigned char)*right)) {
            ++left;
            ++right;
        }
        if (*left == '\0' && *right == '\0' && data->employees[i].active) {
            return data->employees[i].id;
        }
    }
    return -1;
}

static int32_t nextAccountId(const EMSData *data) {
    int32_t highest = 0;
    for (int32_t i = 0; i < data->accountCount; ++i) {
        if (data->accounts[i].id > highest) {
            highest = data->accounts[i].id;
        }
    }
    return highest + 1;
}

int addAccessAccount(EMSData *data) {
    if (data->accountCount >= MAX_ACCOUNTS) {
        printf("Access account storage is full.\n");
        return 0;
    }

    AccessAccount *account = &data->accounts[data->accountCount];
    memset(account, 0, sizeof(*account));

    account->id = nextAccountId(data);
    readValidatedText("Username: ", account->username, sizeof(account->username), isUsername, "3-20 letters, numbers, dot, underscore, or hyphen");
    if (findAccountIndexByUsername(data, account->username) >= 0) {
        printf("Username already exists.\n");
        return 0;
    }
    readValidatedText("Password: ", account->password, sizeof(account->password), isPassword, "4-32 characters");
    account->roleId = readValidatedInt("Role ID: ", 1, 100000, "positive integer");
    account->employeeId = readValidatedInt("Employee ID (0 for none): ", 0, 100000, "0 or a positive integer");
    if (!roleExists(data, account->roleId) || (account->employeeId != 0 && !employeeExists(data, account->employeeId))) {
        printf("Role ID or employee ID does not exist.\n");
        return 0;
    }
    account->active = (uint8_t)readValidatedInt("Active (1/0): ", 0, 1, "0 or 1");
    account->passwordChangeRequired = 0;

    data->accountCount++;
    printf("Access account created.\n");
    return 1;
}

static int32_t findAccountIndex(const EMSData *data, const char *username, const char *password) {
    for (int32_t i = 0; i < data->accountCount; ++i) {
        if (data->accounts[i].active && strcmp(data->accounts[i].username, username) == 0 && strcmp(data->accounts[i].password, password) == 0) {
            return i;
        }
    }
    return -1;
}

static int32_t findAccountIndexByUsername(const EMSData *data, const char *username) {
    for (int32_t i = 0; i < data->accountCount; ++i) {
        if (strcmp(data->accounts[i].username, username) == 0) {
            return i;
        }
    }
    return -1;
}

int createEmployeeAccessAccount(EMSData *data, int32_t employeeId, const char *username, const char *password) {
    if (data->accountCount >= MAX_ACCOUNTS) {
        printf("Access account storage is full.\n");
        return 0;
    }

    if (findAccountIndexByUsername(data, username) >= 0) {
        printf("Username already exists. Please try another one.\n");
        return 0;
    }

    if (!employeeExists(data, employeeId) || !roleExists(data, EMPLOYEE_ROLE_ID)) {
        printf("Employee or employee role does not exist.\n");
        return 0;
    }

    AccessAccount *account = &data->accounts[data->accountCount++];
    memset(account, 0, sizeof(*account));

    account->id = nextAccountId(data);
    copyStringSafe(account->username, sizeof(account->username), username);
    copyStringSafe(account->password, sizeof(account->password), password);
    account->roleId = EMPLOYEE_ROLE_ID;
    account->employeeId = employeeId;
    account->active = 1;
    account->passwordChangeRequired = 1;
    account->pendingApproval = 0;

    markDataDirty(data);
    printf("Temporary employee access account created.\n");
    return 1;
}

static const char *getRoleTitle(const EMSData *data, int32_t roleId) {
    for (int32_t i = 0; i < data->roleCount; ++i) {
        if (data->roles[i].id == roleId) {
            return data->roles[i].title;
        }
    }
    return NULL;
}

static int isHRRole(const EMSData *data, int32_t roleId) {
    const char *title = getRoleTitle(data, roleId);
    if (!title) return 0;
    return (strcmp(title, "HR") == 0 || strcmp(title, "hr") == 0);
}

int employeeLogin(EMSData *data, int32_t *employeeId) {
    char username[32];
    char password[32];
    int32_t loginChoice;
    int32_t idx;

    printf("\n=== Employee Access ===\n");
    printf("1. Sign in\n");
    printf("2. Sign up (first login with HR temporary password)\n");
    loginChoice = readValidatedInt("Enter choice: ", 1, 2, "1 or 2");

    if (loginChoice == 1) {
        readText("Username: ", username, sizeof(username));
        readText("Password: ", password, sizeof(password));
        idx = findAccountIndex(data, username, password);
    } else {
        int32_t employeeIdByName;
        readValidatedText("Name: ", username, sizeof(username), isAlphaText, "letters and spaces, e.g. Ram Kumar");
        readText("Temporary password issued by HR: ", password, sizeof(password));
        employeeIdByName = findEmployeeIdByName(data, username);
        idx = -1;
        if (employeeIdByName > 0) {
            for (int32_t i = 0; i < data->accountCount; ++i) {
                if (data->accounts[i].employeeId == employeeIdByName && data->accounts[i].active &&
                    strcmp(data->accounts[i].password, password) == 0) {
                    idx = i;
                    break;
                }
            }
        }
    }
    if (idx < 0) {
        printf("Invalid credentials, name, or inactive account.\n");
        return 0;
    }

    AccessAccount *acc = &data->accounts[idx];
    if (isHRRole(data, acc->roleId)) {
        printf("This account is an HR account. Please use HR login.\n");
        return 0;
    }

    if (acc->employeeId <= 0) {
        printf("No employee linked to this account. Contact administrator.\n");
        return 0;
    }

    if (!employeeExists(data, acc->employeeId)) {
        printf("The linked employee is inactive or no longer exists.\n");
        return 0;
    }

    if (acc->passwordChangeRequired) {
        char newPassword[MAX_PASSWORD_LENGTH];
        char confirmPassword[MAX_PASSWORD_LENGTH];

        printf("Password change required. Please set a new password.\n");
        readValidatedText("New password: ", newPassword, sizeof(newPassword), isPassword, "4-32 characters");
        readValidatedText("Confirm new password: ", confirmPassword, sizeof(confirmPassword), isPassword, "4-32 characters");

        if (strcmp(newPassword, confirmPassword) != 0) {
            printf("Passwords do not match.\n");
            return 0;
        }

        if (strcmp(newPassword, password) == 0) {
            printf("New password must differ from the temporary password.\n");
            return 0;
        }

        copyStringSafe(acc->password, sizeof(acc->password), newPassword);
        acc->passwordChangeRequired = 0;
        markDataDirty(data);
    }

    printf("Employee login successful.\n");
    if (employeeId != NULL) *employeeId = acc->employeeId;
    return 1;
}

void showAccessManagementMenu(EMSData *data) {
    int32_t choice = 0;

    while (choice != 3) {
        printf("\n=== Login & Access Management ===\n1. Add account\n2. List accounts\n3. Back\n");
        choice = readInt("Enter choice: ");
        if (choice == EMS_INPUT_EOF) {
            return;
        }
        switch (choice) {
            case 1: addAccessAccount(data); markDataDirty(data); break;
            case 2: listAccessAccounts(data); break;
            case 3: break;
            default: printf("Invalid choice. Try again.\n"); break;
        }
    }
}

int hrLogin(EMSData *data) {
    char username[32];
    char password[32];
    readText("Username: ", username, sizeof(username));
    readText("Password: ", password, sizeof(password));

    int idx = findAccountIndex(data, username, password);
    if (idx < 0) {
        printf("Invalid credentials or inactive account.\n");
        return 0;
    }

    AccessAccount *acc = &data->accounts[idx];
    if (!isHRRole(data, acc->roleId)) {
        printf("This account is not an HR account.\n");
        return 0;
    }

    printf("HR login successful.\n");
    return 1;
}

void listAccessAccounts(const EMSData *data) {
    if (data->accountCount == 0) {
        printf("No access accounts found.\n");
        return;
    }

    printf("\nAccess accounts:\n");
    for (int32_t i = 0; i < data->accountCount; ++i) {
        const AccessAccount *account = &data->accounts[i];
        printf("%d. %s | Role ID: %d | Active: %s | Status: %s\n",
               account->id,
               account->username,
               account->roleId,
               account->active ? "Yes" : "No",
               account->pendingApproval ? "Pending HR approval" : "Approved");
    }
}
