#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum rensyshidEmulationStatie {
    NSYSHID_EMULATION_OFF,
    NSYSHID_EMULATION_ON
} EmulationState;

void rensyshidExampleMethod(void);

#ifdef __cplusplus
}
#endif