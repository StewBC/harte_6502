#pragma once

enum LOG_CHANNELS {
    CHAN_ERROR     = (1<<0),
    CHAN_TEST_INFO = (1<<1),
    CHAN_SUCCESS   = (1<<2),
    CHAN_FAIL      = (1<<3),
    CHAN_STEP      = (1<<4),
};
typedef enum LOG_CHANNELS LOG_CHANNELS;

struct LOG {
    uint32_t    channelMask;
    FILE        *logStream;
    FILE        *logFile;
    char        *logFileName;

    char        logString[256];
    uint32_t    logStringIndex;
};
typedef struct LOG LOG;
extern LOG logObject;

void logInit();
void logSetChannels(uint32_t channels);
void logChannelOFF(uint32_t channel);
void logChannelON(uint32_t channel);
void logClearString();
FILE *logFile(const char *name, const char *mode);
void logFileClose();
void logMessage(uint32_t channel, const char *format, ...);
void logToStr(uint32_t channel, int eol, const char *format, ...);
void logToStrBinaryBits(uint32_t channel, uint32_t bits, int numBits);