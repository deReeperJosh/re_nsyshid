#include <wups.h>

#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <nsyshid/hid.h>

#include <array>
#include <cstdio>
#include <format>
#include <list>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include "devices/Device.h"
#include "devices/Skylander.h"
#include "re_nsyshid.h"
#include "utils/logger.h"

std::thread hotplugThread;
bool hotplugThreadStop = false;

std::list<HIDClient *> clients;
std::mutex clientMutex;

std::list<std::shared_ptr<Device>> devices;
std::mutex deviceMutex;

std::recursive_mutex hidMutex;

uint32_t _lastGeneratedHidHandle = 0;
const int HID_MAX_NUM_DEVICES    = 128;
std::array<HIDDevice, HID_MAX_NUM_DEVICES> HIDPool;
std::queue<size_t> HIDPoolIndexQueue;

Device::Device(uint16_t vendorId,
               uint16_t productId,
               uint8_t interfaceIndex,
               uint8_t interfaceSubClass,
               uint8_t protocol,
               uint16_t maxPacketSizeRX,
               uint16_t maxPacketSizeTX) {
    m_vendorId          = vendorId;
    m_productId         = productId;
    m_interfaceIndex    = interfaceIndex;
    m_interfaceSubClass = interfaceSubClass;
    m_protocol          = protocol;
    m_maxPacketSizeRX   = maxPacketSizeRX;
    m_maxPacketSizeTX   = maxPacketSizeTX;
}

void Device::AssignHID(HIDDevice *hid) {
    if (hid != nullptr) {
        hid->vid                = this->m_vendorId;
        hid->pid                = this->m_productId;
        hid->interfaceIndex     = this->m_interfaceIndex;
        hid->subClass           = this->m_interfaceSubClass;
        hid->protocol           = this->m_protocol;
        hid->physicalDeviceInst = 0;
        hid->maxPacketSizeRx    = this->m_maxPacketSizeRX;
        hid->maxPacketSizeTx    = this->m_maxPacketSizeTX;
    }
    this->m_hid = hid;
}

uint32_t GenerateHIDHandle() {
    std::lock_guard<std::recursive_mutex> lock(hidMutex);
    _lastGeneratedHidHandle++;
    return _lastGeneratedHidHandle;
}

void InitHIDPoolIndexQueue() {
    static bool HIDPoolIndexQueueInitialized = false;
    std::lock_guard<std::recursive_mutex> lock(hidMutex);
    if (HIDPoolIndexQueueInitialized) {
        return;
    }
    HIDPoolIndexQueueInitialized = true;
    for (size_t i = 0; i < HID_MAX_NUM_DEVICES; i++) {
        HIDPoolIndexQueue.push(i);
    }
}

HIDDevice *GetFreeHID() {
    std::lock_guard<std::recursive_mutex> lock(hidMutex);
    InitHIDPoolIndexQueue();
    if (HIDPoolIndexQueue.empty()) {
        return nullptr;
    }
    size_t index = HIDPoolIndexQueue.front();
    HIDPoolIndexQueue.pop();
    return &HIDPool[index];
}

std::shared_ptr<Device> GetDeviceByHandle(uint32_t handle) {
    std::shared_ptr<Device> device;
    {
        for (const auto &d : devices) {
            if (d->m_hid->handle == handle) {
                device = d;
                break;
            }
        }
    }
    if (device != nullptr) {
        return device;
    }
    return nullptr;
}

DECL_FUNCTION(int32_t, HIDSetup) {
    initLogging();
    DEBUG_FUNCTION_LINE_VERBOSE("nsyshid::HIDSetup Called");
    EmulationStatus status;
    WUPSStorageAPI::Get(std::string_view(EMULATION_STATUS_CONFIG_ID), status);
    if (status == DISABLED) {
        return real_HIDSetup();
    }
    return 0;
}

DECL_FUNCTION(int32_t, HIDTeardown) {
    DEBUG_FUNCTION_LINE_VERBOSE("nsyshid::HIDTeardown Called");
    EmulationStatus status;
    WUPSStorageAPI::Get(std::string_view(EMULATION_STATUS_CONFIG_ID), status);
    if (status == DISABLED) {
        real_HIDTeardown();
    }
    deinitLogging();
    return 0;
}

