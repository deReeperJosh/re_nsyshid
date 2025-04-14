#include <wut.h>

#include <array>
#include <mutex>
#include <queue>
#include <span>

#include "Device.h"

class KamenRiderUSBDevice final : public Device {
public:
    KamenRiderUSBDevice();
    ~KamenRiderUSBDevice();

    bool GetDescriptor(uint8_t descType,
                       uint8_t descIndex,
                       uint16_t lang,
                       uint8_t *buffer,
                       uint32_t bufferLength) override;

    bool SetDescriptor(uint8_t descType,
                       uint8_t descIndex,
                       uint16_t lang,
                       uint8_t *buffer,
                       uint32_t bufferLength) override;

    bool GetReport(uint8_t *buffer,
                   uint32_t bufferLength) override;

    bool SetReport(uint8_t *buffer,
                   uint32_t bufferLength) override;

    bool GetIdle(uint8_t ifIndex,
                 uint8_t reportId,
                 uint8_t *duration) override;

    bool SetIdle(uint8_t ifIndex,
                 uint8_t reportId,
                 uint8_t duration) override;

    bool GetProtocol(uint8_t ifIndex,
                     uint8_t *protocol) override;

    bool SetProtocol(uint8_t ifIndex,
                     uint8_t protocol) override;

    bool Read(uint8_t *buffer,
              uint32_t bufferLength) override;

    bool Write(uint8_t *buffer,
               uint32_t bufferLength) override;

private:
    bool m_IsOpened;
};

class RiderGate {
public:
    struct RiderFigure final {
        FILE *kamenFile;
        std::array<uint8_t, 0x14 * 0x10> data{};
        std::array<uint8_t, 7> uid{};
        bool present = false;
        void Save();
    };

    void SendCommand(std::span<const uint8_t> buf, const uint8_t length);
    std::array<uint8_t, 64> GetStatus();

    void GetListTags(std::array<uint8_t, 64> &replyBuf, uint8_t command, uint8_t sequence);
    void QueryBlock(std::array<uint8_t, 64> &replyBuf, uint8_t command, uint8_t sequence,
                    const uint8_t *uid, uint8_t block, uint8_t line);
    void WriteBlock(std::array<uint8_t, 64> &replyBuf, uint8_t command, uint8_t sequence,
                    const uint8_t *uid, uint8_t block, uint8_t line, const uint8_t *toWriteBuf);
    void GetBlankResponse(uint8_t command, uint8_t sequence, std::array<uint8_t, 64> &replyBuf);

    bool RemoveFigure(uint8_t index);
    uint8_t LoadFigure(const std::array<uint8_t, 0x14 * 0x10> &buf, FILE *file);

    std::string FindFigure(uint8_t type, uint8_t id);

protected:
    std::mutex m_kamenRiderMutex;
    std::array<RiderFigure, 8> m_figures{};

private:
    uint8_t GenerateChecksum(const std::array<uint8_t, 64> &data,
                             int numOfBytes) const;
    RiderFigure &GetFigureByUID(const std::array<uint8_t, 7> uid);

    std::queue<std::array<uint8_t, 64>> m_figureAddedRemovedResponses;
    std::queue<std::array<uint8_t, 64>> m_queries;

    bool m_isAwake = false;
};
extern RiderGate g_kamenridegate;