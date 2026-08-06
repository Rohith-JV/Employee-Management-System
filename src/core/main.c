#include "ems.h"

static void shutdownSystem(EMSData *data) {
    stopAutosave(data);
    saveAll(data);
    ems_mutex_destroy(&data->mutex);
    ems_condition_destroy(&data->saveRequested);
}

int main(void) {
    EMSData data;
    initializeData(&data);
    if (!startAutosave(&data)) {
        fprintf(stderr, "Warning: autosave thread could not be started.\n");
    }

    int32_t choice = 0;
    int32_t employeeId = 0;
    while (choice != 3) {
        printMenu();
        choice = readInt("");
        if (choice == EMS_INPUT_EOF) {
            shutdownSystem(&data);
            printf("\nInput closed. Data saved. Exiting.\n");
            return 0;
        }

        switch (choice) {
            case 1:
                lockData(&data);
                if (employeeLogin(&data, &employeeId)) {
                    unlockData(&data);
                    showRoleModuleMenu(&data, "Employee", employeeId);
                } else {
                    unlockData(&data);
                }
                break;
            case 2:
                lockData(&data);
                if (hrLogin(&data)) {
                    unlockData(&data);
                    showRoleModuleMenu(&data, "HR", 0);
                } else {
                    unlockData(&data);
                }
                break;
            case 3:
                shutdownSystem(&data);
                printf("Data saved successfully. Exiting.\n");
                break;
            default:
                printf("Invalid choice. Try again.\n");
                break;
        }
    }

    return 0;
}
