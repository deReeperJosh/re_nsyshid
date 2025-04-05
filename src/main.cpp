#include "utils/logger.h"
#include <coreinit/filesystem.h>
#include <wups.h>
#include <wups/button_combo/api.h>
#include <wups/config/WUPSConfigCategory.h>
#include <wups/config/WUPSConfigItemBoolean.h>
#include <wups/config/WUPSConfigItemButtonCombo.h>
#include <wups/config/WUPSConfigItemIntegerRange.h>
#include <wups/config/WUPSConfigItemMultipleValues.h>
#include <wups/config/WUPSConfigItemStub.h>
#include <wups/config_api.h>

#include "config/ConfigItemSelectInfinity.hpp"
#include "config/ConfigItemSelectSkylander.hpp"
#include "re_nsyshid.h"

#include <forward_list>

#include <malloc.h>

#define STR_VALUE(arg)          #arg
#define VERSION_STRING(x, y, z) "v" STR_VALUE(x) "." STR_VALUE(y) "." STR_VALUE(z)

/**
    Mandatory plugin information.
    If not set correctly, the loader will refuse to use the plugin.
**/
WUPS_PLUGIN_NAME("re_nsyshid");
WUPS_PLUGIN_DESCRIPTION("Plugin to Emulate HID Devices");
WUPS_PLUGIN_VERSION(VERSION_STRING(VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH));
WUPS_PLUGIN_AUTHOR("deReeperJosh");
WUPS_PLUGIN_LICENSE("GPLv2");

#define LOG_FS_OPEN_CONFIG_ID             "logFSOpen"
#define BUTTON_COMBO_PRESS_DOWN_CONFIG_ID "pressDownItem"
#define BUTTON_COMBO_HOLD_CONFIG_ID       "holdItem"
#define OTHER_EXAMPLE_BOOL_CONFIG_ID      "otherBoolItem"
#define OTHER_EXAMPLE2_BOOL_CONFIG_ID     "other2BoolItem"
#define INTEGER_RANGE_EXAMPLE_CONFIG_ID   "intRangeExample"

#define TAG_EMULATION_PATH                std::string("/vol/external01/wiiu/re_nsyshid/")

/**
    All of this defines can be used in ANY file.
    It's possible to split it up into multiple files.

**/

WUPS_USE_WUT_DEVOPTAB();        // Use the wut devoptabs
WUPS_USE_STORAGE("re_nsyshid"); // Unique id for the storage api

enum ExampleOptions {
    EXAMPLE_OPTION_1 = 0,
    EXAMPLE_OPTION_2 = 1,
    EXAMPLE_OPTION_3 = 2,
};

#define LOF_FS_OPEN_DEFAULT_VALUE      true
#define INTEGER_RANGE_DEFAULT_VALUE    10
#define EMULATION_STATUS_DEFAULT_VALUE DISABLED
#define EMULATED_DEVICE_DEFAULT_VALUE  NONE

bool sLogFSOpen                  = LOF_FS_OPEN_DEFAULT_VALUE;
int sIntegerRangeValue           = INTEGER_RANGE_DEFAULT_VALUE;
EmulationStatus sEmulationStatus = EMULATION_STATUS_DEFAULT_VALUE;
DeviceToEmulate sEmulatedDevice  = EMULATED_DEVICE_DEFAULT_VALUE;

static WUPSButtonCombo_ComboHandle sPressDownExampleHandle(nullptr);
static WUPSButtonCombo_ComboHandle sHoldExampleHandle(nullptr);

WUPSButtonCombo_Buttons DEFAULT_PRESS_DOWN_BUTTON_COMBO = WUPS_BUTTON_COMBO_BUTTON_L | WUPS_BUTTON_COMBO_BUTTON_R;
WUPSButtonCombo_Buttons DEFAULT_PRESS_HOLD_COMBO        = WUPS_BUTTON_COMBO_BUTTON_L | WUPS_BUTTON_COMBO_BUTTON_R | WUPS_BUTTON_COMBO_BUTTON_DOWN;

/**
 * Callback that will be called if the config has been changed
 */
