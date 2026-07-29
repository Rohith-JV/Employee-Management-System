#include <stdio.h>
#include <stdlib.h>
#include <CUnit/Basic.h>
#include "../src/ems.h"

EMSData *test_data = NULL;

/* ==========================================
 * SUITE 1: Payroll Tests
 * ========================================== */
int init_suite_payroll(void) {
    test_data = (EMSData *)malloc(sizeof(EMSData));
    if (test_data == NULL) return -1;
    
    // Ensure the payroll count starts at 0 for our tests
    test_data->payrollCount = 0;
    return 0;
}

int clean_suite_payroll(void) {
    if (test_data != NULL) {
        free(test_data);
        test_data = NULL;
    }
    return 0;
}

void test_addPayroll(void) {
    // 1. Create a mock input file with the EXACT 4 inputs in sequence
    FILE *mock_input = fopen("mock_input.txt", "w");
    
    // Expected order: Employee ID, Month (YYYY-MM), Gross Salary, Deductions
    fprintf(mock_input, "101\n2023-10\n50000.0\n2000.0\n"); 
    fclose(mock_input);

    // 2. Redirect stdin to read from our mock file
    freopen("mock_input.txt", "r", stdin);

    // 3. Call the function
    int result = addPayroll(test_data);
    
    // 4. Assert the expected return value and check if count increased
    CU_ASSERT_EQUAL(result, 1); 
    CU_ASSERT_EQUAL(test_data->payrollCount, 1);

    // 5. Restore stdin and clean up
    freopen("/dev/tty", "r", stdin);
    remove("mock_input.txt");
}

void test_listPayroll(void) {
    // listPayroll only reads from the data struct and prints to stdout,
    // so it does NOT need standard input redirection.
    
    listPayroll(test_data);
    
    CU_PASS("listPayroll executed without crashing.");
}

/* ==========================================
 * SUITE 2: Core EMS Tests (ems.c)
 * ========================================== */
int init_suite_core(void) {
    // Setup specific to core tests if needed
    return 0;
}

int clean_suite_core(void) {
    // Cleanup specific to core tests
    return 0;
}

/* Test Case 1: System Initialization/Loading */
void test_loadData(void) {
    // Replace 'loadData' with the actual function name in ems.c that reads from files
    // Example: 
    // EMSData *data = loadData();
    // CU_ASSERT_PTR_NOT_NULL(data); // Verifies the pointer is not NULL
    
    CU_PASS("Placeholder for loadData test.");
}

/* Test Case 2: System Saving */
void test_saveData(void) {
    // Replace 'saveData' with the actual function name in ems.c that writes to files
    // Example:
    // int result = saveData(test_data);
    // CU_ASSERT_EQUAL(result, 1); // Verifies the save was successful
    
    CU_PASS("Placeholder for saveData test.");
}

/* ==========================================
 * MAIN RUNNER
 * ========================================== */
int main() {
    CU_pSuite pSuitePayroll = NULL;
    CU_pSuite pSuiteCore = NULL;

    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    // 1. Register Payroll Suite
    pSuitePayroll = CU_add_suite("Payroll_Test_Suite", init_suite_payroll, clean_suite_payroll);
    if (NULL == pSuitePayroll) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuitePayroll, "test of addPayroll()", test_addPayroll)) ||
        (NULL == CU_add_test(pSuitePayroll, "test of listPayroll()", test_listPayroll))) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    // 2. Register Core EMS Suite
    pSuiteCore = CU_add_suite("Core_EMS_Test_Suite", init_suite_core, clean_suite_core);
    if (NULL == pSuiteCore) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuiteCore, "test of loadData()", test_loadData)) ||
        (NULL == CU_add_test(pSuiteCore, "test of saveData()", test_saveData))) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    // Run all tests
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}