#ifndef EMS_H
#define EMS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>
#include "threading.h"

#define MAX_EMPLOYEES 100
#define MAX_DEPARTMENTS 50
#define MAX_ROLES 50
#define MAX_ATTENDANCE 500
#define MAX_PAYROLL 500
#define MAX_LEAVES 500
#define MAX_ACCOUNTS 200
#define MAX_PROJECTS 300
#define MAX_NAME_LENGTH 50
#define MAX_EMAIL_LENGTH 50
#define MAX_PHONE_LENGTH 50
#define MAX_DATE_LENGTH 12
#define MAX_STATUS_LENGTH 20
#define MAX_USERNAME_LENGTH 32
#define MAX_PASSWORD_LENGTH 32
#define MAX_REASON_LENGTH 100
#define MAX_PROJECT_NAME_LENGTH 50
#define MAX_INPUT_BUFFER_LENGTH 64
#define MIN_USERNAME_LENGTH 3
#define MAX_USERNAME_LENGTH_VALUE 20
#define MIN_PASSWORD_LENGTH 4
#define MAX_PASSWORD_LENGTH_VALUE 32
#define MIN_PHONE_DIGITS 10
#define MAX_PHONE_DIGITS 15
#define MAX_DEPARTMENT_ID 100000
#define MAX_ROLE_ID 100000
#define MAX_SALARY 100000000
#define MAX_DOUBLE_VALUE 1000000000.0
#define MIN_STATUS_VALUE 0
#define MAX_STATUS_VALUE 2
#define EMPLOYEE_ROLE_ID 2
#define HR_ROLE_ID 1
#define ADMIN_ROLE_ID 1
#define DEFAULT_EMPLOYEE_ROLE_ID 2
#define DEFAULT_ADMIN_ROLE_ID 1
#define ACTIVE_FLAG 1
#define INACTIVE_FLAG 0
#define EXIT_MENU_CHOICE 3
#define BACK_TO_MAIN_MENU 3
#define EMS_INPUT_EOF (-2147483647)

typedef struct {
    int32_t id;
    char name[50];
    int32_t departmentId;
    int32_t roleId;
    int32_t salary;
    char email[MAX_EMAIL_LENGTH];
    char phone[MAX_PHONE_LENGTH];
    uint8_t active;
    char joinDate[MAX_DATE_LENGTH];
    char status[MAX_STATUS_LENGTH];
} Employee;

typedef struct {
    int32_t id;
    char name[MAX_NAME_LENGTH];
    char head[MAX_NAME_LENGTH];
} Department;

typedef struct {
    int32_t id;
    char title[MAX_NAME_LENGTH];
    char description[MAX_NAME_LENGTH];
} Role;

typedef struct {
    int32_t id;
    int32_t employeeId;
    char date[MAX_DATE_LENGTH];
    int32_t status;
} AttendanceRecord;

typedef struct {
    int32_t id;
    int32_t employeeId;
    char month[MAX_DATE_LENGTH];
    double salary;
    double deductions;
    double netPay;
    uint8_t credited;
} PayrollRecord;

typedef struct {
    int32_t id;
    int32_t employeeId;
    char startDate[MAX_DATE_LENGTH];
    char endDate[MAX_DATE_LENGTH];
    char reason[MAX_REASON_LENGTH];
    int32_t status;
} LeaveRequest;

typedef struct {
    int32_t id;
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    int32_t roleId;
    int32_t employeeId;
    uint8_t active;
    uint8_t passwordChangeRequired;
    uint8_t pendingApproval;
} AccessAccount;

typedef struct {
    int32_t id;
    int32_t employeeId;
    char projectName[MAX_PROJECT_NAME_LENGTH];
    char orientationDate[MAX_DATE_LENGTH];
    uint8_t completed;
} ProjectOrientation;

