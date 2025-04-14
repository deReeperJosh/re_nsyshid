#include "KamenRider.h"

#include <array>
#include <cstdlib>
#include <cstring>
#include <format>
#include <map>
#include <random>
#include <span>

#include "utils/logger.h"

RiderGate g_kamenridegate;

const std::map<const std::pair<const uint8_t, const uint8_t>, const char *> s_listKamenRiders = {
        {{0x10, 0x10}, "Kamen Rider Drive Wind"},
        {{0x10, 0x20}, "Kamen Rider Drive Water"},
        {{0x10, 0x30}, "Kamen Rider Drive Fire"},
        {{0x10, 0x40}, "Kamen Rider Drive Light"},
        {{0x10, 0x50}, "Kamen Rider Drive Dark"},
        {{0x11, 0x10}, "Kamen Rider Gaim Wind"},
        {{0x11, 0x20}, "Kamen Rider Gaim Water"},
        {{0x12, 0x20}, "Kamen Rider Wizard Water"},
        {{0x12, 0x30}, "Kamen Rider Wizard Fire"},
        {{0x13, 0x40}, "Kamen Rider Fourze Light"},
        {{0x14, 0x20}, "Kamen Rider 000 Water"},
        {{0x15, 0x10}, "Kamen Rider Double Wind"},
        {{0x16, 0x50}, "Kamen Rider Decade Dark"},
        {{0x17, 0x50}, "Kamen Rider Kiva Dark"},
        {{0x18, 0x40}, "Kamen Rider Den-O Light"},
        {{0x19, 0x30}, "Kamen Rider Kabuto Fire"},
        {{0x1A, 0x30}, "Kamen Rider Hibiki Fire"},
        {{0x1B, 0x50}, "Kamen Rider Blade Dark"},
        {{0x1C, 0x50}, "Kamen Rider Faiz Dark"},
        {{0x1D, 0x10}, "Kamen Rider Ryuki Wind"},
        {{0x1E, 0x20}, "Kamen Rider Agito Water"},
        {{0x1F, 0x40}, "Kamen Rider Kuuga Light"},
};

const std::map<const uint8_t, const char *> s_listChips = {
        {0x20, "Type Wild"},
        {0x21, "Kamen Rider Zangetsu"},
        {0x22, "All Dragon"},
        {0x31, "Kachidoki Arms"},
};

KamenRiderUSBDevice::KamenRiderUSBDevice()
    : Device(0x6F0E, 0x0A20, 1, 2, 0, 64, 64) {
}

KamenRiderUSBDevice::~KamenRiderUSBDevice() = default;

