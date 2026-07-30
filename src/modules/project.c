#include "ems.h"

static int32_t nextProjectId(const EMSData *data) {
    int32_t highest = 0;
    for (int32_t i = 0; i < data->projectCount; ++i) {
        if (data->projects[i].id > highest) {
            highest = data->projects[i].id;
        }
    }
    return highest + 1;
}

int addProjectOrientation(EMSData *data) {
    if (data->projectCount >= MAX_PROJECTS) {
        printf("Project storage is full.\n");
        return 0;
    }

    ProjectOrientation *project = &data->projects[data->projectCount];
    memset(project, 0, sizeof(*project));

    project->id = nextProjectId(data);
    project->employeeId = readValidatedInt("Employee ID: ", 1, 100000, "positive integer");
    if (!employeeExists(data, project->employeeId)) {
        printf("Employee ID does not exist or is inactive.\n");
        return 0;
    }
    readValidatedText("Project name: ", project->projectName, sizeof(project->projectName), isTextWithSpaces, "letters and spaces, e.g. Payroll System");
    readValidatedText("Orientation date (YYYY-MM-DD): ", project->orientationDate, sizeof(project->orientationDate), isDate, "YYYY-MM-DD");
    project->completed = (uint8_t)readValidatedInt("Completed (1/0): ", 0, 1, "0 or 1");

    data->projectCount++;
    printf("Project orientation entry added.\n");
    return 1;
}

void listProjectOrientations(const EMSData *data) {
    if (data->projectCount == 0) {
        printf("No project orientations found.\n");
        return;
    }

    printf("\nProject orientations:\n");
    for (int32_t i = 0; i < data->projectCount; ++i) {
        const ProjectOrientation *project = &data->projects[i];
        printf("%d. Employee %d | %s | %s | Completed: %s\n",
               project->id,
               project->employeeId,
               project->projectName,
               project->orientationDate,
               project->completed ? "Yes" : "No");
    }
}
