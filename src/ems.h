#ifndef EMS_H
#define EMS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_EMPLOYEES 100
#define MAX_DEPARTMENTS 50
#define MAX_ROLES 50
#define MAX_ATTENDANCE 500
#define MAX_PAYROLL 500
#define MAX_LEAVES 500
#define MAX_ACCOUNTS 200
#define MAX_PROJECTS 300

typedef struct {
    int id;
    char name[50];
    int departmentId;
    int roleId;
    int salary;
    char email[50];
    char phone[50];
    int active;
    char joinDate[12];
    char status[20];
} Employee;

typedef struct {
    int id;
    char name[50];
    char head[50];
} Department;

typedef struct {
    int id;
    char title[50];
    char description[50];
} Role;

typedef struct {
    int id;
    int employeeId;
    char date[12];
    int status;
} AttendanceRecord;

typedef struct {
    int id;
    int employeeId;
    char month[12];
    double salary;
    double deductions;
    double netPay;
} PayrollRecord;

typedef struct {
    int id;
    int employeeId;
    char startDate[12];
    char endDate[12];
    char reason[100];
    int status;
} LeaveRequest;

typedef struct {
    int id;
    char username[32];
    char password[32];
    int roleId;
    int employeeId;
    int active;
} AccessAccount;

typedef struct {
    int id;
    int employeeId;
    char projectName[50];
    char orientationDate[12];
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
void showModuleMenu(EMSData *data, const char *title, int (*addFunc)(EMSData *), void (*listFunc)(const EMSData *));
int addEmployee(EMSData *data);
void listEmployees(const EMSData *data);
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
