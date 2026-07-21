#include "ems.h"

static int nextAttendanceId(const EMSData *data) {
    int highest = 0;
    for (int i = 0; i < data->attendanceCount; ++i) {
        if (data->attendance[i].id > highest) {
            highest = data->attendance[i].id;
        }
    }
    return highest + 1;
}

int addAttendance(EMSData *data) {
    if (data->attendanceCount >= MAX_ATTENDANCE) {
        printf("Attendance storage is full.\n");
        return 0;
    }

    AttendanceRecord *record = &data->attendance[data->attendanceCount];
    memset(record, 0, sizeof(*record));

    record->id = nextAttendanceId(data);
    record->employeeId = readValidatedInt("Employee ID: ", 1, 100000, "positive integer");
    readValidatedText("Date (YYYY-MM-DD): ", record->date, sizeof(record->date), isDate, "YYYY-MM-DD");
    record->status = readValidatedInt("Status (1=Present, 0=Absent): ", 0, 1, "0 or 1");

    data->attendanceCount++;
    printf("Attendance recorded.\n");
    return 1;
}

void listAttendance(const EMSData *data) {
    if (data->attendanceCount == 0) {
        printf("No attendance records found.\n");
        return;
    }

    printf("\nAttendance:\n");
    for (int i = 0; i < data->attendanceCount; ++i) {
        const AttendanceRecord *record = &data->attendance[i];
        printf("%d. Employee %d | %s | %s\n",
               record->id,
               record->employeeId,
               record->date,
               record->status ? "Present" : "Absent");
    }
}
