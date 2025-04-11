/*
* Code repurposed from re_nfpii
*/

#include <wups.h>

#include <array>
#include <optional>
#include <queue>
#include <string>
#include <vector>

#include "devices/Dimensions.h"
#include "utils/DrawUtils.hpp"

typedef void (*DimensionsSelectedCallback)(struct ConfigItemDimensionsPad *, const char *fileName, uint8_t slot);

struct ConfigItemDimensionsPad {
    char *configID;
    WUPSConfigItemHandle handle;

    DimensionsSelectedCallback callback;

    std::optional<uint8_t> moveIndex;
    std::array<std::optional<std::string>, 7> figureFiles;
    std::array<std::optional<uint32_t>, 7> figureNumbers;
    std::string rootPath;
    std::string currentPath;
    Color topColor = Color(255, 255, 255, 255);
    Color leftColor = Color(255, 255, 255, 255);
    Color rightColor = Color(255, 255, 255, 255);
    std::queue<Color> topColors;
    std::queue<Color> leftColors;
    std::queue<Color> rightColors;
    DimensionsToypad::DimensionsLEDColor topPad;
    DimensionsToypad::DimensionsLEDColor leftPad;
    DimensionsToypad::DimensionsLEDColor rightPad;
};

std::vector<std::string> &ConfigItemDimensionsPad_GetFavorites(void);

void ConfigItemDimensionsPad_Init(std::string rootPath, bool favoritesPerTitle);

bool ConfigItemDimensionsPad_AddToCategory(WUPSConfigCategoryHandle cat, const char *configID, const char *displayName, const char *dimensionsFolder, const char *currentFigure, DimensionsSelectedCallback callback);
