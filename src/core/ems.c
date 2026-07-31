#include "ems.h"

static void saveAllUnlocked(EMSData *data);

void lockData(EMSData *data) {
    ems_mutex_lock(&data->mutex);
}

void unlockData(EMSData *data) {
    ems_mutex_unlock(&data->mutex);
}

void markDataDirty(EMSData *data) {
    data->dirty = 1;
    ems_condition_signal(&data->saveRequested);
}

static EMS_THREAD_RETURN EMS_THREAD_CALL autosaveWorker(void *argument) {
    EMSData *data = argument;

    lockData(data);
    while (data->autosaveRunning) {
        while (data->autosaveRunning && !data->dirty) {
            ems_condition_wait(&data->saveRequested, &data->mutex);
        }
        if (data->autosaveRunning && data->dirty) {
            data->dirty = 0;
            saveAllUnlocked(data);
        }
    }
    unlockData(data);
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

int startAutosave(EMSData *data) {
    int result;
    lockData(data);
    data->autosaveRunning = 1;
    unlockData(data);
    result = ems_thread_create(&data->autosaveThread, autosaveWorker, data);
    if (result != 0) {
        lockData(data);
        data->autosaveRunning = 0;
        unlockData(data);
    }
    return result == 0;
}

void stopAutosave(EMSData *data) {
    if (!data->autosaveRunning) {
        return;
    }
    lockData(data);
    data->autosaveRunning = 0;
    ems_condition_signal(&data->saveRequested);
    unlockData(data);
    ems_thread_join(data->autosaveThread);
}

static void trimNewline(char *text) {
    size_t len = strlen(text);
    if (len > 0 && text[len - 1] == '\n') {
        text[len - 1] = '\0';
    }
}

int copyStringSafe(char *dest, size_t destSize, const char *src) {
    size_t srcLength;

    if (dest == NULL || destSize == 0) {
        return 0;
    }

    if (src == NULL) {
        dest[0] = '\0';
        return 0;
    }

    srcLength = strlen(src);
    if (srcLength >= destSize) {
        memcpy(dest, src, destSize - 1);
        dest[destSize - 1] = '\0';
        return 0;
    }

    memcpy(dest, src, srcLength + 1);
    return 1;
}

static int parseIntValue(const char *buffer, int32_t *value) {
    char *end = NULL;
    long parsedValue;

    if (buffer == NULL || value == NULL) {
        return 0;
    }

    errno = 0;
    parsedValue = strtol(buffer, &end, 10);
    if (errno != 0 || end == buffer || *end != '\0') {
        return 0;
    }

    if (parsedValue < INT_MIN || parsedValue > INT_MAX) {
        return 0;
    }

    *value = (int)parsedValue;
    return 1;
}

static int parseDoubleValue(const char *buffer, double *value) {
    char *end = NULL;
    double parsedValue;

    if (buffer == NULL || value == NULL) {
        return 0;
    }

    errno = 0;
    parsedValue = strtod(buffer, &end);
    if (errno != 0 || end == buffer || *end != '\0') {
        return 0;
    }

    *value = parsedValue;
    return 1;
}

void readText(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);
    if (!fgets(buffer, (int)size, stdin)) {
        buffer[0] = '\0';
        return;
    }
    trimNewline(buffer);
}

int isAlphaText(const char *input) {
    int sawLetter = 0;
    for (int i = 0; input[i] != '\0'; ++i) {
        if (isalpha((unsigned char)input[i]) || input[i] == ' ' || input[i] == '.' || input[i] == '-' || input[i] == '\'') {
            if (isalpha((unsigned char)input[i])) {
                sawLetter = 1;
            }
        } else {
            return 0;
        }
    }
    return sawLetter;
}

