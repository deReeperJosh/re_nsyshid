#include "Device.h"
#include <wut.h>

#include <array>
#include <map>
#include <mutex>
#include <optional>
#include <queue>
#include <stdio.h>

class InfinityUSBDevice : public Device {
public:
    InfinityUSBDevice(/* args */);
    ~InfinityUSBDevice();

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
};

constexpr uint16_t INF_BLOCK_COUNT = 0x14;
constexpr uint16_t INF_BLOCK_SIZE  = 0x10;
constexpr uint16_t INF_FIGURE_SIZE = INF_BLOCK_COUNT * INF_BLOCK_SIZE;
constexpr uint8_t MAX_FIGURES      = 9;

class InfinityBase {
public:
    struct InfinityFigure final {
        FILE *infFile;
        std::array<uint8_t, INF_FIGURE_SIZE> data{};
        bool present       = false;
        uint8_t orderAdded = 255;
        uint32_t figNum    = 0;
        void Save();
    };

    void SendCommand(uint8_t *buf, uint32_t length);
    std::array<uint8_t, 32> GetStatus();

    void GetBlankResponse(uint8_t sequence, std::array<uint8_t, 32> &replyBuf);
    void DescrambleAndSeed(uint8_t *buf, uint8_t sequence,
                           std::array<uint8_t, 32> &replyBuf);
    void GetNextAndScramble(uint8_t sequence, std::array<uint8_t, 32> &replyBuf);
    void GetPresentFigures(uint8_t sequence, std::array<uint8_t, 32> &replyBuf);
    void QueryBlock(uint8_t figNum, uint8_t block, std::array<uint8_t, 32> &replyBuf,
                    uint8_t sequence);
    void WriteBlock(uint8_t figNum, uint8_t block, const uint8_t *toWriteBuf,
                    std::array<uint8_t, 32> &replyBuf, uint8_t sequence);
    void GetFigureIdentifier(uint8_t figNum, uint8_t sequence,
                             std::array<uint8_t, 32> &replyBuf);

    bool RemoveFigure(uint8_t position);
    uint32_t LoadFigure(const std::array<uint8_t, INF_FIGURE_SIZE> &buf,
                        FILE *file, uint8_t position);
    static std::map<const uint32_t, const std::pair<const uint8_t, const char *>> GetFigureList();
    std::pair<uint8_t, std::string> FindFigure(uint32_t figNum);
    uint32_t FindFigureFromSlot(uint8_t slot);

protected:
    std::mutex m_infinityMutex;
    std::array<InfinityFigure, 9> m_figures;

private:
    uint8_t GenerateChecksum(const std::array<uint8_t, 32> &data,
                             int numOfBytes) const;
    uint32_t Descramble(uint64_t numToDescramble);
    uint64_t Scramble(uint32_t numToScramble, uint32_t garbage);
    void GenerateSeed(uint32_t seed);
    uint32_t GetNext();
    InfinityFigure &GetFigureByOrder(uint8_t orderAdded);
    uint8_t DeriveFigurePosition(uint8_t position);
    std::array<uint8_t, 16> GenerateInfinityFigureKey(const std::vector<uint8_t> &sha1Data);
    //std::array<uint8_t, 16> GenerateBlankFigureData(uint32_t figureNum, uint8_t series);

    uint32_t m_randomA;
    uint32_t m_randomB;
    uint32_t m_randomC;
    uint32_t m_randomD;

    uint8_t m_figureOrder = 0;
    std::queue<std::array<uint8_t, 32>> m_figureAddedRemovedResponses;
    std::queue<std::array<uint8_t, 32>> m_queries;
};

extern InfinityBase g_infinitybase;