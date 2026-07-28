#ifndef EMS_H
#define EMS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>

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

typedef struct {
    int id;
    char name[50];
    int departmentId;
    int roleId;
    int salary;
    char email[MAX_EMAIL_LENGTH];
    char phone[MAX_PHONE_LENGTH];
    int active;
    char joinDate[MAX_DATE_LENGTH];
    char status[MAX_STATUS_LENGTH];
} Employee;

typedef struct {
    int id;
    char name[MAX_NAME_LENGTH];
    char head[MAX_NAME_LENGTH];
} Department;

typedef struct {
    int id;
    char title[MAX_NAME_LENGTH];
    char description[MAX_NAME_LENGTH];
} Role;

typedef struct {
    int id;
    int employeeId;
    char date[MAX_DATE_LENGTH];
    int status;
} AttendanceRecord;

typedef struct {
    int id;
    int employeeId;
    char month[MAX_DATE_LENGTH];
    double salary;
    double deductions;
    double netPay;
} PayrollRecord;

typedef struct {
    int id;
    int employeeId;
    char startDate[MAX_DATE_LENGTH];
    char endDate[MAX_DATE_LENGTH];
    char reason[MAX_REASON_LENGTH];
    int status;
} LeaveRequest;

typedef struct {
    int id;
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    int roleId;
    int employeeId;
    int active;
} AccessAccount;

typedef struct {
    int id;
    int employeeId;
    char projectName[MAX_PROJECT_NAME_LENGTH];
    char orientationDate[MAX_DATE_LENGTH];
    int completed;
} ProjectOrientation;

typedef struct {
    Employee employees[MAX_EMPLOYEES];
    int employeeCount;

    Department departments[MAX_DEPARTMENTS];
    int departmentCount;

    Role roles[MAX_ROLES];
    int roleCount;

    AttendanceRecord attendance[MAX_ATTENDANCE];
    int attendanceCount;

    PayrollRecord payroll[MAX_PAYROLL];
    int payrollCount;

    LeaveRequest leaves[MAX_LEAVES];
    int leaveCount;

    AccessAccount accounts[MAX_ACCOUNTS];
    int accountCount;

    ProjectOrientation projects[MAX_PROJECTS];
    int projectCount;
} EMSData;

int copyStringSafe(char *dest, size_t destSize, const char *src);
void readText(const char *prompt, char *buffer, size_t size);
void readValidatedText(const char *prompt, char *buffer, size_t size, int (*validator)(const char *), const char *formatHint);
int readInt(const char *prompt);
int readValidatedInt(const char *prompt, int minValue, int maxValue, const char *formatHint);
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
void saveAll(EMSData *data);
void loadAll(EMSData *data);
void printMenu(void);
void showRoleModuleMenu(EMSData *data, const char *roleLabel);
void showModuleMenu(EMSData *data, const char *title, int (*addFunc)(EMSData *), void (*listFunc)(const EMSData *));
int addEmployee(EMSData *data);
void listEmployees(const EMSData *data);
void findEmployeeByNameOrId(const EMSData *data);
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
int employeeLogin(EMSData *data);
int hrLogin(EMSData *data);
int addProjectOrientation(EMSData *data);
void listProjectOrientations(const EMSData *data);
void showReportingDashboard(const EMSData *data);

#endif