int isEmail(const char *input) {
    int atCount = 0;
    int dotCount = 0;
    int seenAt = 0;
    int seenDotAfterAt = 0;

    for (int i = 0; input[i] != '\0'; ++i) {
        if (input[i] == '@') {
            atCount++;
            seenAt = 1;
            if (i == 0) {
                return 0;
            }
        } else if (input[i] == '.') {
            dotCount++;
            if (seenAt) {
                seenDotAfterAt = 1;
            }
        } else if (!isalnum((unsigned char)input[i]) && input[i] != '_' && input[i] != '-' && input[i] != '.') {
            return 0;
        }
    }

    return atCount == 1 && dotCount >= 1 && seenAt && seenDotAfterAt;
}

int isPhone(const char *input) {
    int digits = 0;
    for (int i = 0; input[i] != '\0'; ++i) {
        if (input[i] == '+') {
            if (i != 0) {
                return 0;
            }
            continue;
        }
        if (!isdigit((unsigned char)input[i])) {
            return 0;
        }
        digits++;
    }
    return digits >= 10 && digits <= 15;
}

int isDate(const char *input) {
    if (strlen(input) != 10) {
        return 0;
    }
    return isdigit((unsigned char)input[0]) && isdigit((unsigned char)input[1]) && isdigit((unsigned char)input[2]) && isdigit((unsigned char)input[3]) &&
           input[4] == '-' && isdigit((unsigned char)input[5]) && isdigit((unsigned char)input[6]) &&
           input[7] == '-' && isdigit((unsigned char)input[8]) && isdigit((unsigned char)input[9]);
}

int isMonth(const char *input) {
    if (strlen(input) != 7) {
        return 0;
    }
    return isdigit((unsigned char)input[0]) && isdigit((unsigned char)input[1]) && isdigit((unsigned char)input[2]) && isdigit((unsigned char)input[3]) &&
           input[4] == '-' && isdigit((unsigned char)input[5]) && isdigit((unsigned char)input[6]);
}

int isStatusValue(const char *input) {
    return strcmp(input, "Active") == 0 || strcmp(input, "Probation") == 0 || strcmp(input, "Inactive") == 0;
}

int isTextWithSpaces(const char *input) {
    if (input[0] == '\0') {
        return 0;
    }
    for (int i = 0; input[i] != '\0'; ++i) {
        if (!isalnum((unsigned char)input[i]) && input[i] != ' ' && input[i] != '.' && input[i] != ',' && input[i] != '\'' && input[i] != '-') {
            return 0;
        }
    }
    return 1;
}

int isUsername(const char *input) {
    int len = (int)strlen(input);
    if (len < 3 || len > 20) {
        return 0;
    }
    for (int i = 0; input[i] != '\0'; ++i) {
        if (!isalnum((unsigned char)input[i]) && input[i] != '.' && input[i] != '_' && input[i] != '-') {
            return 0;
        }
    }
    return 1;
}

int isPassword(const char *input) {
    int len = (int)strlen(input);
    return len >= 4 && len <= 32;
}

void readValidatedText(const char *prompt, char *buffer, size_t size, int (*validator)(const char *), const char *formatHint) {
    while (1) {
        readText(prompt, buffer, size);
        if (buffer[0] == '\0') {
            printf("Warning: input cannot be empty.\n");
            continue;
        }

        if (validator(buffer)) {
            return;
        }

        printf("Warning: invalid input. Expected format: %s\n", formatHint);
    }
}

int32_t readInt(const char *prompt) {
    char buffer[MAX_INPUT_BUFFER_LENGTH];
    int32_t value = 0;

    printf("%s", prompt);
    if (!fgets(buffer, sizeof(buffer), stdin)) {
        return 0;
    }
    trimNewline(buffer);

    if (!parseIntValue(buffer, &value)) {
        return 0;
    }

    return value;
}

