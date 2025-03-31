#pragma once

#include <nsyshid/hid.h>
#include <wut.h>

class Device {
public:
    Device() = delete;

    Device(uint16_t vendorId,
           uint16_t productId,
           uint8_t interfaceIndex,
           uint8_t interfaceSubClass,
           uint8_t protocol,
           uint16_t maxPacketSizeRX,
           uint16_t maxPacketSizeTX);

    Device(const Device &device) = delete;

    Device &operator=(const Device &device) = delete;

    virtual ~Device() = default;

    HIDDevice *m_hid; // this info is passed to applications and must remain intact

    uint16_t m_vendorId;
    uint16_t m_productId;
    uint8_t m_interfaceIndex;
    uint8_t m_interfaceSubClass;
    uint8_t m_protocol;
    uint16_t m_maxPacketSizeRX;
    uint16_t m_maxPacketSizeTX;

    virtual void AssignHID(HIDDevice *hid);

    virtual bool GetDescriptor(uint8_t descType,
                               uint8_t descIndex,
                               uint16_t lang,
                               uint8_t *buffer,
                               uint32_t bufferLength) = 0;

    virtual bool SetDescriptor(uint8_t descType,
                               uint8_t descIndex,
                               uint16_t lang,
                               uint8_t *buffer,
                               uint32_t bufferLength) = 0;

    virtual bool GetReport(uint8_t *buffer,
                           uint32_t bufferLength) = 0;

    virtual bool SetReport(uint8_t *buffer,
                           uint32_t bufferLength) = 0;

    virtual bool GetIdle(uint8_t ifIndex,
                         uint8_t reportId,
                         uint8_t *duration) = 0;

    virtual bool SetIdle(uint8_t ifIndex,
                         uint8_t reportId,
                         uint8_t duration) = 0;

    virtual bool GetProtocol(uint8_t ifIndex,
                             uint8_t *protocol) = 0;

    virtual bool SetProtocol(uint8_t ifIndex,
                             uint8_t protocol) = 0;

    virtual bool Read(uint8_t *buffer,
                      uint32_t bufferLength) = 0;

    virtual bool Write(uint8_t *buffer,
                       uint32_t bufferLength) = 0;
};