typedef struct {
    Employee employees[MAX_EMPLOYEES];
    int32_t employeeCount;

    Department departments[MAX_DEPARTMENTS];
    int32_t departmentCount;

    Role roles[MAX_ROLES];
    int32_t roleCount;

    AttendanceRecord attendance[MAX_ATTENDANCE];
    int32_t attendanceCount;

    PayrollRecord payroll[MAX_PAYROLL];
    int32_t payrollCount;

    LeaveRequest leaves[MAX_LEAVES];
    int32_t leaveCount;

    AccessAccount accounts[MAX_ACCOUNTS];
    int32_t accountCount;

    ProjectOrientation projects[MAX_PROJECTS];
    int32_t projectCount;

    /* Shared-state coordination for the autosave thread. */
    EmsMutex mutex;
    EmsCondition saveRequested;
    EmsThread autosaveThread;
    int autosaveRunning;
    int dirty;
} EMSData;

int copyStringSafe(char *dest, size_t destSize, const char *src);
void readText(const char *prompt, char *buffer, size_t size);
void readValidatedText(const char *prompt, char *buffer, size_t size, int (*validator)(const char *), const char *formatHint);
int32_t readInt(const char *prompt);
int32_t readValidatedInt(const char *prompt, int32_t minValue, int32_t maxValue, const char *formatHint);
double readDouble(const char *prompt);
double readValidatedDouble(const char *prompt, double minValue, double maxValue, const char *formatHint);
int isAlphaText(const char *input);
int isEmail(const char *input);
int isPhone(const char *input);
int isDate(const char *input);
int isMonth(const char *input);
int isStatusValue(const char *input);
int isTextWithSpaces(const char *input);
int isUsername(const char *input);
int isPassword(const char *input);
void initializeData(EMSData *data);
int startAutosave(EMSData *data);
void stopAutosave(EMSData *data);
void lockData(EMSData *data);
void unlockData(EMSData *data);
/* Requires data->mutex to be held by the caller. */
void markDataDirty(EMSData *data);
int employeeExists(const EMSData *data, int32_t employeeId);
int departmentExists(const EMSData *data, int32_t departmentId);
int roleExists(const EMSData *data, int32_t roleId);
void saveAll(EMSData *data);
void loadAll(EMSData *data);
void printMenu(void);
void showRoleModuleMenu(EMSData *data, const char *roleLabel, int32_t employeeId);
void showOwnProfile(EMSData *data, int32_t employeeId);
void showEmployeeAttendanceMenu(EMSData *data, int32_t employeeId);
int markAttendance(EMSData *data, int32_t employeeId);
void viewMyAttendance(const EMSData *data, int32_t employeeId);
void showEmployeeLeaveMenu(EMSData *data, int32_t employeeId);
int applyLeave(EMSData *data, int32_t employeeId);
void viewMyLeaves(const EMSData *data, int32_t employeeId);
void reviewLeaveRequests(EMSData *data);
void reviewPayrollReleases(EMSData *data);
void viewMyPayroll(const EMSData *data, int32_t employeeId);
void showEmployeeProjectMenu(EMSData *data, int32_t employeeId);
void viewMyProjects(const EMSData *data, int32_t employeeId);
void viewProjectProgress(const EMSData *data, int32_t employeeId);
void showModuleMenu(EMSData *data, const char *title, int (*addFunc)(EMSData *), void (*listFunc)(const EMSData *));
int addEmployee(EMSData *data);
void listEmployees(const EMSData *data);
void findEmployeeByNameOrId(const EMSData *data);
void showEmployeeRecordsMenu(EMSData *data);
void searchEmployees(const EMSData *data);
void updateEmployee(EMSData *data);
void deleteEmployee(EMSData *data);
int addDepartment(EMSData *data);
void listDepartments(const EMSData *data);
int addRole(EMSData *data);
void listRoles(const EMSData *data);
int addAttendance(EMSData *data);
void listAttendance(const EMSData *data);
int addPayroll(EMSData *data);
void listPayroll(const EMSData *data);
int addLeaveRequest(EMSData *data);
void listLeaveRequests(const EMSData *data);
int addAccessAccount(EMSData *data);
void listAccessAccounts(const EMSData *data);
int createEmployeeAccessAccount(EMSData *data, int32_t employeeId, const char *username, const char *password);
void showAccessManagementMenu(EMSData *data);
int employeeLogin(EMSData *data, int32_t *employeeId);
int hrLogin(EMSData *data);
int addProjectOrientation(EMSData *data);
void listProjectOrientations(const EMSData *data);
void showReportingDashboard(const EMSData *data);

#endif
