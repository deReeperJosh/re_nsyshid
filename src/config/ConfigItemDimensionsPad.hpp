/*
* Code repurposed from re_nfpii
*/

#include <wups.h>

#include <array>
#include <optional>
#include <string>
#include <vector>

typedef void (*DimensionsSelectedCallback)(struct ConfigItemDimensionsPad*, const char* fileName, uint8_t slot);

struct ConfigItemDimensionsPad {
    char* configID;
    WUPSConfigItemHandle handle;

    DimensionsSelectedCallback callback;

    std::optional<uint8_t> moveIndex;
    std::array<std::optional<std::string>, 7> figureFiles;
    std::array<std::optional<uint32_t>, 7> figureNumbers;
    std::string rootPath;
    std::string currentPath;
};

std::vector<std::string>& ConfigItemDimensionsPad_GetFavorites(void);

void ConfigItemDimensionsPad_Init(std::string rootPath, bool favoritesPerTitle);

bool ConfigItemDimensionsPad_AddToCategory(WUPSConfigCategoryHandle cat, const char* configID, const char* displayName, const char* dimensionsFolder, const char* currentFigure, DimensionsSelectedCallback callback);
