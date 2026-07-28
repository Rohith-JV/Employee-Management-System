#include "ems.h"

int main(void) {
    EMSData data;
    initializeData(&data);

    int choice = 0;
    while (choice != 3) {
        printMenu();
        choice = readInt("Enter choice: ");

        switch (choice) {
            case 1:
                if (employeeLogin(&data)) {
                    showRoleModuleMenu(&data, "Employee");
                }
                break;
            case 2:
                if (hrLogin(&data)) {
                    showRoleModuleMenu(&data, "HR");
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
