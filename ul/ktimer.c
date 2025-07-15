#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define DEVICE_PATH "/dev/mytimer"
#define BUFFER_SIZE 1024
#define INPUT_SIZE 256

// Function to check if a string consists of only digits
bool isNumber(const char* str) {
    for (int i = 0; i < strlen(str); i++) {
        if (!isdigit(str[i])) {
            return false;
        }
    }
    return true;
}

// Function to read output from the timer module
void readFromTimer(FILE* file) {
    char buffer[BUFFER_SIZE] = {0};

    rewind(file);
    fread(buffer, 1, BUFFER_SIZE, file);
    printf("%s", buffer);
}

// Function to interact with the timer module
void sendCommand(const char* command) {
    FILE* file = fopen(DEVICE_PATH, "r+");
    if (!file) {
        perror("Failed to open device file");
        return;
    }

    fprintf(file, "%s\n", command);
    fflush(file);
    readFromTimer(file);

    fclose(file);
}

int main(int argc, char** argv) {
    char input[INPUT_SIZE] = {0};
    
    if (strcmp(argv[1], "-l") == 0 && argc == 2) {
        sendCommand("-l");
    } 
    else if (strcmp(argv[1], "-m") == 0 && argc == 3 && isNumber(argv[2])) {
        snprintf(input, INPUT_SIZE, "-m %s", argv[2]);
        sendCommand(input);
    } 
    else if (strcmp(argv[1], "-s") == 0 && argc == 4 && isNumber(argv[2])) {
        snprintf(input, INPUT_SIZE, "-s %s %s", argv[2], argv[3]);
        sendCommand(input);
    } 

    return 0;
}