void boolItemChanged(ConfigItemBoolean *item, bool newValue) {
    DEBUG_FUNCTION_LINE_INFO("New value in boolItemChanged: %d", newValue);
    if (std::string_view(LOG_FS_OPEN_CONFIG_ID) == item->identifier) {
        sLogFSOpen = newValue;
        // If the value has changed, we store it in the storage.
        WUPSStorageAPI::Store(item->identifier, newValue);
    } else if (std::string_view(OTHER_EXAMPLE_BOOL_CONFIG_ID) == item->identifier) {
        DEBUG_FUNCTION_LINE_ERR("Other bool value has changed to %d", newValue);
    } else if (std::string_view(OTHER_EXAMPLE2_BOOL_CONFIG_ID) == item->identifier) {
        DEBUG_FUNCTION_LINE_ERR("Other2 bool value has changed to %d", newValue);
    }
}

void integerRangeItemChanged(ConfigItemIntegerRange *item, int newValue) {
    DEBUG_FUNCTION_LINE_INFO("New value in integerRangeItemChanged: %d", newValue);
    // If the value has changed, we store it in the storage.
    if (std::string_view(LOG_FS_OPEN_CONFIG_ID) == item->identifier) {
        sIntegerRangeValue = newValue;
        // If the value has changed, we store it in the storage.
        WUPSStorageAPI::Store(item->identifier, newValue);
    }
}

void multipleValueItemChanged(ConfigItemMultipleValues *item, uint32_t newValue) {
    DEBUG_FUNCTION_LINE_INFO("New value in emulationStatus: %d", newValue);
    // If the value has changed, we store it in the storage.
    if (std::string_view(EMULATION_STATUS_CONFIG_ID) == item->identifier) {
        sEmulationStatus = (EmulationStatus) newValue;
        // If the value has changed, we store it in the storage.
        WUPSStorageAPI::Store(item->identifier, sEmulationStatus);
    } else if (std::string_view(EMULATED_DEVICE_CONFIG_ID) == item->identifier) {
        sEmulatedDevice = (DeviceToEmulate) newValue;
        // If the value has changed, we store it in the storage.
        WUPSStorageAPI::Store(item->identifier, sEmulatedDevice);
    }
}

void buttonComboItemChanged(ConfigItemButtonCombo *item, uint32_t newValue) {
    DEBUG_FUNCTION_LINE_INFO("New value in buttonComboItemChanged: %d for %s", newValue, item->identifier);
}

static void skylanderSelectedCallback(ConfigItemSelectSkylander *skylanders, const char *filePath, uint8_t slot) {
    DEBUG_FUNCTION_LINE_INFO("New skylander selected: %d for %s", slot, filePath);
    WUPSStorageAPI_StoreString(nullptr, ("currentSkylanderPath" + std::to_string(slot)).c_str(), filePath);
}

static void infinityToySelectedCallback(ConfigItemSelectInfinity *infinity, const char *filePath, uint8_t slot) {
    DEBUG_FUNCTION_LINE_INFO("New infinity toy selected: %d for %s", slot, filePath);
    WUPSStorageAPI_StoreString(nullptr, ("currentInfinityPath" + std::to_string(slot)).c_str(), filePath);
}

