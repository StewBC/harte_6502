// #include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "6502.h"
#include "cJSON.h"
#include "log.h"

#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
#else
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fnmatch.h>
#endif

#define SetBit(x)       (1<<(x))
const char *flag_names = "CZIDB-VN";

enum RESULT_CODES {
    RESULT_PREP_ERROR,
    RESULT_SUCCESS,
    RESULT_FAIL,
    RESULT_FILTERED_OUT
};
typedef enum RESULT_CODES RESULT_CODES;

// Verification and DEBUG
enum FAIL_STATE {
    PC     = SetBit(1),
    SP     = SetBit(2),
    A      = SetBit(3),
    Y      = SetBit(4),
    X      = SetBit(5),
    P      = SetBit(6),
    MEMORY = SetBit(7),
};
typedef enum FAIL_STATE FAIL_STATE;

struct STATE {
    uint16_t PC;
    uint16_t SP;
    uint8_t  A;
    uint8_t  Y;
    uint8_t  X;
    uint8_t  P;
    cJSON   *RAM;
} ;
typedef struct STATE STATE;

int strrtok(const char *str) {
    int n = strlen(str);
    while(n >= 0 && str[n] != '/' && str[n] != '\\') {
        n--;
    }
    return n >= 0 ? n + 1 : 0;
}

void failureLog(FAIL_STATE *state_accumulator, FAIL_STATE failedState, MACHINE *m, STATE *state) {
    if(failedState & PC) {
        logToStr(CHAN_FAIL, 0, "APC:%04X EPC:%04X ", m->cpu.pc, state->PC);
    }
    if(failedState & SP) {
        logToStr(CHAN_FAIL, 0, "ASP:%04X ESP:%04X ", m->cpu.sp, 0x100 + state->SP);
    }
    if(failedState & A) {
        logToStr(CHAN_FAIL, 0, "AA:%02X EA:%02X ", m->cpu.A, state->A);
    }
    if(failedState & Y) {
        logToStr(CHAN_FAIL, 0, "AY:%02X EY:%02X ", m->cpu.Y, state->Y);
    }
    if(failedState & X) {
        logToStr(CHAN_FAIL, 0, "AX:%02X EX:%02X ", m->cpu.X, state->X);
    }
    if(failedState & P) {
        int i;
        for(i = 7; i >= 0; i--) {
            if((m->cpu.flags & SetBit(i)) != (state->P & SetBit(i))) {
                logToStr(CHAN_FAIL, 0, "%c", flag_names[i]);
            }
        }
        logToStr(CHAN_FAIL, 0, " AP:");
        logToStrBinaryBits(CHAN_FAIL, m->cpu.flags, 8);
        logToStr(CHAN_FAIL, 0, " EP:");
        logToStrBinaryBits(CHAN_FAIL, state->P, 8);
    }

    *state_accumulator |= failedState;
}

int loadStateFromJSON(cJSON *jsonTest, const char *testStateName, STATE *state) {
    cJSON *testState = cJSON_GetObjectItemCaseSensitive(jsonTest, testStateName);
    if(!testState) {
        return RESULT_PREP_ERROR;
    }
    double pc = cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(testState, "pc"));
    double sp = cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(testState, "s"));
    double a = cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(testState, "a"));
    double y = cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(testState, "y"));
    double x = cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(testState, "x"));
    double p = cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(testState, "p"));
    cJSON *ram = cJSON_GetObjectItemCaseSensitive(testState, "ram");
    if(isnan(pc) | isnan(sp) | isnan(a) | isnan(x) | isnan(y) | isnan(p) | !cJSON_IsArray(ram)) {
        return RESULT_PREP_ERROR;
    }
    state->PC = (uint16_t)pc;
    state->SP = (uint16_t)sp;
    state->A = (uint8_t)a;
    state->Y = (uint8_t)y;
    state->X = (uint8_t)x;
    state->P = (uint8_t)p;
    state->RAM = ram;
    return RESULT_SUCCESS;;
}

int setRamFromJSON(MACHINE *m, cJSON *ram) {
    cJSON *address_pair = NULL;
    cJSON_ArrayForEach(address_pair, ram) {
        if(!cJSON_IsArray(address_pair) || 2 != cJSON_GetArraySize(address_pair)) {
            return RESULT_PREP_ERROR;
        }
        double address = cJSON_GetNumberValue(cJSON_GetArrayItem(address_pair, 0));
        double byteValue = cJSON_GetNumberValue(cJSON_GetArrayItem(address_pair, 1));
        if(isnan(address) || isnan(byteValue)) {
            return RESULT_PREP_ERROR;
        }
        write_to_memory(m, address, byteValue);
    }
    return RESULT_SUCCESS;;
}