bool KamenRiderUSBDevice::GetDescriptor(uint8_t descType,
                                        uint8_t descIndex,
                                        uint16_t lang,
                                        uint8_t *output,
                                        uint32_t outputMaxLength) {
    uint8_t configurationDescriptor[0x29];

    uint8_t *currentWritePtr;

    // configuration descriptor
    currentWritePtr                    = configurationDescriptor + 0;
    *(uint8_t *) (currentWritePtr + 0) = 9;    // bLength
    *(uint8_t *) (currentWritePtr + 1) = 2;    // bDescriptorType
    *(uint8_t *) (currentWritePtr + 2) = 0x29; // wTotalLength
    *(uint8_t *) (currentWritePtr + 3) = 0x00; // wTotalLength
    *(uint8_t *) (currentWritePtr + 4) = 1;    // bNumInterfaces
    *(uint8_t *) (currentWritePtr + 5) = 1;    // bConfigurationValue
    *(uint8_t *) (currentWritePtr + 6) = 0;    // iConfiguration
    *(uint8_t *) (currentWritePtr + 7) = 0x80; // bmAttributes
    *(uint8_t *) (currentWritePtr + 8) = 0xFA; // MaxPower
    currentWritePtr                    = currentWritePtr + 9;
    // configuration descriptor
    *(uint8_t *) (currentWritePtr + 0) = 9;    // bLength
    *(uint8_t *) (currentWritePtr + 1) = 0x04; // bDescriptorType
    *(uint8_t *) (currentWritePtr + 2) = 0;    // bInterfaceNumber
    *(uint8_t *) (currentWritePtr + 3) = 0;    // bAlternateSetting
    *(uint8_t *) (currentWritePtr + 4) = 2;    // bNumEndpoints
    *(uint8_t *) (currentWritePtr + 5) = 3;    // bInterfaceClass
    *(uint8_t *) (currentWritePtr + 6) = 0;    // bInterfaceSubClass
    *(uint8_t *) (currentWritePtr + 7) = 0;    // bInterfaceProtocol
    *(uint8_t *) (currentWritePtr + 8) = 0;    // iInterface
    currentWritePtr                    = currentWritePtr + 9;
    // configuration descriptor
    *(uint8_t *) (currentWritePtr + 0) = 9;    // bLength
    *(uint8_t *) (currentWritePtr + 1) = 0x21; // bDescriptorType
    *(uint8_t *) (currentWritePtr + 2) = 0x01; // bcdHID
    *(uint8_t *) (currentWritePtr + 3) = 0x01; // bcdHID
    *(uint8_t *) (currentWritePtr + 4) = 0x00; // bCountryCode
    *(uint8_t *) (currentWritePtr + 5) = 0x01; // bNumDescriptors
    *(uint8_t *) (currentWritePtr + 6) = 0x22; // bDescriptorType
    *(uint8_t *) (currentWritePtr + 7) = 0x1D; // wDescriptorLength
    *(uint8_t *) (currentWritePtr + 8) = 0x00; // wDescriptorLength
    currentWritePtr                    = currentWritePtr + 9;
    // endpoint descriptor 1
    *(uint8_t *) (currentWritePtr + 0) = 7;    // bLength
    *(uint8_t *) (currentWritePtr + 1) = 0x05; // bDescriptorType
    *(uint8_t *) (currentWritePtr + 2) = 0x81; // bEndpointAddress
    *(uint8_t *) (currentWritePtr + 3) = 0x03; // bmAttributes
    *(uint8_t *) (currentWritePtr + 4) = 0x40; // wMaxPacketSize
    *(uint8_t *) (currentWritePtr + 5) = 0x00; // wMaxPacketSize
    *(uint8_t *) (currentWritePtr + 6) = 0x01; // bInterval
    currentWritePtr                    = currentWritePtr + 7;
    // endpoint descriptor 2
    *(uint8_t *) (currentWritePtr + 0) = 7;    // bLength
    *(uint8_t *) (currentWritePtr + 1) = 0x05; // bDescriptorType
    *(uint8_t *) (currentWritePtr + 2) = 0x02; // bEndpointAddress
    *(uint8_t *) (currentWritePtr + 3) = 0x03; // bmAttributes
    *(uint8_t *) (currentWritePtr + 4) = 0x40; // wMaxPacketSize
    *(uint8_t *) (currentWritePtr + 5) = 0x00; // wMaxPacketSize
    *(uint8_t *) (currentWritePtr + 6) = 0x01; // bInterval
    currentWritePtr                    = currentWritePtr + 7;

    memcpy(output, configurationDescriptor,
           std::min<uint32_t>(outputMaxLength, sizeof(configurationDescriptor)));
    return true;
}

bool KamenRiderUSBDevice::SetDescriptor(uint8_t descType,
                                        uint8_t descIndex,
                                        uint16_t lang,
                                        uint8_t *buffer,
                                        uint32_t bufferLength) {
    return true;
}

bool KamenRiderUSBDevice::GetReport(uint8_t *buffer,
                                    uint32_t bufferLength) {
    return true;
}

bool KamenRiderUSBDevice::SetReport(uint8_t *buffer,
                                    uint32_t bufferLength) {
    return true;
}

bool KamenRiderUSBDevice::GetIdle(uint8_t ifIndex,
                                  uint8_t reportId,
                                  uint8_t *duration) {
    return true;
}

bool KamenRiderUSBDevice::SetIdle(uint8_t ifIndex,
                                  uint8_t reportId,
                                  uint8_t duration) {
    return true;
}

bool KamenRiderUSBDevice::GetProtocol(uint8_t ifIndex,
                                      uint8_t *protocol) {
    return true;
}

bool KamenRiderUSBDevice::SetProtocol(uint8_t ifIndex,
                                      uint8_t protocol) {
    return true;
}

bool KamenRiderUSBDevice::Read(uint8_t *buffer,
                               uint32_t bufferLength) {
    memcpy(buffer, g_kamenridegate.GetStatus().data(), bufferLength);
    return true;
}

bool KamenRiderUSBDevice::Write(uint8_t *buffer,
                                uint32_t bufferLength) {
    g_kamenridegate.SendCommand(std::span<const uint8_t>{buffer, bufferLength}, bufferLength);
    return true;
}