DECL_FUNCTION(int32_t, HIDAddClient, HIDClient *client, HIDAttachCallback attachCallback) {
    DEBUG_FUNCTION_LINE_VERBOSE("nsyshid::HIDAddClient Called");
    EmulationStatus status;
    WUPSStorageAPI::Get(std::string_view(EMULATION_STATUS_CONFIG_ID), status);
    if (status == DISABLED) {
        return real_HIDAddClient(client, attachCallback);
    }
    clientMutex.lock();
    client->attachCallback = attachCallback;
    clients.push_front(client);
    clientMutex.unlock();

    if (devices.empty()) {
        DeviceToEmulate deviceToEmulate;
        WUPSStorageAPI::Get(std::string_view(EMULATION_STATUS_CONFIG_ID), deviceToEmulate);
        if (deviceToEmulate == SKYLANDER) {
            DEBUG_FUNCTION_LINE_INFO("adding emulated skylander portal");
            HIDDevice *devicePtr;
            auto skylanderDevice = std::make_shared<SkylanderUSBDevice>();
            devicePtr            = GetFreeHID();
            if (devicePtr == nullptr) {
                return 0;
            }
            devicePtr->handle = GenerateHIDHandle();
            skylanderDevice->AssignHID(devicePtr);
            deviceMutex.lock();
            devices.push_back(skylanderDevice);
            deviceMutex.unlock();
        }
    }
    FireAttachCallbacks();
    return 0;
}

DECL_FUNCTION(int32_t, HIDDelClient, HIDClient *client) {
    DEBUG_FUNCTION_LINE_VERBOSE("nsyshid::HIDDelClient Called");
    EmulationStatus status;
    WUPSStorageAPI::Get(std::string_view(EMULATION_STATUS_CONFIG_ID), status);
    if (status == DISABLED) {
        return real_HIDDelClient(client);
    }
    for (const auto &device : devices) {
        client->attachCallback(client, device->m_hid, HID_DEVICE_DETACH);
    }
    clientMutex.lock();
    clients.remove(client);
    clientMutex.unlock();
    FireAttachCallbacks();
    return 0;
}

void DoHIDCallback(HIDCallback callback, uint32_t handle, void *userContext, uint32_t errorCode, uint32_t responseCode, uint8_t *buffer) {
    callback(handle, errorCode, buffer, responseCode, userContext);
}

void GetDescriptorAsync(std::shared_ptr<Device> device, uint8_t descType, uint8_t descIndex, uint16_t lang,
                        uint8_t *buffer, uint32_t bufferLength, HIDCallback callback, void *userContext) {
    if (device->GetDescriptor(descType, descIndex, lang, buffer, bufferLength)) {
        DoHIDCallback(callback, device->m_hid->handle, userContext, 0, bufferLength, buffer);
    } else {
        DoHIDCallback(callback, device->m_hid->handle, userContext, -1, bufferLength, buffer);
    }
}

DECL_FUNCTION(int32_t, HIDGetDescriptor, uint32_t handle, uint8_t descriptorType,
              uint8_t descriptorIndex, uint16_t languageId, uint8_t *buffer, uint32_t bufferLength,
              HIDCallback callback, void *userContext) {
    DEBUG_FUNCTION_LINE_VERBOSE("nsyshid::HIDGetDescriptor Called");
    EmulationStatus status;
    WUPSStorageAPI::Get(std::string_view(EMULATION_STATUS_CONFIG_ID), status);
    if (status == DISABLED) {
        return real_HIDGetDescriptor(handle, descriptorType, descriptorIndex, languageId, buffer, bufferLength, callback, userContext);
    }

    std::shared_ptr<Device> device = GetDeviceByHandle(handle);

    if (!device)
        return -111;

    if (callback != nullptr) {
        std::thread t(GetDescriptorAsync, device, descriptorType, descriptorIndex, languageId, buffer, bufferLength, callback, userContext);
        t.detach();
    } else {
        if (device->GetDescriptor(descriptorType, descriptorIndex, languageId, buffer, bufferLength)) {
            return bufferLength;
        }
    }
    return 0;
}

void SetDescriptorAsync(std::shared_ptr<Device> device, uint8_t descType, uint8_t descIndex, uint16_t lang,
                        uint8_t *buffer, uint32_t bufferLength, HIDCallback callback, void *userContext) {
    if (device->SetDescriptor(descType, descIndex, lang, buffer, bufferLength)) {
        DoHIDCallback(callback, device->m_hid->handle, userContext, 0, bufferLength, buffer);
    } else {
        DoHIDCallback(callback, device->m_hid->handle, userContext, -1, bufferLength, buffer);
    }
}

