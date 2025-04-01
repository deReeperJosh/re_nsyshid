/*
* Code repurposed from re_nfpii
*/

#include <wups.h>

#include <vector>
#include <string>

typedef void (*SkylanderSelectedCallback)(struct ConfigItemSelectSkylander*, const char* fileName, uint8_t slot);

struct ConfigItemSelectSkylander {
    char* configID;
    WUPSConfigItemHandle handle;

    SkylanderSelectedCallback callback;

    uint8_t slot;
    std::string rootPath;
    std::string currentPath;
    std::string selectedSkylander;
};

std::vector<std::string>& ConfigItemSelectSkylander_GetFavorites(void);

void ConfigItemSelectSkylander_Init(std::string rootPath, bool favoritesPerTitle);

bool ConfigItemSelectSkylander_AddToCategory(WUPSConfigCategoryHandle cat, const char* configID, const char* displayName, uint8_t slot, const char* skylanderFolder, const char* currentSkylander, SkylanderSelectedCallback callback);
