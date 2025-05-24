#include "Device.h"
#include <wut.h>

#include <array>
#include <map>
#include <mutex>
#include <optional>
#include <queue>
#include <stdio.h>


enum SubFolder {
    TOP,
    SSA,
    SSA_CHAR,
    SSA_CHAR_AIR,
    SSA_CHAR_EARTH,
    SSA_CHAR_FIRE,
    SSA_CHAR_LIFE,
    SSA_CHAR_MAGIC,
    SSA_CHAR_TECH,
    SSA_CHAR_UNDEAD,
    SSA_CHAR_WATER,
    SSA_MAGIC_ITEM,
    SSA_SIDEKICK,
    SG,
    SG_GIANTS,
    SG_CHAR,
    SG_CHAR_AIR,
    SG_CHAR_EARTH,
    SG_CHAR_FIRE,
    SG_CHAR_LIFE,
    SG_CHAR_MAGIC,
    SG_CHAR_TECH,
    SG_CHAR_UNDEAD,
    SG_CHAR_WATER,
    SG_MAGIC_ITEM,
    SG_SIDEKICK,
    SSF,
    SSF_SWAPPERS,
    SSF_SWAP_AIR,
    SSF_SWAP_EARTH,
    SSF_SWAP_FIRE,
    SSF_SWAP_WATER,
    SSF_SWAP_MAGIC,
    SSF_SWAP_TECH,
    SSF_SWAP_LIFE,
    SSF_SWAP_UNDEAD,
    SSF_CHAR,
    SSF_CHAR_AIR,
    SSF_CHAR_EARTH,
    SSF_CHAR_FIRE,
    SSF_CHAR_LIFE,
    SSF_CHAR_MAGIC,
    SSF_CHAR_TECH,
    SSF_CHAR_UNDEAD,
    SSF_CHAR_WATER,
    SSF_MAGIC_ITEM,
    STT,
    STT_CHAR,
    STT_CHAR_AIR,
    STT_CHAR_DARK,
    STT_CHAR_EARTH,
    STT_CHAR_FIRE,
    STT_CHAR_LIFE,
    STT_CHAR_LIGHT,
    STT_CHAR_MAGIC,
    STT_CHAR_TECH,
    STT_CHAR_UNDEAD,
    STT_CHAR_WATER,
    STT_MAGIC_ITEM,
    STT_MINIS,
    STT_TRAPS,
    STT_TRAP_AIR,
    STT_TRAP_DARK,
    STT_TRAP_EARTH,
    STT_TRAP_FIRE,
    STT_TRAP_LIFE,
    STT_TRAP_LIGHT,
    STT_TRAP_MAGIC,
    STT_TRAP_TECH,
    STT_TRAP_UNDEAD,
    STT_TRAP_WATER,
    STT_TRAP_KAOS,
    SSC,
    SSC_VEHICLES,
    SSC_VEHICLE_AIR,
    SSC_VEHICLE_LAND,
    SSC_VEHICLE_SEA,
    SSC_CHAR,
    SSC_CHAR_AIR,
    SSC_CHAR_DARK,
    SSC_CHAR_EARTH,
    SSC_CHAR_FIRE,
    SSC_CHAR_LIFE,
    SSC_CHAR_LIGHT,
    SSC_CHAR_MAGIC,
    SSC_CHAR_TECH,
    SSC_CHAR_UNDEAD,
    SSC_CHAR_WATER,
    SSC_TROPHIES,
};

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
    bool CreateSkylander(std::string pathName, uint16_t skyId, uint16_t skyVar);
    uint16_t SkylanderCRC16(uint16_t initValue, const uint8_t *buffer, uint32_t size);
    static std::map<const std::pair<const uint16_t, const uint16_t>, const char *> GetListSkylanders();
    static std::vector<std::pair<const uint16_t, const uint16_t>> GetSkylandersForFolder(const SubFolder &folder);
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