int setMachineState(MACHINE *m, STATE *state) {
    m->cpu.pc = state->PC;
    m->cpu.sp = 0x100 + state->SP;
    m->cpu.A = state->A;
    m->cpu.Y = state->Y;
    m->cpu.X = state->X;
    m->cpu.flags = state->P;
    m->cpu.instruction_cycle = -1;
    return setRamFromJSON(m, state->RAM);
}

int verifyRamFromJSON(MACHINE *m, FAIL_STATE *failState, cJSON *ram) {
    cJSON *address_pair = NULL;
    cJSON_ArrayForEach(address_pair, ram) {
        if(!cJSON_IsArray(address_pair) || 2 != cJSON_GetArraySize(address_pair)) {
            return RESULT_PREP_ERROR;
        }
        double address = cJSON_GetNumberValue(cJSON_GetArrayItem(address_pair, 0));
        double byteValue = cJSON_GetNumberValue(cJSON_GetArrayItem(address_pair, 1));
        if(isnan(address) || isnan(byteValue)) {
            return RESULT_PREP_ERROR;
        }
        if(byteValue != read_from_memory(m, address)) {
            *failState |= MEMORY;
            return RESULT_FAIL;
        }
    }
    return RESULT_SUCCESS;;
}

int verifyState(MACHINE *m, FAIL_STATE *failState, STATE *state, STATE *checkState) {
    *failState = 0;

    if(!checkState || checkState->PC) {
        if(state->PC != m->cpu.pc) {
            failureLog(failState, PC, m, state);
        }
    }
    if(!checkState || checkState->SP) {
        if((0x100 + state->SP) != m->cpu.sp) {
            failureLog(failState, SP, m, state);
        }
    }
    if(!checkState || checkState->A) {
        if(state->A != m->cpu.A) {
            failureLog(failState, A, m, state);
        }
    }
    if(!checkState || checkState->Y) {
        if(state->Y != m->cpu.Y) {
            failureLog(failState, Y, m, state);
        }
    }
    if(!checkState || checkState->X) {
        if(state->X != m->cpu.X) {
            failureLog(failState, X, m, state);
        }
    }
    if(!checkState || checkState->P) {
        if(state->P != m->cpu.flags) {
            failureLog(failState, P, m, state);
        }
    }
    if(!checkState || checkState->RAM) {
        int ramState = verifyRamFromJSON(m, failState, state->RAM);
        if(!failState && ramState != RESULT_SUCCESS) {
            return ramState;
        }
    }
    return !(*failState) ? RESULT_SUCCESS : RESULT_FAIL;
}

// The machine and its components
MACHINE m;
uint8_t RAM_MAIN[RAM_SIZE];
uint8_t RAM_IO[RAM_SIZE];

// Global variables to keep track of tests ran, passed, etc.
int testNumber = 0;
int testsFailed = 0;
int testsPassed = 0;
int testsSkipped = 0;

// Global variables used for comparing test results
double expected_address;
double actual_address;
double expected_value;
double actual_value;
const char *expected_action;
char actual_action;

// Note the actual program counter and value read from RAM on a read
uint8_t io_read_callback(MACHINE *m, uint16_t address) {
    actual_address = address;
    actual_value = m->read_pages.pages[address / PAGE_SIZE].memory[address % PAGE_SIZE];
    actual_action = 'r';
    return actual_value;
}

// Note the actual program counter and value written to RAM on a write
void io_write_callback(MACHINE *m, uint16_t address, uint8_t value) {
    uint16_t page = address / PAGE_SIZE;
    uint16_t offset = address % PAGE_SIZE;
    actual_address = address;
    actual_value = value;
    actual_action = 'w';
    m->write_pages.pages[page].memory[offset] = value;
}