DECL_FUNCTION(int32_t, HIDSetDescriptor, uint32_t handle, uint8_t descriptorType,
              uint8_t descriptorIndex, uint16_t languageId, uint8_t *buffer, uint32_t bufferLength,
              HIDCallback callback, void *userContext) {
    DEBUG_FUNCTION_LINE_VERBOSE("nsyshid::HIDSetDescriptor Called");
    EmulationStatus status;
    WUPSStorageAPI::Get(std::string_view(EMULATION_STATUS_CONFIG_ID), status);
    if (status == DISABLED) {
        return real_HIDSetDescriptor(handle, descriptorType, descriptorIndex, languageId, buffer, bufferLength, callback, userContext);
    }

    std::shared_ptr<Device> device = GetDeviceByHandle(handle);

    if (!device)
        return -111;

    if (callback != nullptr) {
        std::thread t(SetDescriptorAsync, device, descriptorType, descriptorIndex, languageId, buffer, bufferLength, callback, userContext);
        t.detach();
    } else {
        if (device->SetDescriptor(descriptorType, descriptorIndex, languageId, buffer, bufferLength)) {
            return bufferLength;
        }
    }
    return 0;
}

void GetReportAsync(std::shared_ptr<Device> device, uint8_t reportType, uint8_t reportId,
                    uint8_t *buffer, uint32_t bufferLength, HIDCallback callback, void *userContext) {
    if (device->GetReport(buffer, bufferLength)) {
        DoHIDCallback(callback, device->m_hid->handle, userContext, 0, bufferLength, buffer);
    } else {
        DoHIDCallback(callback, device->m_hid->handle, userContext, -1, bufferLength, buffer);
    }
}

DECL_FUNCTION(int32_t, HIDGetReport, uint32_t handle, uint8_t reportType, uint8_t reportId,
              uint8_t *buffer, uint32_t bufferLength, HIDCallback callback, void *userContext) {
    DEBUG_FUNCTION_LINE_VERBOSE("nsyshid::HIDGetReport Called");
    EmulationStatus status;
    WUPSStorageAPI::Get(std::string_view(EMULATION_STATUS_CONFIG_ID), status);
    if (status == DISABLED) {
        return real_HIDGetReport(handle, reportType, reportId, buffer, bufferLength, callback, userContext);
    }

    std::shared_ptr<Device> device = GetDeviceByHandle(handle);

    if (!device)
        return -111;

    if (callback != nullptr) {
        std::thread t(GetReportAsync, device, reportType, reportId, buffer, bufferLength, callback, userContext);
        t.detach();
    } else {
        if (device->GetReport(buffer, bufferLength)) {
            return bufferLength;
        }
    }
    return 0;
}

void SetReportAsync(std::shared_ptr<Device> device, uint8_t reportType, uint8_t reportId,
                    uint8_t *buffer, uint32_t bufferLength, HIDCallback callback, void *userContext) {
    if (device->SetReport(buffer, bufferLength)) {
        DoHIDCallback(callback, device->m_hid->handle, userContext, 0, bufferLength, buffer);
    } else {
        DoHIDCallback(callback, device->m_hid->handle, userContext, -1, bufferLength, buffer);
    }
}

DECL_FUNCTION(int32_t, HIDSetReport, uint32_t handle, uint8_t reportType, uint8_t reportId,
              uint8_t *buffer, uint32_t bufferLength, HIDCallback callback, void *userContext) {
    DEBUG_FUNCTION_LINE_VERBOSE("nsyshid::HIDSetReport Called");
    EmulationStatus status;
    WUPSStorageAPI::Get(std::string_view(EMULATION_STATUS_CONFIG_ID), status);
    if (status == DISABLED) {
        return real_HIDSetReport(handle, reportType, reportId, buffer, bufferLength, callback, userContext);
    }
    std::shared_ptr<Device> device = GetDeviceByHandle(handle);

    if (!device)
        return -111;

    if (callback != nullptr) {
        std::thread t(SetReportAsync, device, reportType, reportId, buffer, bufferLength, callback, userContext);
        t.detach();
    } else {
        if (device->SetReport(buffer, bufferLength)) {
            return bufferLength;
        }
    }
    return 0;
}

