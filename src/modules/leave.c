#include "ems.h"

static int32_t nextLeaveId(const EMSData *data) {
    int32_t highest = 0;
    for (int32_t i = 0; i < data->leaveCount; ++i) {
        if (data->leaves[i].id > highest) {
            highest = data->leaves[i].id;
        }
    }
    return highest + 1;
}

int addLeaveRequest(EMSData *data) {
    if (data->leaveCount >= MAX_LEAVES) {
        printf("Leave storage is full.\n");
        return 0;
    }

    LeaveRequest *leave = &data->leaves[data->leaveCount];
    memset(leave, 0, sizeof(*leave));

    leave->id = nextLeaveId(data);
    leave->employeeId = readValidatedInt("Employee ID: ", 1, 100000, "positive integer");
    if (!employeeExists(data, leave->employeeId)) {
        printf("Employee ID does not exist or is inactive.\n");
        return 0;
    }
    readValidatedText("Start date (YYYY-MM-DD): ", leave->startDate, sizeof(leave->startDate), isDate, "YYYY-MM-DD");
    readValidatedText("End date (YYYY-MM-DD): ", leave->endDate, sizeof(leave->endDate), isDate, "YYYY-MM-DD");
    readValidatedText("Reason: ", leave->reason, sizeof(leave->reason), isTextWithSpaces, "letters, numbers, and spaces");
    leave->status = readValidatedInt("Status (0=Pending, 1=Approved, 2=Rejected): ", 0, 2, "0, 1, or 2");

    data->leaveCount++;
    printf("Leave request recorded.\n");
    return 1;
}

void listLeaveRequests(const EMSData *data) {
    if (data->leaveCount == 0) {
        printf("No leave requests found.\n");
        return;
    }

    printf("\nLeave requests:\n");
    for (int32_t i = 0; i < data->leaveCount; ++i) {
        const LeaveRequest *leave = &data->leaves[i];
        printf("%d. Employee %d | %s to %s | %s | Status: %d\n",
               leave->id,
               leave->employeeId,
               leave->startDate,
               leave->endDate,
               leave->reason,
               leave->status);
    }
}
