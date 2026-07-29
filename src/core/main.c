#include "ems.h"

int main(void) {
    EMSData data;
    initializeData(&data);

    int32_t choice = 0;
    int32_t employeeId = 0;
    while (choice != 3) {
        printMenu();
        choice = readInt("Enter choice: ");

        switch (choice) {
            case 1:
                if (employeeLogin(&data, &employeeId)) {
                    showRoleModuleMenu(&data, "Employee", employeeId);
                }
                break;
            case 2:
                if (hrLogin(&data)) {
                    showRoleModuleMenu(&data, "HR", 0);
                }
                break;
            case 3:
                saveAll(&data);
                printf("Data saved successfully. Exiting.\n");
                break;
            default:
                printf("Invalid choice. Try again.\n");
                break;
        }
    }

    return 0;
}
