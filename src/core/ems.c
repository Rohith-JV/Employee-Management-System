#include "ems.h"

static void saveAllUnlocked(EMSData *data);

void lockData(EMSData *data) {
    ems_mutex_lock(&data->mutex);
}

void unlockData(EMSData *data) {
    ems_mutex_unlock(&data->mutex);
}

/* Caller must hold data->mutex: the flag and the signal have to be paired with
   the wait predicate, otherwise the wakeup can be lost. */
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
        if (data->dirty) {
            data->dirty = 0;
            saveAllUnlocked(data);
        }
    }
    if (data->dirty) {
        data->dirty = 0;
        saveAllUnlocked(data);
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
    int wasRunning;

    lockData(data);
    wasRunning = data->autosaveRunning;
    data->autosaveRunning = 0;
    ems_condition_signal(&data->saveRequested);
    unlockData(data);

    if (wasRunning) {
        ems_thread_join(data->autosaveThread);
    }
}

static void trimNewline(char *text) {
    size_t len = strlen(text);
    if (len > 0 && (text[len - 1] == '\n' || text[len - 1] == '\r')) {
        text[len - 1] = '\0';
    }
}

static void trimSurroundingSpaces(char *text) {
    size_t len = strlen(text);
    size_t start = 0;

    while (len > start && isspace((unsigned char)text[len - 1])) {
        text[--len] = '\0';
    }
    while (text[start] != '\0' && isspace((unsigned char)text[start])) {
        ++start;
    }
    if (start > 0) {
        memmove(text, text + start, len - start + 1);
    }
}

static void sanitizeEmployees(EMSData *data) {
    int32_t writeIndex = 0;
    for (int32_t i = 0; i < data->employeeCount; ++i) {
        if (data->employees[i].id > 0) {
            if (i != writeIndex) {
                data->employees[writeIndex] = data->employees[i];
            }
            writeIndex++;
        }
    }
    for (int32_t i = writeIndex; i < data->employeeCount; ++i) {
        memset(&data->employees[i], 0, sizeof(data->employees[i]));
    }
    data->employeeCount = writeIndex;
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
    trimSurroundingSpaces(buffer);
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

    if (!isdigit((unsigned char)input[0]) || !isdigit((unsigned char)input[1]) ||
        !isdigit((unsigned char)input[2]) || !isdigit((unsigned char)input[3]) ||
        input[4] != '-' ||
        !isdigit((unsigned char)input[5]) || !isdigit((unsigned char)input[6]) ||
        input[7] != '-' ||
        !isdigit((unsigned char)input[8]) || !isdigit((unsigned char)input[9])) {
        return 0;
    }

    int year  = (input[0] - '0') * 1000 + (input[1] - '0') * 100 + (input[2] - '0') * 10 + (input[3] - '0');
    int month = (input[5] - '0') * 10 + (input[6] - '0');
    int day   = (input[8] - '0') * 10 + (input[9] - '0');

    if (month < 1 || month > 12) {
        return 0;
    }

    int daysInMonth[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
        daysInMonth[2] = 29;
    }

    return (day >= 1 && day <= daysInMonth[month]);
}