WUPSConfigAPICallbackStatus ConfigMenuOpenedCallback(WUPSConfigCategoryHandle rootHandle) {
    // To use the C++ API, we create new WUPSConfigCategory from the root handle!
    WUPSConfigCategory root = WUPSConfigCategory(rootHandle);

    // The functions of the Config API come in two variants: One that throws an exception, and another one which doesn't
    // To use the Config API without exception see the example below this try/catch block.
    try {
        // To select value from an enum WUPSConfigItemMultipleValues fits the best.
        constexpr WUPSConfigItemMultipleValues::ValuePair emulationStatusValues[] = {
                {DISABLED, "Disable Emulated Devices"},
                {ENABLED, "Enable Emulated Devices"},
        };

        EmulationStatus status;
        DeviceToEmulate deviceToEmulate;

        WUPSStorageError storageRes;
        if ((storageRes = WUPSStorageAPI::GetOrStoreDefault(EMULATION_STATUS_CONFIG_ID, status,
                                                            DISABLED)) != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE_ERR("GetOrStoreDefault failed: %s (%d)",
                                    WUPSStorageAPI_GetStatusStr(storageRes), storageRes);
        }
        if ((storageRes = WUPSStorageAPI::GetOrStoreDefault(EMULATED_DEVICE_CONFIG_ID, deviceToEmulate,
                                                            NONE)) != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE_ERR("GetOrStoreDefault failed: %s (%d)",
                                    WUPSStorageAPI_GetStatusStr(storageRes), storageRes);
        }
        if ((storageRes = WUPSStorageAPI::SaveStorage()) != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE_ERR("SaveStorage failed: %s (%d)",
                                    WUPSStorageAPI_GetStatusStr(storageRes), storageRes);
        }

        // It comes in two variants.
        // - "WUPSConfigItemMultipleValues::CreateFromValue" will take a default and current **value**
        // - "WUPSConfigItemMultipleValues::CreateFromIndex" will take a default and current **index**
        root.add(WUPSConfigItemMultipleValues::CreateFromValue(EMULATION_STATUS_CONFIG_ID, "Emulation Status",
                                                               DISABLED, status,
                                                               emulationStatusValues,
                                                               multipleValueItemChanged));

        constexpr WUPSConfigItemMultipleValues::ValuePair deviceToEmulateValues[] = {
                {NONE, "No Device Connected"},
                {SKYLANDER, "Emulate Skylander Portal"},
                {INFINITY, "Emulate Infinity Base"},
        };

        root.add(WUPSConfigItemMultipleValues::CreateFromValue(EMULATED_DEVICE_CONFIG_ID, "Device to Emulate",
                                                               NONE, deviceToEmulate,
                                                               deviceToEmulateValues,
                                                               multipleValueItemChanged));

        auto skylanderCategory = WUPSConfigCategory::Create("Skylander Manager");
        skylanderCategory.add(WUPSConfigItemStub::Create("Press \ue002 to Remove Skylander From Slot"));
        for (int i = 0; i < 16; i++) {
            char *currentPath    = new char[1024];
            WUPSStorageError err = WUPSStorageAPI_GetString(nullptr, ("currentSkylanderPath" + std::to_string(i)).c_str(), currentPath, 1024, nullptr);
            DEBUG_FUNCTION_LINE_VERBOSE("Adding skylander config: %d", i);
            if (err == WUPS_STORAGE_ERROR_SUCCESS) {
                if (!ConfigItemSelectSkylander_AddToCategory(skylanderCategory.getHandle(), ("select_skylander" + std::to_string(i)).c_str(), ("Select Skylander " + std::to_string(i + 1)).c_str(), i, TAG_EMULATION_PATH.c_str(), currentPath, skylanderSelectedCallback)) {
                    return WUPSCONFIG_API_CALLBACK_RESULT_ERROR;
                }
            } else {
                if (!ConfigItemSelectSkylander_AddToCategory(skylanderCategory.getHandle(), ("select_skylander" + std::to_string(i)).c_str(), ("Select Skylander " + std::to_string(i + 1)).c_str(), i, TAG_EMULATION_PATH.c_str(), TAG_EMULATION_PATH.c_str(), skylanderSelectedCallback)) {
                    return WUPSCONFIG_API_CALLBACK_RESULT_ERROR;
                }
            }
        }

        root.add(std::move(skylanderCategory));

        auto infinityCategory = WUPSConfigCategory::Create("Infinity Manager");
        infinityCategory.add(WUPSConfigItemStub::Create("Press \ue002 to Remove Infinity Toy From Slot"));
        ConfigItemSelectInfinity_AddToCategory(infinityCategory.getHandle(), "playset_powerdisc", "Play Set/Power Disc", 0, TAG_EMULATION_PATH.c_str(), TAG_EMULATION_PATH.c_str(), infinityToySelectedCallback);
        ConfigItemSelectInfinity_AddToCategory(infinityCategory.getHandle(), "powerdisc_two", "Power Disc Two", 1, TAG_EMULATION_PATH.c_str(), TAG_EMULATION_PATH.c_str(), infinityToySelectedCallback);
        ConfigItemSelectInfinity_AddToCategory(infinityCategory.getHandle(), "powerdisc_three", "Power Disc Three", 2, TAG_EMULATION_PATH.c_str(), TAG_EMULATION_PATH.c_str(), infinityToySelectedCallback);
        ConfigItemSelectInfinity_AddToCategory(infinityCategory.getHandle(), "player_one", "Player One", 3, TAG_EMULATION_PATH.c_str(), TAG_EMULATION_PATH.c_str(), infinityToySelectedCallback);
        ConfigItemSelectInfinity_AddToCategory(infinityCategory.getHandle(), "p1_ability_one", "Ability One (P1)", 4, TAG_EMULATION_PATH.c_str(), TAG_EMULATION_PATH.c_str(), infinityToySelectedCallback);
        ConfigItemSelectInfinity_AddToCategory(infinityCategory.getHandle(), "p1_ability_two", "Ability Two (P2)", 5, TAG_EMULATION_PATH.c_str(), TAG_EMULATION_PATH.c_str(), infinityToySelectedCallback);
        ConfigItemSelectInfinity_AddToCategory(infinityCategory.getHandle(), "player_two", "Player Two", 6, TAG_EMULATION_PATH.c_str(), TAG_EMULATION_PATH.c_str(), infinityToySelectedCallback);
        ConfigItemSelectInfinity_AddToCategory(infinityCategory.getHandle(), "p2_ability_one", "Ability One (P2)", 7, TAG_EMULATION_PATH.c_str(), TAG_EMULATION_PATH.c_str(), infinityToySelectedCallback);
        ConfigItemSelectInfinity_AddToCategory(infinityCategory.getHandle(), "p2_ability_two", "Ability Two (P2)", 8, TAG_EMULATION_PATH.c_str(), TAG_EMULATION_PATH.c_str(), infinityToySelectedCallback);

        root.add(std::move(infinityCategory));
        // root.add(WUPSConfigCategory::Create("Dimensions Manager"));
    } catch (std::exception &e) {
        DEBUG_FUNCTION_LINE_ERR("Creating config menu failed: %s", e.what());
        return WUPSCONFIG_API_CALLBACK_RESULT_ERROR;
    }

    return WUPSCONFIG_API_CALLBACK_RESULT_SUCCESS;
}