void GetIdleAsync(std::shared_ptr<Device> device, uint8_t interfaceIndex, uint8_t reportId,
                  uint8_t *duration, HIDCallback callback, void *userContext) {
    if (device->GetIdle(interfaceIndex, reportId, duration)) {
        DoHIDCallback(callback, device->m_hid->handle, userContext, 0, 1, duration);
    } else {
        DoHIDCallback(callback, device->m_hid->handle, userContext, -1, 1, duration);
    }
}

DECL_FUNCTION(int32_t, HIDGetIdle, uint32_t handle, uint8_t interfaceIndex, uint8_t reportId,
              uint8_t *duration, HIDCallback callback, void *userContext) {
    DEBUG_FUNCTION_LINE_VERBOSE("nsyshid::HIDGetIdle Called");
    EmulationStatus status;
    WUPSStorageAPI::Get(std::string_view(EMULATION_STATUS_CONFIG_ID), status);
    if (status == DISABLED) {
        return real_HIDGetIdle(handle, interfaceIndex, reportId, duration, callback, userContext);
    }
    std::shared_ptr<Device> device = GetDeviceByHandle(handle);

    if (!device)
        return -111;

    if (callback != nullptr) {
        std::thread t(GetIdleAsync, device, interfaceIndex, reportId, duration, callback, userContext);
        t.detach();
    } else {
        if (device->GetIdle(interfaceIndex, reportId, duration)) {
            return 1;
        }
    }
    return 0;
}

void SetIdleAsync(std::shared_ptr<Device> device, uint8_t interfaceIndex, uint8_t reportId,
                  uint8_t duration, HIDCallback callback, void *userContext) {
    if (device->SetIdle(interfaceIndex, reportId, duration)) {
        DoHIDCallback(callback, device->m_hid->handle, userContext, 0, 0, nullptr);
    } else {
        DoHIDCallback(callback, device->m_hid->handle, userContext, -1, 0, nullptr);
    }
}

DECL_FUNCTION(int32_t, HIDSetIdle, uint32_t handle, uint8_t interfaceIndex, uint8_t reportId,
              uint8_t duration, HIDCallback callback, void *userContext) {
    DEBUG_FUNCTION_LINE_VERBOSE("nsyshid::HIDSetIdle Called");
    EmulationStatus status;
    WUPSStorageAPI::Get(std::string_view(EMULATION_STATUS_CONFIG_ID), status);
    if (status == DISABLED) {
        return real_HIDSetIdle(handle, interfaceIndex, reportId, duration, callback, userContext);
    }
    std::shared_ptr<Device> device = GetDeviceByHandle(handle);

    if (!device)
        return -111;

    if (callback != nullptr) {
        std::thread t(SetIdleAsync, device, interfaceIndex, reportId, duration, callback, userContext);
        t.detach();
    } else {
        if (device->SetIdle(interfaceIndex, reportId, duration)) {
            return 0;
        }
    }
    return 0;
}

void GetProtocolAsync(std::shared_ptr<Device> device, uint8_t interfaceIndex, uint8_t *protocol,
                      HIDCallback callback, void *userContext) {
    if (device->GetProtocol(interfaceIndex, protocol)) {
        DoHIDCallback(callback, device->m_hid->handle, userContext, 0, 1, protocol);
    } else {
        DoHIDCallback(callback, device->m_hid->handle, userContext, -1, 1, protocol);
    }
}

DECL_FUNCTION(int32_t, HIDGetProtocol, uint32_t handle, uint8_t interfaceIndex, uint8_t *protocol,
              HIDCallback callback, void *userContext) {
    DEBUG_FUNCTION_LINE_VERBOSE("nsyshid::HIDGetProtocol Called");
    EmulationStatus status;
    WUPSStorageAPI::Get(std::string_view(EMULATION_STATUS_CONFIG_ID), status);
    if (status == DISABLED) {
        return real_HIDGetProtocol(handle, interfaceIndex, protocol, callback, userContext);
    }
    std::shared_ptr<Device> device = GetDeviceByHandle(handle);

    if (!device)
        return -111;

    if (callback != nullptr) {
        std::thread t(GetProtocolAsync, device, interfaceIndex, protocol, callback, userContext);
        t.detach();
    } else {
        if (device->GetProtocol(interfaceIndex, protocol)) {
            return 1;
        }
    }
    return 0;
}

