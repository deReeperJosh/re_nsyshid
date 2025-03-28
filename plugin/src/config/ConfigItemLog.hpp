#include <wups.h>

enum LogType {
    LOG_TYPE_NORMAL,
    LOG_TYPE_WARN,
    LOG_TYPE_ERROR,
};

struct ConfigItemLog {
    char* configID;
    WUPSConfigItemHandle handle;
};

void ConfigItemLog_Init(void);

void ConfigItemLog_PrintType(LogType type, const char* text);

bool ConfigItemLog_AddToCategory(WUPSConfigCategoryHandle cat, const char* configID, const char* displayName);

#define ConfigItemLog_AddToCategoryHandled(__config__, __cat__, __configID__, __displayName__)  \
    do {                                                                                        \
        if (!ConfigItemLog_AddToCategory(__cat__, __configID__, __displayName__)) {             \
            WUPSConfigAPI_Category_Destroy(__config__);                                                     \
            return WUPSCONFIG_API_CALLBACK_RESULT_SUCCESS;                                                                           \
        }                                                                                       \
    } while (0)