int32_t readValidatedInt(const char *prompt, int32_t minValue, int32_t maxValue, const char *formatHint) {
    char buffer[MAX_INPUT_BUFFER_LENGTH];
    int32_t value = 0;

    while (1) {
        printf("%s", prompt);
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            return 0;
        }
        trimNewline(buffer);

        if (parseIntValue(buffer, &value) && value >= minValue && value <= maxValue) {
            return value;
        }

        printf("Warning: invalid input. Expected format: %s\n", formatHint);
    }
}

double readDouble(const char *prompt) {
    char buffer[MAX_INPUT_BUFFER_LENGTH];
    double value = 0.0;

    printf("%s", prompt);
    if (!fgets(buffer, sizeof(buffer), stdin)) {
        return 0.0;
    }
    trimNewline(buffer);

    if (!parseDoubleValue(buffer, &value)) {
        return 0.0;
    }

    return value;
}

double readValidatedDouble(const char *prompt, double minValue, double maxValue, const char *formatHint) {
    char buffer[MAX_INPUT_BUFFER_LENGTH];
    double value = 0.0;

    while (1) {
        printf("%s", prompt);
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            return 0.0;
        }
        trimNewline(buffer);

        if (parseDoubleValue(buffer, &value) && value >= minValue && value <= maxValue) {
            return value;
        }

        printf("Warning: invalid input. Expected format: %s\n", formatHint);
    }
}

void showModuleMenu(EMSData *data, const char *title, int (*addFunc)(EMSData *), void (*listFunc)(const EMSData *)) {
    int32_t subChoice = 0;
    while (subChoice != 3) {
        printf("\n=== %s ===\n", title);
        printf("1. Add\n");
        printf("2. List\n");
        printf("3. Back\n");
        subChoice = readInt("Enter choice: ");

        switch (subChoice) {
            case 1:
                lockData(data);
                addFunc(data);
                markDataDirty(data);
                unlockData(data);
                break;
            case 2:
                lockData(data);
                listFunc(data);
                unlockData(data);
                break;
            case 3:
                break;
            default:
                printf("Invalid choice. Try again.\n");
                break;
        }
    }
}

static int hasRoleId(const EMSData *data, int32_t roleId) {
    for (int32_t i = 0; i < data->roleCount; ++i) {
        if (data->roles[i].id == roleId) {
            return 1;
        }
    }
    return 0;
}

static void ensureDefaultAccounts(EMSData *data) {
    if (!hasRoleId(data, HR_ROLE_ID) && data->roleCount < MAX_ROLES) {
        Role *hrRole = &data->roles[data->roleCount++];
        hrRole->id = HR_ROLE_ID;
        copyStringSafe(hrRole->title, sizeof(hrRole->title), "HR");
        copyStringSafe(hrRole->description, sizeof(hrRole->description), "HR access");
    }

    if (!hasRoleId(data, EMPLOYEE_ROLE_ID) && data->roleCount < MAX_ROLES) {
        Role *employeeRole = &data->roles[data->roleCount++];
        employeeRole->id = EMPLOYEE_ROLE_ID;
        copyStringSafe(employeeRole->title, sizeof(employeeRole->title), "Employee");
        copyStringSafe(employeeRole->description, sizeof(employeeRole->description), "Employee access");
    }

    if (data->accountCount == 0) {
        AccessAccount *adminAccount = &data->accounts[data->accountCount++];
        adminAccount->id = DEFAULT_ADMIN_ROLE_ID;
        copyStringSafe(adminAccount->username, sizeof(adminAccount->username), "admin");
        copyStringSafe(adminAccount->password, sizeof(adminAccount->password), "admin");
        adminAccount->roleId = ADMIN_ROLE_ID;
        adminAccount->employeeId = INACTIVE_FLAG;
        adminAccount->active = ACTIVE_FLAG;

        AccessAccount *employeeAccount = &data->accounts[data->accountCount++];
        employeeAccount->id = DEFAULT_EMPLOYEE_ROLE_ID;
        copyStringSafe(employeeAccount->username, sizeof(employeeAccount->username), "employee");
        copyStringSafe(employeeAccount->password, sizeof(employeeAccount->password), "employee");
        employeeAccount->roleId = EMPLOYEE_ROLE_ID;
        employeeAccount->employeeId = INACTIVE_FLAG;
        employeeAccount->active = ACTIVE_FLAG;
    }
}

