# Employee Management System

A console-based C application for managing employee information, departments, roles, attendance, leave, access, payroll, project orientation, and reporting.

## Project structure
- src/core: shared runtime, data loading/saving, and main entry point
- src/modules: feature-specific modules for each EMS domain

## Features
- Employee onboarding and employee record management
- Department management
- Role management
- Attendance management
- Leave management
- Login and access management
- Payroll processing
- Project orientation tracking
- Reporting dashboard
- Persistent storage in text files under the data directory

## Build
```bash
gcc -std=c99 -Wall -Wextra -I src -o ems src/core/main.c src/core/ems.c src/modules/employee.c src/modules/department.c src/modules/role.c src/modules/attendance.c src/modules/payroll.c src/modules/leave.c src/modules/access.c src/modules/project.c src/modules/report.c
```

## Run
```bash
./ems
```
