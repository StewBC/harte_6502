#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"

LOG logObject;

void logInit() {
#ifdef SHORT_FORM
    logObject.channelMask = 0xffffffe7; // ~(CHAN_FILTER | CHAN_FAIL)
#else
    logObject.channelMask = 0xffffffff;
#endif
    logObject.logStream = stdout;
    logObject.logFile = NULL;
    logObject.logFileName = NULL;
    logObject.logStringIndex = 0;
    logObject.logString[0] = '\0';
}

void logSetChannels(uint32_t channels) {
    logObject.channelMask = channels;
}

void logChannelOFF(uint32_t channel) {
    logObject.channelMask &= ~channel;
}

void logChannelON(uint32_t channel) {
    logObject.channelMask |= channel;
}

void logClearString() {
    logObject.logStringIndex = 0;
    logObject.logString[0] = '\0';
}

FILE *logFile(const char *name, const char *mode) {
    if(logObject.logFile) {
        fclose(logObject.logFile);
    }

    if(logObject.logFileName) {
        free(logObject.logFileName);
    }
    logObject.logFileName = strdup(name);
    if(logObject.logFileName) {
        return (logObject.logFile = fopen(name, mode));
    }
    return NULL;
}

void logFileClose() {
    if(logObject.logFile) {
        fclose(logObject.logFile);
    }
    logObject.logFile = NULL;
    
    if(logObject.logFileName) {
        free(logObject.logFileName);
    }
    logObject.logFileName = NULL;
}

void logMessage(uint32_t channel, const char *format, ...) {
    if(channel & logObject.channelMask && (logObject.logStream || logObject.logFile)) {
        va_list args;
        va_start(args, format);
        if(logObject.logStream) {
            vfprintf(logObject.logStream, format, args);
        }
        if(logObject.logFile) {
            vfprintf(logObject.logFile, format, args);
        }
        va_end(args);
    }
}

void logToStr(uint32_t channel, int eol, const char *format, ...) {
    if(channel & logObject.channelMask) {
        va_list args;
        va_start(args, format);
        logObject.logStringIndex += vsprintf(&logObject.logString[logObject.logStringIndex], format, args);
        va_end(args);
        if(eol) {
            if(logObject.logStream) {
                fprintf(logObject.logStream, "%s", logObject.logString);
                logObject.logStringIndex = 0;
                logObject.logString[0] = '\0';
            }
            if(logObject.logFile) {
                fprintf(logObject.logFile, "%s", logObject.logString);
            }
        }
    }
}

void logToStrBinaryBits(uint32_t channel, uint32_t bits, int numBits) {
    if(channel & logObject.channelMask) {
        int i;
        for (i = numBits-1; i >= 0; i--) {
            sprintf(&logObject.logString[logObject.logStringIndex++], "%c", (bits & (1 << i)) ? '1' : '0');
        }
    }
}