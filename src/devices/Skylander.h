#include "Device.h"
#include <wut.h>

#include <array>
#include <map>
#include <mutex>
#include <optional>
#include <queue>
#include <stdio.h>

class SkylanderUSBDevice : public Device {
public:
    SkylanderUSBDevice(/* args */);
    ~SkylanderUSBDevice();

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
    /* data */
};

constexpr uint16_t SKY_BLOCK_COUNT = 0x40;
constexpr uint16_t SKY_BLOCK_SIZE  = 0x10;
constexpr uint16_t SKY_FIGURE_SIZE = SKY_BLOCK_COUNT * SKY_BLOCK_SIZE;
constexpr uint8_t MAX_SKYLANDERS   = 16;

class SkylanderPortal {
public:
    struct Skylander final {
        FILE *skyFile;
        uint8_t status = 0;
        std::queue<uint8_t> queuedStatus;
        std::array<uint8_t, SKY_FIGURE_SIZE> data{};
        uint32_t lastId = 0;
        void Save();

        enum : uint8_t {
            REMOVED  = 0,
            READY    = 1,
            REMOVING = 2,
            ADDED    = 3
        };
    };

    struct SkylanderLEDColor final {
        uint8_t red   = 0;
        uint8_t green = 0;
        uint8_t blue  = 0;
    };

    void ControlTransfer(uint8_t *buf, uint32_t length);

    void Activate();
    void Deactivate();
    void SetLeds(uint8_t side, uint8_t r, uint8_t g, uint8_t b);

    std::array<uint8_t, 64> GetStatus();
    void QueryBlock(uint8_t skyNum, uint8_t block, uint8_t *replyBuf);
    void WriteBlock(uint8_t skyNum, uint8_t block, const uint8_t *toWriteBuf,
                    uint8_t *replyBuf);

    bool LoadSkylander(uint8_t *buf, FILE *file, uint8_t uiSlot);
    uint8_t LoadSkylander(uint8_t *buf, FILE *file);
    bool RemoveSkylander(uint8_t skyNum);
    static std::map<const std::pair<const uint16_t, const uint16_t>, const char *> GetListSkylanders();
    std::string FindSkylander(uint16_t skyId, uint16_t skyVar);

    std::string GetSkylanderFromUISlot(uint8_t uiSlot);

protected:
    std::mutex m_skyMutex;
    std::mutex m_queryMutex;
    std::array<Skylander, MAX_SKYLANDERS> m_skylanders;
    std::array<std::optional<uint8_t>, MAX_SKYLANDERS> m_skylanderUIPositions;

private:
    std::queue<std::array<uint8_t, 64>> m_queries;
    bool m_activated               = true;
    uint8_t m_interruptCounter     = 0;
    SkylanderLEDColor m_colorRight = {};
    SkylanderLEDColor m_colorLeft  = {};
    SkylanderLEDColor m_colorTrap  = {};
};

extern SkylanderPortal g_skyportal;
