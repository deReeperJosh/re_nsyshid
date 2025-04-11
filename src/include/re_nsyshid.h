#pragma once

#define EMULATION_STATUS_CONFIG_ID              "emulationStatus"
#define EMULATED_DEVICE_CONFIG_ID               "emulatedDevice"

#define RE_NSYSHID_VERSION_MAJOR(v)             ((v >> 16) & 0xff)
#define RE_NSYSHID_VERSION_MINOR(v)             ((v >> 8) & 0xff)
#define RE_NSYSHID_VERSION_PATCH(v)             (v & 0xff)
#define RE_NSYSHID_VERSION(major, minor, patch) ((major << 16) | (minor << 8) | patch)

enum EmulationStatus {
    DISABLED = 0,
    ENABLED  = 1
};

enum DeviceToEmulate {
    NONE       = 0,
    SKYLANDER  = 1,
    INFINITY   = 2,
    DIMENSIONS = 3
};

void ResetClientLibrary();
void FireDetachCallbacks();
void FireAttachCallbacks();