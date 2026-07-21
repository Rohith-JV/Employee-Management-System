#include "ems.h"

static int nextPayrollId(const EMSData *data) {
    int highest = 0;
    for (int i = 0; i < data->payrollCount; ++i) {
        if (data->payroll[i].id > highest) {
            highest = data->payroll[i].id;
        }
    }
    return highest + 1;
}

int addPayroll(EMSData *data) {
    if (data->payrollCount >= MAX_PAYROLL) {
        printf("Payroll storage is full.\n");
        return 0;
    }

    PayrollRecord *pay = &data->payroll[data->payrollCount];
    memset(pay, 0, sizeof(*pay));

    pay->id = nextPayrollId(data);
    pay->employeeId = readValidatedInt("Employee ID: ", 1, 100000, "positive integer");
    readValidatedText("Month (YYYY-MM): ", pay->month, sizeof(pay->month), isMonth, "YYYY-MM");
    pay->salary = readValidatedDouble("Gross Salary: ", 0.0, 1000000000.0, "positive decimal number");
    pay->deductions = readValidatedDouble("Deductions: ", 0.0, 1000000000.0, "positive decimal number");
    pay->netPay = pay->salary - pay->deductions;

    data->payrollCount++;
    printf("Payroll record added.\n");
    return 1;
}

void listPayroll(const EMSData *data) {
    if (data->payrollCount == 0) {
        printf("No payroll records found.\n");
        return;
    }

    printf("\nPayroll:\n");
    for (int i = 0; i < data->payrollCount; ++i) {
        const PayrollRecord *pay = &data->payroll[i];
        printf("%d. Employee %d | %s | Gross: %.2f | Deductions: %.2f | Net: %.2f\n",
               pay->id,
               pay->employeeId,
               pay->month,
               pay->salary,
               pay->deductions,
               pay->netPay);
    }
}