std::array<uint8_t, 64> RiderGate::GetStatus() {
    std::array<uint8_t, 64> response = {};

    bool responded = false;
    do {
        if (!m_figureAddedRemovedResponses.empty() && m_isAwake) {
            std::lock_guard lock(m_kamenRiderMutex);
            response = m_figureAddedRemovedResponses.front();
            m_figureAddedRemovedResponses.pop();
            responded = true;
        } else if (!m_queries.empty()) {
            response = m_queries.front();
            m_queries.pop();
            responded = true;
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    } while (responded == false);
    return response;
}

void RiderGate::SendCommand(std::span<const uint8_t> buf, const uint8_t length) {
    const uint8_t command  = buf[2];
    const uint8_t sequence = buf[3];

    std::array<uint8_t, 64> q_result{};

    switch (command) {
        case 0xB0: // Wake
        {
            m_isAwake = true;
            q_result  = {0x55, 0x1a, command, sequence, 0x00, 0x07, 0x00, 0x03, 0x02,
                         0x09, 0x20, 0x03, 0xf5, 0x00, 0x19, 0x42, 0x52, 0xb7,
                         0xb9, 0xa1, 0xae, 0x2b, 0x88, 0x42, 0x05, 0xfe, 0xe0, 0x1c, 0xac};
            break;
        }
        case 0xC0:
        case 0xC3: // Color Commands
        {
            g_kamenridegate.GetBlankResponse(command, sequence, q_result);
            break;
        }
        case 0xD0: // Tag List
        {
            // Return list of figure UIDs, separated by an 09
            g_kamenridegate.GetListTags(q_result, command, sequence);
            break;
        }
        case 0xD2: // Read
        {
            // Read 16 bytes from figure with UID buf[4] - buf[10]
            g_kamenridegate.QueryBlock(q_result, command, sequence, &buf[4], buf[11], buf[12]);
            break;
        }
        case 0xD3: {
            // Write 16 bytes to figure with UID buf[4] - buf[10]
            g_kamenridegate.WriteBlock(q_result, command, sequence, &buf[4], buf[11], buf[12], &buf[13]);
            break;
        }
        default:
            DEBUG_FUNCTION_LINE_ERR("Unknown Kamen Rider Command: %02X", command);
            break;
    }

    m_queries.push(q_result);
}

void RiderGate::GetListTags(std::array<uint8_t, 64> &replyBuf, uint8_t command, uint8_t sequence) {
    replyBuf      = {0x55, 0x02, command, sequence};
    uint8_t index = 4;
    for (auto &figure : m_figures) {
        if (figure.present) {
            replyBuf[index] = 0x09;
            memcpy(&replyBuf[index + 1], figure.data.data(), 7);
            index += 8;
            replyBuf[1] += 8;
        }
    }
    replyBuf[index] = GenerateChecksum(replyBuf, index);
}

void RiderGate::QueryBlock(std::array<uint8_t, 64> &replyBuf, uint8_t command, uint8_t sequence, const uint8_t *uid, uint8_t sector, uint8_t block) {
    replyBuf = {0x55, 0x13, command, sequence, 0x00};

    std::array<uint8_t, 7> uid_array = {uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6]};

    RiderFigure &figure = GetFigureByUID(uid_array);
    if (figure.present) {
        if (sector < 5 && block < 4) {
            memcpy(&replyBuf[5], &figure.data[(sector * 4 * 16) + (block * 16)], 16);
        }
    }
    replyBuf[21] = GenerateChecksum(replyBuf, 21);
}

void RiderGate::WriteBlock(std::array<uint8_t, 64> &replyBuf, uint8_t command, uint8_t sequence, const uint8_t *uid, uint8_t sector, uint8_t block, const uint8_t *toWriteBuf) {
    std::array<uint8_t, 7> uid_array = {uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6]};

    RiderFigure &figure = GetFigureByUID(uid_array);
    if (figure.present) {
        if (sector < 5 && block < 4) {
            memcpy(&figure.data[(sector * 4 * 16) + (block * 16)], toWriteBuf, 16);
        }
    }

    GetBlankResponse(command, sequence, replyBuf);
}

void RiderGate::GetBlankResponse(uint8_t command, uint8_t sequence, std::array<uint8_t, 64> &replyBuf) {
    replyBuf    = {0x55, 0x02, command, sequence};
    replyBuf[4] = GenerateChecksum(replyBuf, 4);
}

bool RiderGate::LoadFigure(const std::array<uint8_t, 0x14 * 0x10> &buf, FILE *file, uint8_t uiSlot) {
    if (m_figureUIPositions[uiSlot]) {
        RemoveFigure(uiSlot);
    }
    m_figureUIPositions[uiSlot] = LoadFigure(buf, std::move(file));
    return true;
}

