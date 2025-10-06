#include "Device.h"
#include <wut.h>

#include <array>
#include <map>
#include <mutex>
#include <optional>
#include <queue>
#include <span>

class DimensionsUSBDevice final : public Device {
public:
    DimensionsUSBDevice();
    ~DimensionsUSBDevice();

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

constexpr uint16_t DIM_BLOCK_COUNT = 0x2D;
constexpr uint16_t DIM_BLOCK_SIZE  = 0x04;
constexpr uint16_t DIM_FIGURE_SIZE = DIM_BLOCK_COUNT * DIM_BLOCK_SIZE;

class DimensionsToypad {
public:
    struct DimensionsMini final {
        std::string filePath;
        std::array<uint8_t, DIM_FIGURE_SIZE> data{};
        uint8_t index = 255;
        uint8_t pad   = 255;
        uint32_t id   = 0;
        void Save();
    };

    enum class DimensionsColorType {
        NONE,
        COLOR,
        FADE,
        FLASH
    };

    struct DimensionsLEDColor final {
        uint8_t red                   = 0;
        uint8_t green                 = 0;
        uint8_t blue                  = 0;
        uint8_t speed                 = 0;
        uint8_t cycles                = 0;
        uint8_t whiteDuration         = 0;
        uint8_t colorDuration         = 0;
        DimensionsColorType colorType = DimensionsColorType::NONE;
    };

    std::array<uint8_t, 32> GetStatus();
    void SendCommand(std::span<const uint8_t, 32> buf);

    void GenerateRandomNumber(std::span<const uint8_t, 8> buf, uint8_t sequence,
                              std::array<uint8_t, 32> &replyBuf);
    void GetChallengeResponse(std::span<const uint8_t, 8> buf, uint8_t sequence,
                              std::array<uint8_t, 32> &replyBuf);
    void SetPadColor(std::span<const uint8_t, 4> buf, uint8_t sequence,
                     std::array<uint8_t, 32> &replyBuf);
    void SetPadFade(std::span<const uint8_t, 6> buf, uint8_t sequence,
                    std::array<uint8_t, 32> &replyBuf);
    void SetPadFlash(std::span<const uint8_t, 7> buf, uint8_t sequence,
                     std::array<uint8_t, 32> &replyBuf);
    void SetFadeRandom(std::span<const uint8_t, 3> buf, uint8_t sequence,
                       std::array<uint8_t, 32> &replyBuf);
    void SetFadeAll(std::span<const uint8_t, 18> buf, uint8_t sequence,
                    std::array<uint8_t, 32> &replyBuf);
    void SetFlashAll(std::span<const uint8_t, 21> buf, uint8_t sequence,
                     std::array<uint8_t, 32> &replyBuf);
    void SetColorAll(std::span<const uint8_t, 12> buf, uint8_t sequence,
                     std::array<uint8_t, 32> &replyBuf);
    void QueryBlock(uint8_t index, uint8_t page, std::array<uint8_t, 32> &replyBuf,
                    uint8_t sequence);
    void WriteBlock(uint8_t index, uint8_t page, std::span<const uint8_t, 4> toWriteBuf, std::array<uint8_t, 32> &replyBuf,
                    uint8_t sequence);
    void GetModel(std::span<const uint8_t, 8> buf, uint8_t sequence,
                  std::array<uint8_t, 32> &replyBuf);

    bool RemoveFigure(uint8_t pad, uint8_t index, bool fullRemove);
    bool TempRemove(uint8_t index);
    bool CancelRemove(uint8_t index);
    uint32_t LoadFigure(const std::array<uint8_t, DIM_FIGURE_SIZE> &buf, std::string file, uint8_t pad, uint8_t index);
    bool CreateFigure(std::string pathName, uint32_t id);
    bool MoveFigure(uint8_t pad, uint8_t index, uint8_t oldPad, uint8_t oldIndex);
    static std::map<const uint32_t, const char *> GetListMinifigs();
    static std::map<const uint32_t, const char *> GetListTokens();
    std::string FindFigure(uint32_t figNum);

    std::array<std::optional<uint32_t>, 7> GetCurrentFigures();
    std::array<DimensionsLEDColor, 3> GetPadColors();

protected:
    std::mutex m_dimensionsMutex;
    std::array<DimensionsMini, 7> m_figures{};

private:
    void RandomUID(std::array<uint8_t, DIM_FIGURE_SIZE> &uidBuffer);
    uint8_t GenerateChecksum(const std::array<uint8_t, 32> &data,
                             int numOfBytes) const;
    void InitializeRNG(uint32_t seed);
    std::array<uint8_t, 8> Decrypt(std::span<const uint8_t, 8> buf, std::optional<std::array<uint8_t, 16>> key);
    std::array<uint8_t, 8> Encrypt(std::span<const uint8_t, 8> buf, std::optional<std::array<uint8_t, 16>> key);
    std::array<uint8_t, 16> GenerateFigureKey(const std::array<uint8_t, DIM_FIGURE_SIZE> &uid);
    std::array<uint8_t, 4> PWDGenerate(const std::array<uint8_t, DIM_FIGURE_SIZE> &uid);
    std::array<uint8_t, 4> DimensionsRandomize(const std::vector<uint8_t> key, uint8_t count);
    uint32_t GetFigureId(const std::array<uint8_t, DIM_FIGURE_SIZE> &buf);
    uint32_t Scramble(const std::array<uint8_t, 7> &uid, uint8_t count);
    uint32_t GetNext();
    DimensionsMini &GetFigureByIndex(uint8_t index);

    uint32_t m_randomA;
    uint32_t m_randomB;
    uint32_t m_randomC;
    uint32_t m_randomD;

    bool m_isAwake            = false;
    bool m_wasLastRespFigure  = false;
    uint8_t m_noResponseCount = 0;

    std::queue<std::array<uint8_t, 32>> m_figureAddedRemovedResponses;
    std::queue<std::array<uint8_t, 32>> m_queries;

    DimensionsLEDColor m_colorRight = {};
    DimensionsLEDColor m_colorLeft  = {};
    DimensionsLEDColor m_colorTop   = {};
};
extern DimensionsToypad g_dimensionstoypad;