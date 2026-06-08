#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DEVICES 8
#define HOURS 24
#define NAME_SIZE 50

// 상태 플래그
#define FLAG_POWER_ON   0x01  // bit0: 전원 ON
#define FLAG_OVERLOAD   0x02  // bit1: 과부하 경고
#define FLAG_SAVE_POWER 0x04  // bit2: 절전 권고
#define FLAG_ERROR      0x08  // bit3: 측정 오류

typedef struct {
    char name[NAME_SIZE];
    char location[NAME_SIZE];

    double ratedPower;     // 정격전력 W
    double voltage;        // 전압 V
    double current;        // 전류 A
    double power;          // 소비전력 W
    double runningHours;   // 사용 시간 h
    double energyKWh;      // 누적 전력량 kWh

    unsigned char status;  // 상태 플래그
} Device;

void removeNewline(char str[]);
void inputString(const char* message, char str[], int size);
int inputIntRange(const char* message, int min, int max);
double inputDoubleMin(const char* message, double min);

void inputDevice(Device* dev, int index);
void analyzeDevice(Device* dev);
void makePowerLog(Device devices[], double powerLog[MAX_DEVICES][HOURS], int count);
double getAveragePower(double powerLog[MAX_DEVICES][HOURS], int index);
double getMaxPower(double powerLog[MAX_DEVICES][HOURS], int index);

void printStatus(unsigned char status);
void printDashboard(Device devices[], double powerLog[MAX_DEVICES][HOURS], int count);
void saveReport(Device devices[], double powerLog[MAX_DEVICES][HOURS], int count);

int main(void) {
    Device devices[MAX_DEVICES];
    double powerLog[MAX_DEVICES][HOURS] = { 0 };

    int count = 0;
    int menu;

    while (1) {
        printf("\n========================================\n");
        printf(" Electronic Device Power Monitor System\n");
        printf("========================================\n");
        printf("1. Input device data\n");
        printf("2. Print dashboard\n");
        printf("3. Save report files\n");
        printf("4. Reset data\n");
        printf("0. Exit\n");

        menu = inputIntRange("Select menu: ", 0, 4);

        if (menu == 1) {
            count = inputIntRange("How many devices? (1~8): ", 1, MAX_DEVICES);

            for (int i = 0; i < count; i++) {
                inputDevice(&devices[i], i + 1);
                analyzeDevice(&devices[i]);
            }

            makePowerLog(devices, powerLog, count);
            printf("\nDevice data input completed.\n");
        }
        else if (menu == 2) {
            if (count == 0) {
                printf("\nNo data. Please input device data first.\n");
            }
            else {
                printDashboard(devices, powerLog, count);
            }
        }
        else if (menu == 3) {
            if (count == 0) {
                printf("\nNo data. Please input device data first.\n");
            }
            else {
                saveReport(devices, powerLog, count);
            }
        }
        else if (menu == 4) {
            count = 0;
            memset(devices, 0, sizeof(devices));
            memset(powerLog, 0, sizeof(powerLog));
            printf("\nAll data has been reset.\n");
        }
        else if (menu == 0) {
            printf("\nProgram ended.\n");
            break;
        }
    }

    return 0;
}

void removeNewline(char str[]) {
    str[strcspn(str, "\n")] = '\0';
}

void inputString(const char* message, char str[], int size) {
    printf("%s", message);
    fgets(str, size, stdin);
    removeNewline(str);
}

int inputIntRange(const char* message, int min, int max) {
    char buffer[100];
    int value;

    while (1) {
        printf("%s", message);
        fgets(buffer, sizeof(buffer), stdin);

        if (sscanf(buffer, "%d", &value) == 1 && value >= min && value <= max) {
            return value;
        }

        printf("Invalid input. Please enter a number between %d and %d.\n", min, max);
    }
}

double inputDoubleMin(const char* message, double min) {
    char buffer[100];
    double value;

    while (1) {
        printf("%s", message);
        fgets(buffer, sizeof(buffer), stdin);

        if (sscanf(buffer, "%lf", &value) == 1 && value >= min) {
            return value;
        }

        printf("Invalid input. Please enter a number greater than or equal to %.2f.\n", min);
    }
}

void inputDevice(Device* dev, int index) {
    printf("\n----- Device %d -----\n", index);

    inputString("Device name: ", dev->name, NAME_SIZE);
    inputString("Location: ", dev->location, NAME_SIZE);

    dev->ratedPower = inputDoubleMin("Rated power(W): ", 0.1);
    dev->voltage = inputDoubleMin("Voltage(V): ", 0.0);
    dev->current = inputDoubleMin("Current(A): ", 0.0);
    dev->runningHours = inputDoubleMin("Running hours per day(h): ", 0.0);

    if (dev->runningHours > 24.0) {
        printf("Running hours cannot exceed 24. It will be changed to 24.\n");
        dev->runningHours = 24.0;
    }

    dev->power = dev->voltage * dev->current;
    dev->energyKWh = dev->power * dev->runningHours / 1000.0;
    dev->status = 0;
}

void analyzeDevice(Device* dev) {
    dev->status = 0;

    if (dev->voltage > 0 && dev->current > 0 && dev->runningHours > 0) {
        dev->status |= FLAG_POWER_ON;
    }

    if (dev->power > dev->ratedPower * 1.1) {
        dev->status |= FLAG_OVERLOAD;
    }

    if (dev->runningHours >= 8.0 || dev->energyKWh >= 1.0) {
        dev->status |= FLAG_SAVE_POWER;
    }

    if (dev->power > dev->ratedPower * 2.0) {
        dev->status |= FLAG_ERROR;
    }
}