void SetProtocolAsync(std::shared_ptr<Device> device, uint8_t interfaceIndex, uint8_t protocol,
                      HIDCallback callback, void *userContext) {
    if (device->SetProtocol(interfaceIndex, protocol)) {
        DoHIDCallback(callback, device->m_hid->handle, userContext, 0, 0, nullptr);
    } else {
        DoHIDCallback(callback, device->m_hid->handle, userContext, -1, 0, nullptr);
    }
}

DECL_FUNCTION(int32_t, HIDSetProtocol, uint32_t handle, uint8_t interfaceIndex, uint8_t protocol,
              HIDCallback callback, void *userContext) {
    DEBUG_FUNCTION_LINE_VERBOSE("nsyshid::HIDSetProtocol Called");
    EmulationStatus status;
    WUPSStorageAPI::Get(std::string_view(EMULATION_STATUS_CONFIG_ID), status);
    if (status == DISABLED) {
        return real_HIDSetProtocol(handle, interfaceIndex, protocol, callback, userContext);
    }
    std::shared_ptr<Device> device = GetDeviceByHandle(handle);

    if (!device)
        return -111;

    if (callback != nullptr) {
        std::thread t(SetProtocolAsync, device, interfaceIndex, protocol, callback, userContext);
        t.detach();
    } else {
        if (device->SetProtocol(interfaceIndex, protocol)) {
            return 0;
        }
    }
    return 0;
}

void ReadAsync(std::shared_ptr<Device> device, uint8_t *buffer, uint32_t bufferLength,
               HIDCallback callback, void *userContext) {
    if (device->Read(buffer, bufferLength)) {
        DoHIDCallback(callback, device->m_hid->handle, userContext, 0, bufferLength, buffer);
    } else {
        DoHIDCallback(callback, device->m_hid->handle, userContext, -1, bufferLength, buffer);
    }
}

DECL_FUNCTION(int32_t, HIDRead, uint32_t handle, uint8_t *buffer, uint32_t bufferLength,
              HIDCallback callback, void *userContext) {
    DEBUG_FUNCTION_LINE_VERBOSE("nsyshid::HIDRead Called");
    EmulationStatus status;
    WUPSStorageAPI::Get(std::string_view(EMULATION_STATUS_CONFIG_ID), status);
    if (status == DISABLED) {
        return real_HIDRead(handle, buffer, bufferLength, callback, userContext);
    }
    std::shared_ptr<Device> device = GetDeviceByHandle(handle);

    if (!device)
        return -111;

    if (callback != nullptr) {
        std::thread t(ReadAsync, device, buffer, bufferLength, callback, userContext);
        t.detach();
    } else {
        if (device->Read(buffer, bufferLength)) {
            return bufferLength;
        }
    }
    return 0;
}

void WriteAsync(std::shared_ptr<Device> device, uint8_t *buffer, uint32_t bufferLength,
                HIDCallback callback, void *userContext) {
    if (device->Write(buffer, bufferLength)) {
        DoHIDCallback(callback, device->m_hid->handle, userContext, 0, bufferLength, buffer);
    } else {
        DoHIDCallback(callback, device->m_hid->handle, userContext, -1, bufferLength, buffer);
    }
}

DECL_FUNCTION(int32_t, HIDWrite, int32_t handle, uint8_t *buffer, uint32_t bufferLength,
              HIDCallback callback, void *userContext) {
    DEBUG_FUNCTION_LINE_VERBOSE("nsyshid::HIDWrite Called");
    EmulationStatus status;
    WUPSStorageAPI::Get(std::string_view(EMULATION_STATUS_CONFIG_ID), status);
    if (status == DISABLED) {
        return real_HIDWrite(handle, buffer, bufferLength, callback, userContext);
    }
    std::shared_ptr<Device> device = GetDeviceByHandle(handle);

    if (!device)
        return -111;

    if (callback != nullptr) {
        std::thread t(WriteAsync, device, buffer, bufferLength, callback, userContext);
        t.detach();
    } else {
        if (device->Write(buffer, bufferLength)) {
            return bufferLength;
        }
    }
    return 0;
}

