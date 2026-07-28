CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -I src
TARGET = ems
SRC = src/core/main.c src/core/ems.c src/modules/employee.c src/modules/department.c src/modules/role.c src/modules/attendance.c src/modules/payroll.c src/modules/leave.c src/modules/access.c src/modules/project.c src/modules/report.c

all:
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