void makePowerLog(Device devices[], double powerLog[MAX_DEVICES][HOURS], int count) {
    for (int i = 0; i < count; i++) {
        double remainHours = devices[i].runningHours;

        for (int h = 0; h < HOURS; h++) {
            if (remainHours >= 1.0) {
                powerLog[i][h] = devices[i].power;
                remainHours -= 1.0;
            }
            else if (remainHours > 0.0) {
                powerLog[i][h] = devices[i].power * remainHours;
                remainHours = 0.0;
            }
            else {
                powerLog[i][h] = 0.0;
            }
        }
    }
}

double getAveragePower(double powerLog[MAX_DEVICES][HOURS], int index) {
    double sum = 0.0;

    for (int h = 0; h < HOURS; h++) {
        sum += powerLog[index][h];
    }

    return sum / HOURS;
}

double getMaxPower(double powerLog[MAX_DEVICES][HOURS], int index) {
    double max = powerLog[index][0];

    for (int h = 1; h < HOURS; h++) {
        if (powerLog[index][h] > max) {
            max = powerLog[index][h];
        }
    }

    return max;
}

void printStatus(unsigned char status) {
    if (status & FLAG_POWER_ON) {
        printf("ON ");
    }
    else {
        printf("OFF ");
    }

    if (status & FLAG_OVERLOAD) {
        printf("| OVERLOAD ");
    }

    if (status & FLAG_SAVE_POWER) {
        printf("| SAVE_POWER ");
    }

    if (status & FLAG_ERROR) {
        printf("| ERROR ");
    }
}

void printDashboard(Device devices[], double powerLog[MAX_DEVICES][HOURS], int count) {
    printf("\n================ POWER DASHBOARD ================\n");
    printf("%-3s %-12s %-12s %-10s %-10s %-10s %-10s %-20s\n",
        "No", "Name", "Location", "Power(W)", "Avg(W)", "Max(W)", "kWh", "Status");

    for (int i = 0; i < count; i++) {
        printf("%-3d %-12s %-12s %-10.2f %-10.2f %-10.2f %-10.3f ",
            i + 1,
            devices[i].name,
            devices[i].location,
            devices[i].power,
            getAveragePower(powerLog, i),
            getMaxPower(powerLog, i),
            devices[i].energyKWh);

        printStatus(devices[i].status);
        printf("\n");
    }

    printf("=================================================\n");
}

void saveReport(Device devices[], double powerLog[MAX_DEVICES][HOURS], int count) {
    FILE* fp;
    FILE* alert;
    int hasAlert = 0;

    fp = fopen("power_log.csv", "w");

    if (fp == NULL) {
        printf("Failed to open power_log.csv\n");
        return;
    }

    fprintf(fp, "No,Name,Location,RatedPower(W),Voltage(V),Current(A),Power(W),RunningHours(h),Energy(kWh),AveragePower(W),MaxPower(W),Status\n");

    for (int i = 0; i < count; i++) {
        fprintf(fp, "%d,%s,%s,%.2f,%.2f,%.2f,%.2f,%.2f,%.3f,%.2f,%.2f,",
            i + 1,
            devices[i].name,
            devices[i].location,
            devices[i].ratedPower,
            devices[i].voltage,
            devices[i].current,
            devices[i].power,
            devices[i].runningHours,
            devices[i].energyKWh,
            getAveragePower(powerLog, i),
            getMaxPower(powerLog, i));

        if (devices[i].status & FLAG_POWER_ON) {
            fprintf(fp, "ON ");
        }
        else {
            fprintf(fp, "OFF ");
        }

        if (devices[i].status & FLAG_OVERLOAD) {
            fprintf(fp, "OVERLOAD ");
        }

        if (devices[i].status & FLAG_SAVE_POWER) {
            fprintf(fp, "SAVE_POWER ");
        }

        if (devices[i].status & FLAG_ERROR) {
            fprintf(fp, "ERROR ");
        }

        fprintf(fp, "\n");
    }

    fclose(fp);

    alert = fopen("alert_report.txt", "w");

    if (alert == NULL) {
        printf("Failed to open alert_report.txt\n");
        return;
    }

    fprintf(alert, "Power Alert Report\n");
    fprintf(alert, "==================\n\n");

    for (int i = 0; i < count; i++) {
        if ((devices[i].status & FLAG_OVERLOAD) ||
            (devices[i].status & FLAG_SAVE_POWER) ||
            (devices[i].status & FLAG_ERROR)) {

            hasAlert = 1;

            fprintf(alert, "[%s / %s]\n", devices[i].name, devices[i].location);
            fprintf(alert, "Power: %.2f W\n", devices[i].power);
            fprintf(alert, "Rated Power: %.2f W\n", devices[i].ratedPower);
            fprintf(alert, "Energy: %.3f kWh\n", devices[i].energyKWh);

            if (devices[i].status & FLAG_OVERLOAD) {
                fprintf(alert, "- Warning: Overload detected.\n");
            }

            if (devices[i].status & FLAG_SAVE_POWER) {
                fprintf(alert, "- Recommendation: Power saving is needed.\n");
            }

            if (devices[i].status & FLAG_ERROR) {
                fprintf(alert, "- Error: Measurement value may be abnormal.\n");
            }

            fprintf(alert, "\n");
        }
    }

    if (!hasAlert) {
        fprintf(alert, "No overload or power-saving warning detected.\n");
    }

    fclose(alert);

    printf("\nFiles saved successfully.\n");
    printf("- power_log.csv\n");
    printf("- alert_report.txt\n");
}
