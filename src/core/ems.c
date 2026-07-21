#include "ems.h"

static void trimNewline(char *text) {
    size_t len = strlen(text);
    if (len > 0 && text[len - 1] == '\n') {
        text[len - 1] = '\0';
    }
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

int readInt(const char *prompt) {
    char buffer[64];
    int value = 0;
    printf("%s", prompt);
    if (!fgets(buffer, sizeof(buffer), stdin)) {
        return 0;
    }
    sscanf(buffer, "%d", &value);
    return value;
}

int readValidatedInt(const char *prompt, int minValue, int maxValue, const char *formatHint) {
    char buffer[64];
    int value = 0;

    while (1) {
        printf("%s", prompt);
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            return 0;
        }
        trimNewline(buffer);

        if (sscanf(buffer, "%d", &value) == 1 && value >= minValue && value <= maxValue) {
            return value;
        }

        printf("Warning: invalid input. Expected format: %s\n", formatHint);
    }
}

double readDouble(const char *prompt) {
    char buffer[64];
    double value = 0.0;
    printf("%s", prompt);
    if (!fgets(buffer, sizeof(buffer), stdin)) {
        return 0.0;
    }
    sscanf(buffer, "%lf", &value);
    return value;
}

double readValidatedDouble(const char *prompt, double minValue, double maxValue, const char *formatHint) {
    char buffer[64];
    double value = 0.0;

    while (1) {
        printf("%s", prompt);
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            return 0.0;
        }
        trimNewline(buffer);

        if (sscanf(buffer, "%lf", &value) == 1 && value >= minValue && value <= maxValue) {
            return value;
        }

        printf("Warning: invalid input. Expected format: %s\n", formatHint);
    }
}

void showModuleMenu(EMSData *data, const char *title, int (*addFunc)(EMSData *), void (*listFunc)(const EMSData *)) {
    int subChoice = 0;
    while (subChoice != 3) {
        printf("\n=== %s ===\n", title);
        printf("1. Add\n");
        printf("2. List\n");
        printf("3. Back\n");
        printf("Enter choice: ");
        scanf("%d", &subChoice);
        while (getchar() != '\n') {
        }

        switch (subChoice) {
            case 1:
                addFunc(data);
                break;
            case 2:
                listFunc(data);
                break;
            case 3:
                break;
            default:
                printf("Invalid choice. Try again.\n");
                break;
        }
    }
}

void initializeData(EMSData *data) {
    memset(data, 0, sizeof(*data));
    loadAll(data);
}

void loadAll(EMSData *data) {
    FILE *file = fopen("data/employees.txt", "r");
    if (file) {
        int count = 0;
        if (fscanf(file, "%d\n", &count) == 1) {
            data->employeeCount = count;
            for (int i = 0; i < count && i < MAX_EMPLOYEES; ++i) {
                fscanf(file, "%d|%49[^|]|%d|%d|%d|%49[^|]|%49[^|]|%d|%11[^|]|%19[^|]\n",
                       &data->employees[i].id,
                       data->employees[i].name,
                       &data->employees[i].departmentId,
                       &data->employees[i].roleId,
                       &data->employees[i].salary,
                       data->employees[i].email,
                       data->employees[i].phone,
                       &data->employees[i].active,
                       data->employees[i].joinDate,
                       data->employees[i].status);
            }
        }
        fclose(file);
    }

    file = fopen("data/departments.txt", "r");
    if (file) {
        int count = 0;
        if (fscanf(file, "%d\n", &count) == 1) {
            data->departmentCount = count;
            for (int i = 0; i < count && i < MAX_DEPARTMENTS; ++i) {
                fscanf(file, "%d|%49[^|]|%49[^|]\n",
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
            data->roleCount = count;
            for (int i = 0; i < count && i < MAX_ROLES; ++i) {
                fscanf(file, "%d|%49[^|]|%49[^|]\n",
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
            data->attendanceCount = count;
            for (int i = 0; i < count && i < MAX_ATTENDANCE; ++i) {
                fscanf(file, "%d|%d|%11[^|]|%d\n",
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
            data->payrollCount = count;
            for (int i = 0; i < count && i < MAX_PAYROLL; ++i) {
                fscanf(file, "%d|%d|%11[^|]|%lf|%lf|%lf\n",
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
            data->leaveCount = count;
            for (int i = 0; i < count && i < MAX_LEAVES; ++i) {
                fscanf(file, "%d|%d|%11[^|]|%11[^|]|%99[^|]|%d\n",
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
        int count = 0;
        if (fscanf(file, "%d\n", &count) == 1) {
            data->accountCount = count;
            for (int i = 0; i < count && i < MAX_ACCOUNTS; ++i) {
                fscanf(file, "%d|%31[^|]|%31[^|]|%d|%d|%d\n",
                       &data->accounts[i].id,
                       data->accounts[i].username,
                       data->accounts[i].password,
                       &data->accounts[i].roleId,
                       &data->accounts[i].employeeId,
                       &data->accounts[i].active);
            }
        }
        fclose(file);
    }

    file = fopen("data/projects.txt", "r");
    if (file) {
        int count = 0;
        if (fscanf(file, "%d\n", &count) == 1) {
            data->projectCount = count;
            for (int i = 0; i < count && i < MAX_PROJECTS; ++i) {
                fscanf(file, "%d|%d|%49[^|]|%11[^|]|%d\n",
                       &data->projects[i].id,
                       &data->projects[i].employeeId,
                       data->projects[i].projectName,
                       data->projects[i].orientationDate,
                       &data->projects[i].completed);
            }
        }
        fclose(file);
    }
}

void saveAll(EMSData *data) {
    FILE *file = fopen("data/employees.txt", "w");
    if (file) {
        fprintf(file, "%d\n", data->employeeCount);
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
        fprintf(file, "%d\n", data->departmentCount);
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
        fprintf(file, "%d\n", data->roleCount);
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
        fprintf(file, "%d\n", data->attendanceCount);
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
        fprintf(file, "%d\n", data->payrollCount);
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
        fprintf(file, "%d\n", data->leaveCount);
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
        fprintf(file, "%d\n", data->accountCount);
        for (int i = 0; i < data->accountCount; ++i) {
            fprintf(file, "%d|%s|%s|%d|%d|%d\n",
                    data->accounts[i].id,
                    data->accounts[i].username,
                    data->accounts[i].password,
                    data->accounts[i].roleId,
                    data->accounts[i].employeeId,
                    data->accounts[i].active);
        }
        fclose(file);
    }

    file = fopen("data/projects.txt", "w");
    if (file) {
        fprintf(file, "%d\n", data->projectCount);
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

void printMenu(void) {
    printf("\n=== Employee Management System ===\n");
    printf("1. Employee onboarding\n");
    printf("2. Employee records\n");
    printf("3. Department management\n");
    printf("4. Attendance management\n");
    printf("5. Leave management\n");
    printf("6. Login & access management\n");
    printf("7. Role management\n");
    printf("8. Payroll\n");
    printf("9. Project orientation\n");
    printf("6. Employee login\n");
    printf("7. HR login\n");
    printf("8. Login & access management\n");
    printf("9. Role management\n");
    printf("10. Payroll\n");
    printf("11. Project orientation\n");
    printf("12. Reporting dashboard\n");
    printf("13. Save and exit\n");
    printf("Enter choice: ");
}