void ConfigMenuClosedCallback() {
    DEBUG_FUNCTION_LINE("Saving config of re_nsyshid");
    WUPSStorageAPI::SaveStorage();
}

/**
    Gets called ONCE when the plugin was loaded.
**/
INITIALIZE_PLUGIN() {
    initLogging();
    DEBUG_FUNCTION_LINE("INITIALIZE_PLUGIN of re_nsyshid!");

    WUPSConfigAPIOptionsV1 configOptions = {.name = "re_nsyshid"};
    if (WUPSConfigAPI_Init(configOptions, ConfigMenuOpenedCallback, ConfigMenuClosedCallback) != WUPSCONFIG_API_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to init config api");
    }

    deinitLogging();
}

/**
    Gets called when the plugin will be unloaded.
**/
DEINITIALIZE_PLUGIN() {
    // Remove all button combos from this plugin.
    DEBUG_FUNCTION_LINE("DEINITIALIZE_PLUGIN of re_nsyshid!");
}

/**
    Gets called when an application starts.
**/
ON_APPLICATION_START() {
    initLogging();

    DEBUG_FUNCTION_LINE("ON_APPLICATION_START of re_nsyshid!");
}

/**
 * Gets called when an application actually ends
 */
ON_APPLICATION_ENDS() {
    DEBUG_FUNCTION_LINE("ON_APPLICATION_ENDS of re_nsyshid!");
    ResetClientLibrary();
    deinitLogging();
}

/**
    Gets called when an application request to exit.
**/
ON_APPLICATION_REQUESTS_EXIT() {
    DEBUG_FUNCTION_LINE_INFO("ON_APPLICATION_REQUESTS_EXIT of re_nsyshid!");
}

ON_ACQUIRED_FOREGROUND() {
    FireAttachCallbacks();
}

ON_RELEASE_FOREGROUND() {
    FireDetachCallbacks();
}
