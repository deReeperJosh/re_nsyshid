/*
* Code repurposed from re_nfpii
*/

#include <wups.h>

#include <vector>
#include <string>

typedef void (*InfinityFigureSelectedCallback)(struct ConfigItemSelectInfinity*, const char* fileName, uint8_t slot);

struct ConfigItemSelectInfinity {
    char* configID;
    WUPSConfigItemHandle handle;

    InfinityFigureSelectedCallback callback;

    uint8_t slot;
    uint32_t figNum;
    std::string rootPath;
    std::string currentPath;
    std::string selectedFigure;
};

std::vector<std::string>& ConfigItemSelectInfinity_GetFavorites(void);

void ConfigItemSelectInfinity_Init(std::string rootPath, bool favoritesPerTitle);

bool ConfigItemSelectInfinity_AddToCategory(WUPSConfigCategoryHandle cat, const char* configID, const char* displayName, uint8_t slot, const char* infinityFolder, const char* currentFigure, InfinityFigureSelectedCallback callback);
