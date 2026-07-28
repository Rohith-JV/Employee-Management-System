#include "ems.h"

static int nextAccountId(const EMSData *data) {
    int highest = 0;
    for (int i = 0; i < data->accountCount; ++i) {
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
    readValidatedText("Password: ", account->password, sizeof(account->password), isPassword, "4-32 characters");
    account->roleId = readValidatedInt("Role ID: ", 1, 100000, "positive integer");
    account->employeeId = readValidatedInt("Employee ID (0 for none): ", 0, 100000, "0 or a positive integer");
    account->active = readValidatedInt("Active (1/0): ", 0, 1, "0 or 1");

    data->accountCount++;
    printf("Access account created.\n");
    return 1;
}

static int findAccountIndex(const EMSData *data, const char *username, const char *password) {
    for (int i = 0; i < data->accountCount; ++i) {
        if (data->accounts[i].active && strcmp(data->accounts[i].username, username) == 0 && strcmp(data->accounts[i].password, password) == 0) {
            return i;
        }
    }
    return -1;
}

static int findAccountIndexByUsername(const EMSData *data, const char *username) {
    for (int i = 0; i < data->accountCount; ++i) {
        if (strcmp(data->accounts[i].username, username) == 0) {
            return i;
        }
    }
    return -1;
}

static int createAccountForRole(EMSData *data, int roleId, const char *username, const char *password) {
    if (data->accountCount >= MAX_ACCOUNTS) {
        printf("Access account storage is full.\n");
        return 0;
    }

    if (findAccountIndexByUsername(data, username) >= 0) {
        printf("Username already exists. Please try another one.\n");
        return 0;
    }

    AccessAccount *account = &data->accounts[data->accountCount++];
    memset(account, 0, sizeof(*account));

    account->id = nextAccountId(data);
    strncpy(account->username, username, sizeof(account->username) - 1);
    account->username[sizeof(account->username) - 1] = '\0';
    strncpy(account->password, password, sizeof(account->password) - 1);
    account->password[sizeof(account->password) - 1] = '\0';
    account->roleId = roleId;
    account->employeeId = 0;
    account->active = 1;

    saveAll(data);
    printf("Account created successfully.\n");
    return 1;
}

static const char *getRoleTitle(const EMSData *data, int roleId) {
    for (int i = 0; i < data->roleCount; ++i) {
        if (data->roles[i].id == roleId) {
            return data->roles[i].title;
        }
    }
    return NULL;
}

static int isHRRole(const EMSData *data, int roleId) {
    const char *title = getRoleTitle(data, roleId);
    if (!title) return 0;
    return (strcmp(title, "HR") == 0 || strcmp(title, "hr") == 0);
}

int employeeLogin(EMSData *data) {
    char username[32];
    char password[32];
    char choice[8];

    readText("New user? (y/n): ", choice, sizeof(choice));
    if (choice[0] == 'y' || choice[0] == 'Y') {
        readValidatedText("Choose username: ", username, sizeof(username), isUsername, "3-20 letters, numbers, dot, underscore, or hyphen");
        readValidatedText("Choose password: ", password, sizeof(password), isPassword, "4-32 characters");
        if (!createAccountForRole(data, 2, username, password)) {
            return 0;
        }
        printf("Employee account created. Logging in...\n");
        return 1;
    }

    readText("Username: ", username, sizeof(username));
    readText("Password: ", password, sizeof(password));

    int idx = findAccountIndex(data, username, password);
    if (idx < 0) {
        printf("Invalid credentials or inactive account.\n");
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

    printf("Employee login successful.\n");
    return 1;
}

int hrLogin(EMSData *data) {
    char username[32];
    char password[32];
    char choice[8];

    readText("New user? (y/n): ", choice, sizeof(choice));
    if (choice[0] == 'y' || choice[0] == 'Y') {
        readValidatedText("Choose username: ", username, sizeof(username), isUsername, "3-20 letters, numbers, dot, underscore, or hyphen");
        readValidatedText("Choose password: ", password, sizeof(password), isPassword, "4-32 characters");
        if (!createAccountForRole(data, 1, username, password)) {
            return 0;
        }
        printf("HR account created. Logging in...\n");
        return 1;
    }

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
    for (int i = 0; i < data->accountCount; ++i) {
        const AccessAccount *account = &data->accounts[i];
        printf("%d. %s | Role ID: %d | Active: %s\n",
               account->id,
               account->username,
               account->roleId,
               account->active ? "Yes" : "No");
    }
}
