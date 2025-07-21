/*
* Code repurposed from re_nfpii
*/

#include <wups.h>

#include <vector>
#include <string>

typedef void (*KamenRiderSelectedCallback)(struct ConfigItemSelectKamenRider*, const char* fileName, uint8_t slot);

struct ConfigItemSelectKamenRider {
    char* configID;
    WUPSConfigItemHandle handle;

    KamenRiderSelectedCallback callback;

    uint8_t slot;
    std::string rootPath;
    std::string currentPath;
    std::string selectedKamenRider;
};

std::vector<std::string>& ConfigItemSelectKamenRider_GetFavorites(void);

void ConfigItemSelectKamenRider_Init(std::string rootPath, bool favoritesPerTitle);

bool ConfigItemSelectKamenRider_AddToCategory(WUPSConfigCategoryHandle cat, const char* configID, const char* displayName, uint8_t slot, const char* kamenRiderFolder, const char* currentKamenRider, KamenRiderSelectedCallback callback);
