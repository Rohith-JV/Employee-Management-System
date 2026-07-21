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
gcc -std=c99 -Wall -Wextra -o ems src/main.c src/ems.c
```

## Run
```bash
./ems
```
