#include "Device.h"
#include <wut.h>

#include <array>
#include <map>
#include <mutex>
#include <optional>
#include <queue>
#include <stdio.h>

namespace Infinity {

    enum SubFolder {
        TOP,
        ONE,
        ONE_CHAR,
        ONE_CHAR_CARS,
        ONE_CHAR_FROZEN,
        ONE_CHAR_MONSTERS,
        ONE_CHAR_PHINEAS,
        ONE_CHAR_PIRATES,
        ONE_CHAR_INCREDIBLES,
        ONE_CHAR_RANGER,
        ONE_CHAR_TOY_STORY,
        ONE_CHAR_WRECK_IT_RALPH,
        ONE_CHAR_OTHER,
        ONE_PLAYSET,
        ONE_POWER_DISCS,
        ONE_POWER_SERIES_ONE,
        ONE_POWER_SERIES_TWO,
        ONE_POWER_SERIES_THREE,
        ONE_POWER_EXCLUSIVES,
        TWO,
        TWO_CHAR,
        TWO_CHAR_ALADDIN,
        TWO_CHAR_BIG_HERO,
        TWO_CHAR_GUARDIANS,
        TWO_CHAR_AVENGERS,
        TWO_CHAR_SPIDERMAN,
        TWO_CHAR_OTHER,
        TWO_PLAYSET,
        TWO_TOYBOX_GAME,
        TWO_POWER_DISCS,
        TWO_POWER_WAVE_ONE,
        TWO_POWER_WAVE_TWO,
        TWO_POWER_WAVE_THREE,
        THREE,
        THREE_CHAR,
        THREE_CHAR_ALICE,
        THREE_CHAR_AVENGERS,
        THREE_CHAR_CAPTAIN_AMERICA,
        THREE_CHAR_FINDING_DORY,
        THREE_CHAR_INSIDE_OUT,
        THREE_CHAR_MOUSE_UNIVERSE,
        THREE_CHAR_STAR_WARS,
        THREE_CHAR_STAR_WARS_REBELS,
        THREE_CHAR_STAR_WARS_CLONE_WARS,
        THREE_CHAR_STAR_WARS_FORCE_AWAKENS,
        THREE_CHAR_TRON,
        THREE_CHAR_ZOOTOPIA,
        THREE_CHAR_OTHER,
        THREE_PLAYSET,
        THREE_TOYBOX_GAME,
        THREE_POWER_DISCS,
        THREE_POWER_LAUNCH,
        THREE_POWER_POST_LAUNCH,
        THREE_POWER_OTHER,
    };

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
        bool CreateFigure(std::string pathName, uint32_t figureNum, uint8_t series);
        static std::map<const uint32_t, const std::pair<const uint8_t, const char *>> GetFigureList();
        static std::vector<uint32_t> GetFiguresForFolder(const SubFolder &folder);
        std::pair<uint8_t, std::string> FindFigure(uint32_t figNum);
        uint32_t FindFigureFromSlot(uint8_t slot);

    protected:
        std::mutex m_infinityMutex;
        std::array<InfinityFigure, 9> m_figures;
        InfinityFigure m_notPresentFigure;

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
        std::array<uint8_t, 16> GenerateBlankFigureData(uint32_t figureNum, uint8_t series);

        uint32_t m_randomA;
        uint32_t m_randomB;
        uint32_t m_randomC;
        uint32_t m_randomD;

        uint8_t m_figureOrder = 0;
        std::queue<std::array<uint8_t, 32>> m_figureAddedRemovedResponses;
        std::queue<std::array<uint8_t, 32>> m_queries;
    };

    extern InfinityBase g_infinitybase;
} // namespace Infinity