int isMonth(const char *input) {
    if (strlen(input) != 7) {
        return 0;
    }
    if (!isdigit((unsigned char)input[0]) || !isdigit((unsigned char)input[1]) ||
        !isdigit((unsigned char)input[2]) || !isdigit((unsigned char)input[3]) ||
        input[4] != '-' ||
        !isdigit((unsigned char)input[5]) || !isdigit((unsigned char)input[6])) {
        return 0;
    }

    int month = (input[5] - '0') * 10 + (input[6] - '0');
    return month >= 1 && month <= 12;
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
    if (!input) {
        return 0;
    }

    size_t len = strlen(input);
    if (len < 8 || len > 32) {
        return 0;
    }

    regex_t regex_upper;
    regex_t regex_symbol;

    /* Compile regex patterns */
    if (regcomp(&regex_upper, "[A-Z]", REG_EXTENDED | REG_NOSUB) != 0) {
        return 0;
    }
    if (regcomp(&regex_symbol, "[^a-zA-Z0-9]", REG_EXTENDED | REG_NOSUB) != 0) {
        regfree(&regex_upper);
        return 0;
    }

    /* Execute pattern matching */
    int upper_match = regexec(&regex_upper, input, 0, NULL, 0);
    int symbol_match = regexec(&regex_symbol, input, 0, NULL, 0);

    /* Free allocated regex resources */
    regfree(&regex_upper);
    regfree(&regex_symbol);

    return (upper_match == 0) && (symbol_match == 0);
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
        return EMS_INPUT_EOF;
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
            return EMS_INPUT_EOF;
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
        if (subChoice == EMS_INPUT_EOF) {
            return;
        }

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

static int hasUsername(const EMSData *data, const char *username) {
    for (int32_t i = 0; i < data->accountCount; ++i) {
        if (strcmp(data->accounts[i].username, username) == 0) {
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

    /* The administrator account must always exist, otherwise a data file that
       only holds employee accounts locks everyone out of the system. */
    if (!hasUsername(data, "admin") && data->accountCount < MAX_ACCOUNTS) {
        AccessAccount *adminAccount = &data->accounts[data->accountCount++];
        memset(adminAccount, 0, sizeof(*adminAccount));
        adminAccount->id = DEFAULT_ADMIN_ROLE_ID;
        copyStringSafe(adminAccount->username, sizeof(adminAccount->username), "admin");
        copyStringSafe(adminAccount->password, sizeof(adminAccount->password), "admin");
        adminAccount->roleId = ADMIN_ROLE_ID;
        adminAccount->employeeId = INACTIVE_FLAG;
        adminAccount->active = ACTIVE_FLAG;
    }

    /* Demo employee login, only seeded on a brand-new database. */
    if (data->accountCount == 1 && !hasUsername(data, "employee")) {
        AccessAccount *employeeAccount = &data->accounts[data->accountCount++];
        memset(employeeAccount, 0, sizeof(*employeeAccount));
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

/* Splits a pipe-delimited line in place, keeping empty fields. */
static int splitFields(char *line, char *fields[], int maxFields) {
    int count = 0;
    char *cursor = line;

    while (count < maxFields) {
        char *separator = strchr(cursor, '|');
        fields[count++] = cursor;
        if (separator == NULL) {
            break;
        }
        *separator = '\0';
        cursor = separator + 1;
    }
    return count;
}

void loadAll(EMSData *data) {
    FILE *file = fopen("data/employees.dat", "r");
    if (file) {
        char line[512];
        int tempCount = 0;
        if (fgets(line, sizeof(line), file) && sscanf(line, "%d", &tempCount) == 1) {
            int32_t loadedCount = 0;
            for (int32_t i = 0; i < tempCount && i < MAX_EMPLOYEES; ++i) {
                Employee tempEmployee;
                char *fields[10] = {0};

                if (!fgets(line, sizeof(line), file)) {
                    break;
                }
                trimNewline(line);
                if (splitFields(line, fields, 10) < 10) {
                    continue;
                }

                memset(&tempEmployee, 0, sizeof(tempEmployee));
                tempEmployee.id = (int32_t)strtol(fields[0], NULL, 10);
                copyStringSafe(tempEmployee.name, sizeof(tempEmployee.name), fields[1]);
                tempEmployee.departmentId = (int32_t)strtol(fields[2], NULL, 10);
                tempEmployee.roleId = (int32_t)strtol(fields[3], NULL, 10);
                tempEmployee.salary = (int32_t)strtol(fields[4], NULL, 10);
                copyStringSafe(tempEmployee.email, sizeof(tempEmployee.email), fields[5]);
                copyStringSafe(tempEmployee.phone, sizeof(tempEmployee.phone), fields[6]);
                tempEmployee.active = (uint8_t)strtol(fields[7], NULL, 10);
                copyStringSafe(tempEmployee.joinDate, sizeof(tempEmployee.joinDate), fields[8]);
                copyStringSafe(tempEmployee.status, sizeof(tempEmployee.status), fields[9]);

                if (tempEmployee.id > 0) {
                    data->employees[loadedCount++] = tempEmployee;
                }
            }
            data->employeeCount = loadedCount;
        }
        fclose(file);
    }

    file = fopen("data/departments.dat", "r");
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

    file = fopen("data/roles.dat", "r");
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

    file = fopen("data/attendance.dat", "r");
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

    file = fopen("data/payroll.dat", "r");
    if (file) {
        int count = 0;
        char line[256];
        if (fgets(line, sizeof(line), file) && sscanf(line, "%d", &count) == 1) {
            data->payrollCount = count < 0 ? 0 : (count > MAX_PAYROLL ? MAX_PAYROLL : count);
            for (int32_t i = 0; i < count && i < MAX_PAYROLL; ++i) {
                if (!fgets(line, sizeof(line), file)) break;
                trimNewline(line);
                char *tokens[7] = {0};
                int tokenCount = 0;
                char *token = strtok(line, "|");
                while (token != NULL && tokenCount < 7) {
                    tokens[tokenCount++] = token;
                    token = strtok(NULL, "|");
                }
                if (tokenCount < 6) continue;
                data->payroll[i].id = (int32_t)strtol(tokens[0], NULL, 10);
                data->payroll[i].employeeId = (int32_t)strtol(tokens[1], NULL, 10);
                copyStringSafe(data->payroll[i].month, sizeof(data->payroll[i].month), tokens[2]);
                data->payroll[i].salary = strtod(tokens[3], NULL);
                data->payroll[i].deductions = strtod(tokens[4], NULL);
                data->payroll[i].netPay = strtod(tokens[5], NULL);
                data->payroll[i].credited = (tokenCount >= 7) ? (uint8_t)strtol(tokens[6], NULL, 10) : 0;
            }
        }
        fclose(file);
    }

    file = fopen("data/leaves.dat", "r");
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

    file = fopen("data/access.dat", "r");
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
                data->accounts[i].pendingApproval = (tokenCount >= 8) ? (uint8_t)strtol(tokens[7], NULL, 10) : 0;
            }
        }
        fclose(file);
    }

    file = fopen("data/projects.dat", "r");
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
    sanitizeEmployees(data);
    FILE *file = fopen("data/employees.dat", "w");
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

    file = fopen("data/departments.dat", "w");
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

    file = fopen("data/roles.dat", "w");
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

    file = fopen("data/attendance.dat", "w");
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

    file = fopen("data/payroll.dat", "w");
    if (file) {
        fprintf(file, "%d\n", (int)data->payrollCount);
        for (int i = 0; i < data->payrollCount; ++i) {
            fprintf(file, "%d|%d|%s|%.2f|%.2f|%.2f|%d\n",
                    data->payroll[i].id,
                    data->payroll[i].employeeId,
                    data->payroll[i].month,
                    data->payroll[i].salary,
                    data->payroll[i].deductions,
                    data->payroll[i].netPay,
                    data->payroll[i].credited);
        }
        fclose(file);
    }

    file = fopen("data/leaves.dat", "w");
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

    file = fopen("data/access.dat", "w");
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

    file = fopen("data/projects.dat", "w");
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

/* Copies the pending request with the given id, if it is still pending.
   Caller must hold data->mutex. */
static int copyPendingLeave(const EMSData *data, int32_t leaveId, LeaveRequest *out) {
    for (int32_t i = 0; i < data->leaveCount; ++i) {
        if (data->leaves[i].id == leaveId && data->leaves[i].status == 0) {
            *out = data->leaves[i];
            return 1;
        }
    }
    return 0;
}

static int setLeaveStatus(EMSData *data, int32_t leaveId, int32_t status) {
    int applied = 0;

    lockData(data);
    for (int32_t i = 0; i < data->leaveCount; ++i) {
        if (data->leaves[i].id == leaveId && data->leaves[i].status == 0) {
            data->leaves[i].status = status;
            markDataDirty(data);
            applied = 1;
            break;
        }
    }
    unlockData(data);
    return applied;
}

void reviewLeaveRequests(EMSData *data) {
    while (1) {
        LeaveRequest selected;
        int32_t choice = 0;
        int32_t decision = 0;
        int found = 0;

        printf("\n=== HR Leave Requests ===\n");
        lockData(data);
        for (int32_t i = 0; i < data->leaveCount; ++i) {
            if (data->leaves[i].status == 0) {
                const LeaveRequest *leave = &data->leaves[i];
                printf("ID: %d | Employee: %d | %s to %s | %s | Status: Pending\n",
                       leave->id,
                       leave->employeeId,
                       leave->startDate,
                       leave->endDate,
                       leave->reason);
                found = 1;
            }
        }
        unlockData(data);

        if (!found) {
            printf("No pending leave requests.\n");
            return;
        }

        choice = readInt("Enter leave request ID to review (0 to go back): ");
        if (choice == 0 || choice == EMS_INPUT_EOF) {
            return;
        }

        lockData(data);
        found = copyPendingLeave(data, choice, &selected);
        unlockData(data);
        if (!found) {
            printf("No pending leave request with ID %d.\n", choice);
            continue;
        }

        printf("\nLeave request %d for employee %d:\n", selected.id, selected.employeeId);
        printf("%s to %s | %s\n", selected.startDate, selected.endDate, selected.reason);
        printf("1. Approve\n2. Reject\n3. Back\n");
        decision = readInt("Enter choice: ");
        if (decision == EMS_INPUT_EOF) {
            return;
        }
        if (decision == 1 || decision == 2) {
            if (setLeaveStatus(data, selected.id, decision)) {
                printf(decision == 1 ? "Leave request approved.\n" : "Leave request rejected.\n");
            } else {
                printf("Leave request %d is no longer pending.\n", selected.id);
            }
        }
    }
}

/* Caller must hold data->mutex. */
static int copyUncreditedPayroll(const EMSData *data, int32_t payrollId, PayrollRecord *out) {
    for (int32_t i = 0; i < data->payrollCount; ++i) {
        if (data->payroll[i].id == payrollId && !data->payroll[i].credited) {
            *out = data->payroll[i];
            return 1;
        }
    }
    return 0;
}

static int creditPayroll(EMSData *data, int32_t payrollId) {
    int applied = 0;

    lockData(data);
    for (int32_t i = 0; i < data->payrollCount; ++i) {
        if (data->payroll[i].id == payrollId && !data->payroll[i].credited) {
            data->payroll[i].credited = 1;
            markDataDirty(data);
            applied = 1;
            break;
        }
    }
    unlockData(data);
    return applied;
}

void reviewPayrollReleases(EMSData *data) {
    while (1) {
        PayrollRecord selected;
        int32_t choice = 0;
        int32_t decision = 0;
        int found = 0;

        printf("\n=== HR Payroll Releases ===\n");
        lockData(data);
        for (int32_t i = 0; i < data->payrollCount; ++i) {
            if (!data->payroll[i].credited) {
                const PayrollRecord *pay = &data->payroll[i];
                printf("ID: %d | Employee: %d | %s | Net: %.2f | Credited: No\n",
                       pay->id,
                       pay->employeeId,
                       pay->month,
                       pay->netPay);
                found = 1;
            }
        }
        unlockData(data);

        if (!found) {
            printf("No uncredited payroll records.\n");
            return;
        }

        choice = readInt("Enter payroll record ID to release (0 to go back): ");
        if (choice == 0 || choice == EMS_INPUT_EOF) {
            return;
        }

        lockData(data);
        found = copyUncreditedPayroll(data, choice, &selected);
        unlockData(data);
        if (!found) {
            printf("No uncredited payroll record with ID %d.\n", choice);
            continue;
        }

        printf("\nPayroll record %d for employee %d:\n", selected.id, selected.employeeId);
        printf("%s | Gross: %.2f | Deductions: %.2f | Net: %.2f\n",
               selected.month, selected.salary, selected.deductions, selected.netPay);
        printf("1. Release payment\n2. Back\n");
        decision = readInt("Enter choice: ");
        if (decision == EMS_INPUT_EOF) {
            return;
        }
        if (decision == 1) {
            if (creditPayroll(data, selected.id)) {
                printf("Payroll payment released.\n");
            } else {
                printf("Payroll record %d was already credited.\n", selected.id);
            }
        }
    }
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
            printf("11. Leave request approvals\n");
            printf("12. Payroll release reviews\n");
            printf("13. Reporting dashboard\n");
            printf("14. Back to login menu\n");
        } else {
            printf("1. Attendance management\n");
            printf("2. Leave management\n");
            printf("3. Payroll\n");
            printf("4. Project orientation\n");
            printf("5. View profile\n");
            printf("6. Logout\n");
        }

        printf("Enter choice: ");
        choice = readInt("");
        if (choice == EMS_INPUT_EOF) {
            return;
        }

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
                    reviewLeaveRequests(data);
                    break;
                case 12:
                    reviewPayrollReleases(data);
                    break;
                case 13:
                    lockData(data);
                    showReportingDashboard(data);
                    unlockData(data);
                    break;
                case 14:
                    printf("Returning to login menu.\n");
                    return;
                default:
                    printf("Invalid choice. Try again.\n");
                    break;
            }
        } else {
            switch (choice) {
                case 1:
                    showEmployeeAttendanceMenu(data, employeeId);
                    break;
                case 2:
                    showEmployeeLeaveMenu(data, employeeId);
                    break;
                case 3:
                    lockData(data);
                    viewMyPayroll(data, employeeId);
                    unlockData(data);
                    break;
                case 4:
                    showEmployeeProjectMenu(data, employeeId);
                    break;
                case 5:
                    lockData(data);
                    showOwnProfile(data, employeeId);
                    unlockData(data);
                    break;
                case 6:
                    printf("Logging out from employee menu.\n");
                    return;
                default:
                    printf("Invalid choice. Try again.\n");
                    break;
            }
        }
    }
}