void initializeData(EMSData *data) {
    memset(data, 0, sizeof(*data));
    ems_mutex_init(&data->mutex);
    ems_condition_init(&data->saveRequested);
    loadAll(data);
    ensureDefaultAccounts(data);
}

int employeeExists(const EMSData *data, int32_t employeeId) {
    for (int32_t i = 0; i < data->employeeCount; ++i) {
        if (data->employees[i].id == employeeId && data->employees[i].active) return 1;
    }
    return 0;
}

int departmentExists(const EMSData *data, int32_t departmentId) {
    for (int32_t i = 0; i < data->departmentCount; ++i) {
        if (data->departments[i].id == departmentId) return 1;
    }
    return 0;
}

int roleExists(const EMSData *data, int32_t roleId) {
    for (int32_t i = 0; i < data->roleCount; ++i) {
        if (data->roles[i].id == roleId) return 1;
    }
    return 0;
}

void loadAll(EMSData *data) {
    FILE *file = fopen("data/employees.txt", "r");
    if (file) {
        int tempCount = 0;
        if (fscanf(file, "%d\n", &tempCount) == 1) {
            data->employeeCount = tempCount < 0 ? 0 : (tempCount > MAX_EMPLOYEES ? MAX_EMPLOYEES : tempCount);
            for (int32_t i = 0; i < tempCount && i < MAX_EMPLOYEES; ++i) {
                int tempActive = 0;
                fscanf(file, "%d|%49[^|\n]|%d|%d|%d|%49[^|\n]|%49[^|\n]|%d|%11[^|\n]|%19[^|\n]\n",
                       &data->employees[i].id,
                       data->employees[i].name,
                       &data->employees[i].departmentId,
                       &data->employees[i].roleId,
                       &data->employees[i].salary,
                       data->employees[i].email,
                       data->employees[i].phone,
                       &tempActive,
                       data->employees[i].joinDate,
                       data->employees[i].status);
                data->employees[i].active = (uint8_t)tempActive;
            }
        }
        fclose(file);
    }

    file = fopen("data/departments.txt", "r");
    if (file) {
        int count = 0;
        if (fscanf(file, "%d\n", &count) == 1) {
            data->departmentCount = count < 0 ? 0 : (count > MAX_DEPARTMENTS ? MAX_DEPARTMENTS : count);
            for (int32_t i = 0; i < count && i < MAX_DEPARTMENTS; ++i) {
                fscanf(file, "%d|%49[^|\n]|%49[^|\n]\n",
                       &data->departments[i].id,
                       data->departments[i].name,
                       data->departments[i].head);
            }
        }
        fclose(file);
    }

    file = fopen("data/roles.txt", "r");
    if (file) {
        int count = 0;
        if (fscanf(file, "%d\n", &count) == 1) {
            data->roleCount = count < 0 ? 0 : (count > MAX_ROLES ? MAX_ROLES : count);
            for (int32_t i = 0; i < count && i < MAX_ROLES; ++i) {
                fscanf(file, "%d|%49[^|\n]|%49[^|\n]\n",
                       &data->roles[i].id,
                       data->roles[i].title,
                       data->roles[i].description);
            }
        }
        fclose(file);
    }

    file = fopen("data/attendance.txt", "r");
    if (file) {
        int count = 0;
        if (fscanf(file, "%d\n", &count) == 1) {
            data->attendanceCount = count < 0 ? 0 : (count > MAX_ATTENDANCE ? MAX_ATTENDANCE : count);
            for (int32_t i = 0; i < count && i < MAX_ATTENDANCE; ++i) {
                fscanf(file, "%d|%d|%11[^|\n]|%d\n",
                       &data->attendance[i].id,
                       &data->attendance[i].employeeId,
                       data->attendance[i].date,
                       &data->attendance[i].status);
            }
        }
        fclose(file);
    }

    file = fopen("data/payroll.txt", "r");
    if (file) {
        int count = 0;
        if (fscanf(file, "%d\n", &count) == 1) {
            data->payrollCount = count < 0 ? 0 : (count > MAX_PAYROLL ? MAX_PAYROLL : count);
            for (int32_t i = 0; i < count && i < MAX_PAYROLL; ++i) {
                fscanf(file, "%d|%d|%11[^|\n]|%lf|%lf|%lf\n",
                       &data->payroll[i].id,
                       &data->payroll[i].employeeId,
                       data->payroll[i].month,
                       &data->payroll[i].salary,
                       &data->payroll[i].deductions,
                       &data->payroll[i].netPay);
            }
        }
        fclose(file);
    }

    file = fopen("data/leaves.txt", "r");
    if (file) {
        int count = 0;
        if (fscanf(file, "%d\n", &count) == 1) {
            data->leaveCount = count < 0 ? 0 : (count > MAX_LEAVES ? MAX_LEAVES : count);
            for (int32_t i = 0; i < count && i < MAX_LEAVES; ++i) {
                fscanf(file, "%d|%d|%11[^|\n]|%11[^|\n]|%99[^|\n]|%d\n",
                       &data->leaves[i].id,
                       &data->leaves[i].employeeId,
                       data->leaves[i].startDate,
                       data->leaves[i].endDate,
                       data->leaves[i].reason,
                       &data->leaves[i].status);
            }
        }
        fclose(file);
    }

    file = fopen("data/access.txt", "r");
    if (file) {
        char line[256];
        int count = 0;
        if (fgets(line, sizeof(line), file) && sscanf(line, "%d", &count) == 1) {
            data->accountCount = count < 0 ? 0 : (count > MAX_ACCOUNTS ? MAX_ACCOUNTS : count);
            for (int32_t i = 0; i < count && i < MAX_ACCOUNTS; ++i) {
                char *tokens[8] = {0};
                int tokenCount = 0;
                char *token = NULL;

                if (!fgets(line, sizeof(line), file)) {
                    break;
                }
                trimNewline(line);

                token = strtok(line, "|");
                while (token != NULL && tokenCount < 8) {
                    tokens[tokenCount++] = token;
                    token = strtok(NULL, "|");
                }

                if (tokenCount < 6) {
                    continue;
                }

                data->accounts[i].id = (int)strtol(tokens[0], NULL, 10);
                copyStringSafe(data->accounts[i].username, sizeof(data->accounts[i].username), tokens[1]);
                copyStringSafe(data->accounts[i].password, sizeof(data->accounts[i].password), tokens[2]);
                data->accounts[i].roleId = (int)strtol(tokens[3], NULL, 10);
                data->accounts[i].employeeId = (int)strtol(tokens[4], NULL, 10);
                data->accounts[i].active = (uint8_t)strtol(tokens[5], NULL, 10);
                data->accounts[i].passwordChangeRequired = (tokenCount >= 7) ? (uint8_t)strtol(tokens[6], NULL, 10) : 0;
                data->accounts[i].pendingApproval = (tokenCount >= 8) ? (int)strtol(tokens[7], NULL, 10) : 0;
            }
        }
        fclose(file);
    }

    file = fopen("data/projects.txt", "r");
    if (file) {
        int tempCount = 0;
        if (fscanf(file, "%d\n", &tempCount) == 1) {
            data->projectCount = tempCount < 0 ? 0 : (tempCount > MAX_PROJECTS ? MAX_PROJECTS : tempCount);
            for (int32_t i = 0; i < tempCount && i < MAX_PROJECTS; ++i) {
                int tempCompleted = 0;
                fscanf(file, "%d|%d|%49[^|\n]|%11[^|\n]|%d\n",
                       &data->projects[i].id,
                       &data->projects[i].employeeId,
                       data->projects[i].projectName,
                       data->projects[i].orientationDate,
                       &tempCompleted);
                data->projects[i].completed = (uint8_t)tempCompleted;
            }
        }
        fclose(file);
    }
}