void FireAttachCallbacks() {
    EmulationStatus status;
    WUPSStorageAPI::Get(std::string_view(EMULATION_STATUS_CONFIG_ID), status);
    if (status == ENABLED) {
        deviceMutex.lock();
        clientMutex.lock();
        for (const auto &device : devices) {
            for (const auto &client : clients) {
                bool claimedDevice = false;
                if (!claimedDevice && client->attachCallback) {
                    int32_t result = client->attachCallback(client, device->m_hid, HID_DEVICE_ATTACH);
                    if (result == 1) {
                        claimedDevice = true;
                        DEBUG_FUNCTION_LINE_INFO("Client wants device handle: %d inst: %d vid: %x pid: %x interface: %d subclass: %d protocol: %d rx: %d tx: %d", device->m_hid->handle, device->m_hid->physicalDeviceInst, device->m_hid->vid, device->m_hid->pid, device->m_hid->interfaceIndex, device->m_hid->subClass, device->m_hid->protocol, device->m_hid->maxPacketSizeRx, device->m_hid->maxPacketSizeTx);
                    } else {
                        DEBUG_FUNCTION_LINE_INFO("Client doesn't want device handle: %d inst: %d vid: %x pid: %x interface: %d subclass: %d protocol: %d rx: %d tx: %d", device->m_hid->handle, device->m_hid->physicalDeviceInst, device->m_hid->vid, device->m_hid->pid, device->m_hid->interfaceIndex, device->m_hid->subClass, device->m_hid->protocol, device->m_hid->maxPacketSizeRx, device->m_hid->maxPacketSizeTx);
                    }
                }
            }
        }
        clientMutex.unlock();
        deviceMutex.unlock();
    }
}

void FireDetachCallbacks() {
    EmulationStatus status;
    WUPSStorageAPI::Get(std::string_view(EMULATION_STATUS_CONFIG_ID), status);
    if (status == ENABLED) {
        deviceMutex.lock();
        clientMutex.lock();
        for (const auto &device : devices) {
            for (const auto &client : clients) {
                if (client->attachCallback) {
                    DEBUG_FUNCTION_LINE_INFO("detaching device handle: %d inst: %d vid: %x pid: %x interface: %d subclass: %d protocol: %d rx: %d tx: %d", device->m_hid->handle, device->m_hid->physicalDeviceInst, device->m_hid->vid, device->m_hid->pid, device->m_hid->interfaceIndex, device->m_hid->subClass, device->m_hid->protocol, device->m_hid->maxPacketSizeRx, device->m_hid->maxPacketSizeTx);
                    client->attachCallback(client, device->m_hid, HID_DEVICE_DETACH);
                }
            }
        }
        clientMutex.unlock();
        deviceMutex.unlock();
    }
}

void ResetClientLibrary() {
    clients.clear();
    for (const auto &device : devices) {
        device->AssignHID(device->m_hid);
    }
}

WUPS_MUST_REPLACE(HIDSetup, WUPS_LOADER_LIBRARY_NSYSHID, HIDSetup);
WUPS_MUST_REPLACE(HIDTeardown, WUPS_LOADER_LIBRARY_NSYSHID, HIDTeardown);
WUPS_MUST_REPLACE(HIDAddClient, WUPS_LOADER_LIBRARY_NSYSHID, HIDAddClient);
WUPS_MUST_REPLACE(HIDDelClient, WUPS_LOADER_LIBRARY_NSYSHID, HIDDelClient);
WUPS_MUST_REPLACE(HIDGetDescriptor, WUPS_LOADER_LIBRARY_NSYSHID, HIDGetDescriptor);
WUPS_MUST_REPLACE(HIDSetDescriptor, WUPS_LOADER_LIBRARY_NSYSHID, HIDSetDescriptor);
WUPS_MUST_REPLACE(HIDGetReport, WUPS_LOADER_LIBRARY_NSYSHID, HIDGetReport);
WUPS_MUST_REPLACE(HIDSetReport, WUPS_LOADER_LIBRARY_NSYSHID, HIDSetReport);
WUPS_MUST_REPLACE(HIDGetIdle, WUPS_LOADER_LIBRARY_NSYSHID, HIDGetIdle);
WUPS_MUST_REPLACE(HIDSetIdle, WUPS_LOADER_LIBRARY_NSYSHID, HIDSetIdle);
WUPS_MUST_REPLACE(HIDGetProtocol, WUPS_LOADER_LIBRARY_NSYSHID, HIDGetProtocol);
WUPS_MUST_REPLACE(HIDSetProtocol, WUPS_LOADER_LIBRARY_NSYSHID, HIDSetProtocol);
WUPS_MUST_REPLACE(HIDRead, WUPS_LOADER_LIBRARY_NSYSHID, HIDRead);
WUPS_MUST_REPLACE(HIDWrite, WUPS_LOADER_LIBRARY_NSYSHID, HIDWrite);