// Create the test machine configuration
int configure_Harte(MACHINE *m, opcode_steps **cpu_type) {
    uint8_t *file_data;

    opcodes = cpu_type;

    // RAM
    if(!ram_init(&m->ram, 1)) {
        return RESULT_PREP_ERROR;
    }
    ram_add(&m->ram, 0, 0x0000, RAM_SIZE, RAM_MAIN);

    // PAGES
    if(!pages_init(&m->read_pages, RAM_SIZE / PAGE_SIZE)) {
        return RESULT_PREP_ERROR;
    }
    if(!pages_init(&m->write_pages, RAM_SIZE / PAGE_SIZE)) {
        return RESULT_PREP_ERROR;
    }
    if(!pages_init(&m->io_pages, RAM_SIZE / PAGE_SIZE)) {
        return RESULT_PREP_ERROR;
    }

    // Map main write ram
    pages_map(&m->write_pages, m->ram.ram_banks[0].address / PAGE_SIZE, m->ram.ram_banks[0].length / PAGE_SIZE, m->ram.ram_banks[0].memory);

    // Map read ram (same as write ram in this case)
    pages_map(&m->read_pages , m->ram.ram_banks[0].address / PAGE_SIZE, m->ram.ram_banks[0].length / PAGE_SIZE, m->ram.ram_banks[0].memory);

    // Install the IO callbacks
    m->io_write = io_write_callback;
    m->io_read = io_read_callback;

    // Map IO area checks (all IO, no 0's so all access traps to IO handler to capture test actuals)
    memset(RAM_IO, 0xff, RAM_SIZE);
    pages_map(&m->io_pages, 0, RAM_SIZE / PAGE_SIZE, RAM_IO);

    return RESULT_SUCCESS;;
}

// Configure the machine from the initial state and run the code for a single test
// At the end, report if the test succeeded and if not, where it failed
int runTest(cJSON *aTest, int testNumber, FAIL_STATE *failState) {
    const char *name = cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(aTest, "name"));
    if(name) {
        STATE initialState, finalState;

        logToStr(CHAN_TEST_INFO, 0, "Test %5d: %s ", testNumber, name);

        if(!loadStateFromJSON(aTest, "initial", &initialState)) {
            return RESULT_PREP_ERROR;
        }

        RESULT_CODES r;
        r = setMachineState(&m, &initialState);
        if(r != RESULT_SUCCESS) {
            return r;
        }

        // See if first opcode to read is implemented, else skip this file
        if(UNDEFINED == opcodes[read_from_memory(&m, m.cpu.pc)]) {
            return RESULT_FILTERED_OUT;
        }
        
        if(!loadStateFromJSON(aTest, "final", &finalState)) {
            return RESULT_PREP_ERROR;
        }

        // Get the cycles array
        cJSON *cycles = cJSON_GetObjectItemCaseSensitive(aTest, "cycles");
        if(!cJSON_IsArray(cycles)) {
            return RESULT_PREP_ERROR;
        }
        cJSON *cycle = NULL;

        // for all cycles, process a cycle
        cJSON_ArrayForEach(cycle, cycles) {
            if(!cJSON_IsArray(cycle) || 3 != cJSON_GetArraySize(cycle)) {
                return RESULT_PREP_ERROR;
            }
            expected_address = cJSON_GetNumberValue(cJSON_GetArrayItem(cycle, 0));
            expected_value = cJSON_GetNumberValue(cJSON_GetArrayItem(cycle, 1));
            expected_action = cJSON_GetStringValue(cJSON_GetArrayItem(cycle, 2));
            if(isnan(expected_address) || isnan(expected_value) || expected_action == NULL) {
                return RESULT_PREP_ERROR;
            }

            // Step the machine a single CPU cycle
            machine_step(&m);

            // Compare expected result with actual result
            if(actual_address != expected_address || 
                actual_value != expected_value ||
                actual_action != expected_action[0]) {
                    return RESULT_FAIL;
            }
        }

        // All steps passed - check if the end state is the desired end state
        return verifyState(&m, failState, &finalState, NULL);

    } else {
        logMessage(CHAN_ERROR, "Test not found");
        return RESULT_PREP_ERROR;
    }
}

// Process a single json test file, one test at a time, reporting test outcomes
int processFile(const char *filePath) {
    FILE *file = fopen(filePath, "r");
    if (file != NULL) {
        
        fseek(file, 0, SEEK_END);
        long fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);
        char *buffer = (char *)malloc(fileSize + 1);
        fread(buffer, 1, fileSize, file);
        buffer[fileSize] = '\0';
        fclose(file);

        cJSON *json = cJSON_Parse(buffer);
        if (json == NULL) {
            const char *error_ptr = cJSON_GetErrorPtr();
            if (error_ptr != NULL) {
                logMessage(CHAN_ERROR, "JSON Error before: %s\n", error_ptr);
            }
            cJSON_Delete(json);
            free(buffer);
            return RESULT_PREP_ERROR;
        }

        FAIL_STATE failState;
        cJSON *aTest = NULL;
        cJSON *arrayContainer = NULL;

        // If a single test is loaded, put it in an array
        if(!cJSON_IsArray(json)) {
            arrayContainer = cJSON_CreateArrayReference(json);
            if(!arrayContainer) {
                return RESULT_PREP_ERROR;
            }
            cJSON_AddItemToArray(arrayContainer, json);
        } else {
            arrayContainer = json;
            json = NULL;
        }

        // For all tests in the array (file) do
        cJSON_ArrayForEach(aTest, arrayContainer) {

            int result = runTest(aTest, ++testNumber, &failState);
            switch(result) {
                case RESULT_PREP_ERROR:
                    logToStr(CHAN_ERROR, 1, " - SKIPPED - other (JSON?) error\n");
                    testsSkipped++;
                    break;
                case RESULT_SUCCESS:
                    logToStr(CHAN_SUCCESS, 1, " - PASS\n");
                    testsPassed++;
                    break;
                case RESULT_FAIL:
                    logToStr(CHAN_FAIL, 1, " - FAIL\n");
                    testsFailed++;
                    break;
                case RESULT_FILTERED_OUT:
                    logToStr(CHAN_FILTER, 1, " - SKIPPED - filtered out\n");
                    testsSkipped++;
                    break;
                default:
                    logToStr(CHAN_ERROR, 1, " - Unknown result code\n");
            }

            // Always reset the string
            logClearString();

            // If unimplemented opcode, break out of this loop of tests
            if(result == RESULT_FILTERED_OUT) {
                break;
            }
        }

        cJSON_Delete(arrayContainer);
        if(json) {
            cJSON_Delete(json);
        }
        free(buffer);
        return RESULT_SUCCESS;
    }
    return RESULT_PREP_ERROR;
}