static void saveAllUnlocked(EMSData *data) {
    FILE *file = fopen("data/employees.txt", "w");
    if (file) {
        fprintf(file, "%d\n", (int)data->employeeCount);
        for (int i = 0; i < data->employeeCount; ++i) {
            fprintf(file, "%d|%s|%d|%d|%d|%s|%s|%d|%s|%s\n",
                    data->employees[i].id,
                    data->employees[i].name,
                    data->employees[i].departmentId,
                    data->employees[i].roleId,
                    data->employees[i].salary,
                    data->employees[i].email,
                    data->employees[i].phone,
                    data->employees[i].active,
                    data->employees[i].joinDate,
                    data->employees[i].status);
        }
        fclose(file);
    }

    file = fopen("data/departments.txt", "w");
    if (file) {
        fprintf(file, "%d\n", (int)data->departmentCount);
        for (int i = 0; i < data->departmentCount; ++i) {
            fprintf(file, "%d|%s|%s\n",
                    data->departments[i].id,
                    data->departments[i].name,
                    data->departments[i].head);
        }
        fclose(file);
    }

    file = fopen("data/roles.txt", "w");
    if (file) {
        fprintf(file, "%d\n", (int)data->roleCount);
        for (int i = 0; i < data->roleCount; ++i) {
            fprintf(file, "%d|%s|%s\n",
                    data->roles[i].id,
                    data->roles[i].title,
                    data->roles[i].description);
        }
        fclose(file);
    }

    file = fopen("data/attendance.txt", "w");
    if (file) {
        fprintf(file, "%d\n", (int)data->attendanceCount);
        for (int i = 0; i < data->attendanceCount; ++i) {
            fprintf(file, "%d|%d|%s|%d\n",
                    data->attendance[i].id,
                    data->attendance[i].employeeId,
                    data->attendance[i].date,
                    data->attendance[i].status);
        }
        fclose(file);
    }

    file = fopen("data/payroll.txt", "w");
    if (file) {
        fprintf(file, "%d\n", (int)data->payrollCount);
        for (int i = 0; i < data->payrollCount; ++i) {
            fprintf(file, "%d|%d|%s|%.2f|%.2f|%.2f\n",
                    data->payroll[i].id,
                    data->payroll[i].employeeId,
                    data->payroll[i].month,
                    data->payroll[i].salary,
                    data->payroll[i].deductions,
                    data->payroll[i].netPay);
        }
        fclose(file);
    }

    file = fopen("data/leaves.txt", "w");
    if (file) {
        fprintf(file, "%d\n", (int)data->leaveCount);
        for (int i = 0; i < data->leaveCount; ++i) {
            fprintf(file, "%d|%d|%s|%s|%s|%d\n",
                    data->leaves[i].id,
                    data->leaves[i].employeeId,
                    data->leaves[i].startDate,
                    data->leaves[i].endDate,
                    data->leaves[i].reason,
                    data->leaves[i].status);
        }
        fclose(file);
    }

    file = fopen("data/access.txt", "w");
    if (file) {
        fprintf(file, "%d\n", (int)data->accountCount);
        for (int i = 0; i < data->accountCount; ++i) {
            fprintf(file, "%d|%s|%s|%d|%d|%d|%d|%d\n",
                    data->accounts[i].id,
                    data->accounts[i].username,
                    data->accounts[i].password,
                    data->accounts[i].roleId,
                    data->accounts[i].employeeId,
                    data->accounts[i].active,
                    data->accounts[i].passwordChangeRequired,
                    data->accounts[i].pendingApproval);
        }
        fclose(file);
    }

    file = fopen("data/projects.txt", "w");
    if (file) {
        fprintf(file, "%d\n", (int)data->projectCount);
        for (int i = 0; i < data->projectCount; ++i) {
            fprintf(file, "%d|%d|%s|%s|%d\n",
                    data->projects[i].id,
                    data->projects[i].employeeId,
                    data->projects[i].projectName,
                    data->projects[i].orientationDate,
                    data->projects[i].completed);
        }
        fclose(file);
    }
}

