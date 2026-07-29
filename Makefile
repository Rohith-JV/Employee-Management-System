CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -I src
TARGET = ems

# Original source files for the main application
SRC = src/core/main.c src/core/ems.c src/modules/employee.c src/modules/department.c src/modules/role.c src/modules/attendance.c src/modules/payroll.c src/modules/leave.c src/modules/access.c src/modules/project.c src/modules/report.c

# --- CUnit Testing Setup ---
TEST_TARGET = test_ems

# Exclude src/core/main.c to prevent multiple main() function errors, 
# and include the new test file (tests/test_ems.c)
TEST_SRC = tests/test_ems.c src/core/ems.c src/modules/employee.c src/modules/department.c src/modules/role.c src/modules/attendance.c src/modules/payroll.c src/modules/leave.c src/modules/access.c src/modules/project.c src/modules/report.c

# Append CUnit include path to your existing CFLAGS
TEST_CFLAGS = $(CFLAGS) -I/usr/include/CUnit

# Linker flags for CUnit
TEST_LDFLAGS = -lcunit

# Default build target
all:
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Compile the test executable
test:
	$(CC) $(TEST_CFLAGS) -o $(TEST_TARGET) $(TEST_SRC) $(TEST_LDFLAGS)

# Compile and immediately run the tests
run-tests: test
	./$(TEST_TARGET)

# Clean both the main executable and the test executable
clean:
	rm -f $(TARGET) $(TEST_TARGET)

# --- Static Analysis ---
# --enable=all turns on all warnings, style, performance, and portability checks.
# --suppress=missingIncludeSystem prevents it from complaining about standard C libraries like <stdio.h>.
cppcheck:
	cppcheck --enable=all --suppress=missingIncludeSystem --inconclusive --std=c99 -I src/ src/ tests/