uint8_t RiderGate::LoadFigure(const std::array<uint8_t, 0x14 * 0x10> &buf, FILE *file) {
    std::lock_guard lock(m_kamenRiderMutex);

    uint8_t foundSlot = 0xFF;

    // mimics spot retaining on the portal
    for (auto i = 0; i < 7; i++) {
        if (!m_figures[i].present) {
            foundSlot = i;
            break;
        }
    }

    if (foundSlot != 0xFF) {
        auto &figure = m_figures[foundSlot];
        memcpy(figure.data.data(), buf.data(), buf.size());
        figure.kamenFile = std::move(file);
        figure.uid       = {buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6]};
        figure.present   = true;

        if (m_isAwake) {
            std::array<uint8_t, 64> figureAddedResponse = {0x56, 0x09, 0x09, 0x01};
            memcpy(&figureAddedResponse[4], figure.uid.data(), figure.uid.size());
            figureAddedResponse[11] = GenerateChecksum(figureAddedResponse, 11);
            m_figureAddedRemovedResponses.push(figureAddedResponse);
        }
    }
    return foundSlot;
}

bool RiderGate::RemoveFigure(uint8_t index) {
    if (!m_figureUIPositions[index]) {
        return false;
    }
    std::lock_guard lock(m_kamenRiderMutex);

    auto &figure = m_figures[m_figureUIPositions[index].value()];

    if (figure.present) {
        figure.present = false;
        figure.Save();
        fclose(figure.kamenFile);
        if (m_isAwake) {
            std::array<uint8_t, 64> figureRemovedResponse = {0x56, 0x09, 0x09, 0x00};
            memcpy(&figureRemovedResponse[4], figure.uid.data(), figure.uid.size());
            figureRemovedResponse[11] = GenerateChecksum(figureRemovedResponse, 11);
            m_figureAddedRemovedResponses.push(figureRemovedResponse);
        }
        figure.uid = {};
        return true;
    }

    return false;
}

std::string RiderGate::FindFigure(uint8_t type, uint8_t id) {
    if (type == 0x00) {
        for (const auto &it : GetChipList()) {
            if (it.first == id) {
                return it.second;
            }
        }
        return std::format("Unknown Chip ({})", id);
    }
    for (const auto &it : GetRiderList()) {
        if (it.first.first == id && it.first.second == type) {
            return it.second;
        }
    }
    switch (type) {
        case 0x10:
            return std::format("Unknown Rider Wind ({})", id);
        case 0x20:
            return std::format("Unknown Rider Water ({})", id);
        case 0x30:
            return std::format("Unknown Rider Fire ({})", id);
        case 0x40:
            return std::format("Unknown Rider Light ({})", id);
        case 0x50:
            return std::format("Unknown Rider Dark ({})", id);
        default:
            return std::format("Unknown Rider ({})", id);
    }
}

std::map<const std::pair<const uint8_t, const uint8_t>, const char *> RiderGate::GetRiderList() {
    return s_listKamenRiders;
}

std::map<const uint8_t, const char *> RiderGate::GetChipList() {
    return s_listChips;
}

RiderGate::RiderFigure &RiderGate::GetFigureByUID(const std::array<uint8_t, 7> uid) {
    for (uint8_t i = 0; i < m_figures.size(); i++) {
        if (m_figures[i].uid == uid) {
            return m_figures[i];
        }
    }
    return m_figures[7];
}

uint8_t RiderGate::GenerateChecksum(const std::array<uint8_t, 64> &data,
                                    int numOfBytes) const {
    int checksum = 0;
    for (int i = 0; i < numOfBytes; i++) {
        checksum += data[i];
    }
    return (checksum & 0xFF);
}

std::string RiderGate::GetFigureFromUISlot(uint8_t uiSlot) {
    if (m_figureUIPositions[uiSlot]) {
        auto &figure       = m_figures[m_figureUIPositions[uiSlot].value()];
        uint8_t figureType = figure.data[0x1B];
        uint8_t figureId   = figure.data[0x19];
        return FindFigure(figureType, figureId);
    }
    return "None";
}

void RiderGate::RiderFigure::Save() {
    if (!kamenFile)
        return;

    fseeko(kamenFile, 0, SEEK_SET);
    fwrite(data.data(), data.size(), sizeof(data[0]), kamenFile);
}