void saveAll(EMSData *data) {
    lockData(data);
    saveAllUnlocked(data);
    unlockData(data);
}

void printMenu(void) {
    printf("\n=== Employee Management System ===\n");
    printf("1. Employee login\n");
    printf("2. HR login\n");
    printf("3. Save and exit\n");
    printf("Enter choice: ");
}

void showRoleModuleMenu(EMSData *data, const char *roleLabel, int32_t employeeId) {
    int32_t choice = 0;
    while (1) {
        printf("\n=== %s Module Menu ===\n", roleLabel);

        if (strcmp(roleLabel, "HR") == 0) {
            printf("1. Employee onboarding\n");
            printf("2. Employee records\n");
            printf("3. Find employee by name/ID\n");
            printf("4. Department management\n");
            printf("5. Attendance management\n");
            printf("6. Leave management\n");
            printf("7. Login & access management\n");
            printf("8. Role management\n");
            printf("9. Payroll\n");
            printf("10. Project orientation\n");
            printf("11. Reporting dashboard\n");
            printf("12. Back to login menu\n");
        } else {
            printf("1. View profile\n");
            printf("2. Logout\n");
        }

        printf("Enter choice: ");
        choice = readInt("");

        if (strcmp(roleLabel, "HR") == 0) {
            switch (choice) {
                case 1:
                    showModuleMenu(data, "Employee onboarding", addEmployee, listEmployees);
                    break;
                case 2:
                    lockData(data);
                    showEmployeeRecordsMenu(data);
                    markDataDirty(data);
                    unlockData(data);
                    break;
                case 3:
                    lockData(data);
                    findEmployeeByNameOrId(data);
                    unlockData(data);
                    break;
                case 4:
                    showModuleMenu(data, "Department management", addDepartment, listDepartments);
                    break;
                case 5:
                    showModuleMenu(data, "Attendance management", addAttendance, listAttendance);
                    break;
                case 6:
                    showModuleMenu(data, "Leave management", addLeaveRequest, listLeaveRequests);
                    break;
                case 7:
                    lockData(data);
                    showAccessManagementMenu(data);
                    unlockData(data);
                    break;
                case 8:
                    showModuleMenu(data, "Role management", addRole, listRoles);
                    break;
                case 9:
                    showModuleMenu(data, "Payroll", addPayroll, listPayroll);
                    break;
                case 10:
                    showModuleMenu(data, "Project orientation", addProjectOrientation, listProjectOrientations);
                    break;
                case 11:
                    lockData(data);
                    showReportingDashboard(data);
                    unlockData(data);
                    break;
                case 12:
                    printf("Returning to login menu.\n");
                    return;
                default:
                    printf("Invalid choice. Try again.\n");
                    break;
            }
        } else {
            switch (choice) {
                case 1:
                    lockData(data);
                    for (int32_t i = 0; i < data->employeeCount; ++i) {
                        if (data->employees[i].id == employeeId) {
                            printf("ID: %d | Name: %s | Department: %d | Role: %d | Email: %s | Status: %s\n", data->employees[i].id, data->employees[i].name, data->employees[i].departmentId, data->employees[i].roleId, data->employees[i].email, data->employees[i].status);
                            break;
                        }
                    }
                    unlockData(data);
                    break;
                case 2:
                    printf("Logging out from module menu.\n");
                    return;
                default:
                    printf("Invalid choice. Try again.\n");
                    break;
            }
        }
    }
}