int main(int argc, char **argv) {
    int ergv = 0;
    opcode_steps **cpu_type = opcodes_6502;
    logInit();
    if(argc >= 2 && argv[1][0] == '-') {
        ergv++;
        cpu_type = opcodes_65c02;
    }
    if(argc < 2 + ergv) {
        logMessage(CHAN_ERROR, "USAGE: %s <directory/*.json>\n"\
               "   where *.json is a particluar (ex. a9.json) or\n"\
               "   all (*.json) files from the HARTE test to run.\n",
               argv[0]+strrtok(argv[0]));
        return RESULT_SUCCESS;;
    }

    logChannelOFF(CHAN_SUCCESS);

    // Set this machine up as a harte test computer
    if(!configure_Harte(&m, cpu_type)) {
        logMessage(CHAN_ERROR, "Failed to configure Harte Test Computer\n");
        return 1;   // FAIL code at OS level
    }

    // Init the CPU inside the machine
    cpu_init(&m.cpu);

    // Process the command line and call ProcessFile on all files that
    // were passed on the command line (through wildcards)
#ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    hFind = FindFirstFile(argv[ergv+1], &findFileData);

    // Iterate over files that match the command line wildcard
    if (hFind == INVALID_HANDLE_VALUE) {
        logMessage(CHAN_ERROR, "FindFirstFile failed (%d)\n", GetLastError());
        return 1;
    } else {
        char filePath[MAX_PATH];
        int nameOffset = strrtok(argv[ergv+1]);
        strncpy(filePath, argv[ergv+1], nameOffset);
        do {
            snprintf(filePath+nameOffset, MAX_PATH-nameOffset, "%s", findFileData.cFileName);
#ifndef SHORT_FORM
            logMessage(CHAN_TEST_INFO, "Test File: %s\n", findFileData.cFileName);
#endif
            // Process all matching files in turn
            processFile(filePath);
        } while (FindNextFile(hFind, &findFileData) != 0);
        FindClose(hFind);
    }
#else
    int i;
    for(i = 1; i < argc; i++) {
#ifndef SHORT_FORM
        logMessage(CHAN_TEST_INFO, "Test File: %s\n", &argv[ergv+i][strrtok(argv[ergv+i])]);
#endif        
        processFile(argv[ergv+i]);
    }
#endif

#ifdef SHORT_FORM
    logMessage(CHAN_TEST_INFO, "TEST %.2s: "\
                               "Ran: %5d "\
                               "Passed: %5d "\
                               "Failed: %5d "\
                               "Skipped: %d "\
                               "Percent: %3.2f%%\n",
                               findFileData.cFileName, testNumber - testsSkipped, testsPassed, testsFailed, testsSkipped, testsPassed + testsFailed ? (double)(testsPassed*100)/(testsPassed + testsFailed) : 0);
#else
    logMessage(CHAN_TEST_INFO, "TEST RESULTS:\n"\
                               "Ran    : %d\n"\
                               "Passed : %d\n"\
                               "Failed : %d\n"\
                               "Skipped: %d\n"\
                               "Percent: %3.2f%%\n",
                               testNumber - testsSkipped, testsPassed, testsFailed, testsSkipped, testsPassed + testsFailed ? (double)(testsPassed*100)/(testsPassed + testsFailed) : 0);
#endif                               
    return 0;   // SUCCESS code at OS level
}
