CC = gcc
ifeq ($(OS),Windows_NT)
THREAD_FLAGS =
else
THREAD_FLAGS = -pthread
endif

CFLAGS = -std=c99 -Wall -Wextra -I include -I src $(THREAD_FLAGS)
TARGET = ems

# Original source files for the main application
SRC = src/core/main.c src/core/ems.c src/modules/employee.c src/modules/employee_user.c src/modules/department.c src/modules/role.c src/modules/attendance.c src/modules/payroll.c src/modules/leave.c src/modules/access.c src/modules/project.c src/modules/report.c

TEST_TARGET = test_ems
TEST_SRC = tests/test_main.c tests/test_helpers.c tests/test_core.c tests/test_employee.c tests/test_department.c tests/test_role.c tests/test_attendance.c tests/test_payroll.c tests/test_leave.c tests/test_access.c tests/test_project.c tests/test_report.c tests/test_employee_user.c tests/test_ems.c tests/test_populate.c tests/test_integration.c src/core/ems.c src/modules/employee.c src/modules/employee_user.c src/modules/department.c src/modules/role.c src/modules/attendance.c src/modules/payroll.c src/modules/leave.c src/modules/access.c src/modules/project.c src/modules/report.c

# Default build target
all:
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Compile the test executable
test:
	$(CC) $(CFLAGS) -o $(TEST_TARGET) $(TEST_SRC)

# Compile and immediately run the tests
run-tests: test
	./$(TEST_TARGET)

# Clean both the main executable and the test executable
clean:
	rm -f $(TARGET) $(TEST_TARGET)
	rm -rf build

# --- Static Analysis ---
# --enable=all turns on all warnings, style, performance, and portability checks.
# --suppress=missingIncludeSystem prevents it from complaining about standard C libraries like <stdio.h>.
cppcheck:
	cppcheck --enable=all --suppress=missingIncludeSystem --inconclusive --std=c99 -I include/ -I src/ src/ tests/
