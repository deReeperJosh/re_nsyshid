#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum rensyshidEmulationState {
    NSYSHID_EMULATION_OFF,
    NSYSHID_EMULATION_ON
} EmulationState;

typedef enum rensyshidLogVerbosity {
    NSYSHID_LOG_VERBOSITY_INFO,
    NSYSHID_LOG_VERBOSITY_WARN,
    NSYSHID_LOG_VERBOSITY_ERROR,
} rensyshidLogVerbosity;

typedef void (*renyshsidLogHandler)(rensyshidLogVerbosity verb, const char* message);

rensyshidEmulationState rensyshidGetEmulationState(void);

void rensyshidSetEmulationState(rensyshidEmulationState state);

void rensyshidSetLogHandler(renyshsidLogHandler handler);

#ifdef __cplusplus
}
#endif