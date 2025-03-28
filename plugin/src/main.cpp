#include <coreinit/filesystem.h>

#include <wups.h>
#include <wups/config/WUPSConfigItemBoolean.h>
#include <wups/config/WUPSConfigItemMultipleValues.h>

#include "config/ConfigItemLog.hpp"
#include "re_nsyshid.h"
#include "utils/logger.h"

#include <forward_list>

#include <whb/libmanager.h>
#include <whb/log_cafe.h>
#include <whb/log_module.h>
#include <whb/log_udp.h>

#include <malloc.h>

/**
  Mandatory plugin information.
  If not set correctly, the loader will refuse to use the plugin.
**/
WUPS_PLUGIN_NAME("re_nsyshid");
WUPS_PLUGIN_DESCRIPTION("A nsyshid reimplementation with support for Amiibo emulation");
WUPS_PLUGIN_VERSION("0.1");
WUPS_PLUGIN_AUTHOR("deReeperJosh");
WUPS_PLUGIN_LICENSE("GPL");

#define LOG_FS_OPEN_CONFIG_ID "logFSOpen"
#define LOF_FS_OPEN_DEFAULT_VALUE true
bool sLogFSOpen = LOF_FS_OPEN_DEFAULT_VALUE;

#define OTHER_EXAMPLE_BOOL_CONFIG_ID "otherBoolItem"

WUPS_USE_WUT_DEVOPTAB();         // Use the wut devoptabs
WUPS_USE_STORAGE("re_nsyshid");  // Unique id for the storage api

static void nsyshidLogHandler(rensyshidLogVerbosity verb, const char* message)
{
  ConfigItemLog_PrintType((LogType)verb, message);
}

/**
 * Callback that will be called if the config has been changed
 */
void boolItemChanged(ConfigItemBoolean* item, bool newValue)
{
  if (newValue)
  {
    rensyshidSetEmulationState(NSYSHID_EMULATION_ON);
  }
  else
  {
    rensyshidSetEmulationState(NSYSHID_EMULATION_OFF);
  }
}

void stateChangedCallback(ConfigItemMultipleValues* values, uint32_t index)
{
  WUPSStorageAPI_StoreInt(nullptr, "emulationState", (int32_t)index);
  rensyshidSetEmulationState((rensyshidEmulationState)index);
}

WUPSConfigAPICallbackStatus ConfigMenuOpenedCallback(WUPSConfigCategoryHandle rootHandle)
{
  // To use the C++ API, we create new WUPSConfigCategory from the root handle!
  WUPSConfigCategory root = WUPSConfigCategory(rootHandle);

  WUPSConfigCategoryHandle config;
  WUPSConfigAPICreateCategoryOptionsV1 options = {.name = "re_nsyshid"};
  WUPSConfigAPI_Category_Create(options, &config);

  ConfigItemLog_AddToCategoryHandled(config, root.getHandle(), "log", "Logs");

  try
  {
    // To select value from an enum WUPSConfigItemMultipleValues fits the best.
    constexpr WUPSConfigItemMultipleValues::ValuePair emulationStateValues[] = {
        {NSYSHID_EMULATION_OFF, "Emulation Disabled"},
        {NSYSHID_EMULATION_ON, "Emulation Enabled"},
    };

    root.add(WUPSConfigItemMultipleValues::CreateFromValue(
        "emulationState", "Select an option!", NSYSHID_EMULATION_OFF,
        rensyshidGetEmulationState(), emulationStateValues, stateChangedCallback));
  }
  catch (std::exception& e)
  {
    DEBUG_FUNCTION_LINE_ERR("Creating config menu failed: %s", e.what());
    return WUPSCONFIG_API_CALLBACK_RESULT_ERROR;
  }

  return WUPSCONFIG_API_CALLBACK_RESULT_SUCCESS;
}

void ConfigMenuClosedCallback()
{
  WUPSStorageAPI::SaveStorage();
}

/**
  Gets called ONCE when the plugin was loaded.
**/
INITIALIZE_PLUGIN()
{
  initLogging();
  DEBUG_FUNCTION_LINE("INITIALIZE_PLUGIN of re_nsyshid!");
  ConfigItemLog_Init();
  rensyshidSetLogHandler(nsyshidLogHandler);

  WUPSConfigAPIOptionsV1 configOptions = {.name = "re_nsyshid"};
  if (WUPSConfigAPI_Init(configOptions, ConfigMenuOpenedCallback, ConfigMenuClosedCallback) !=
      WUPSCONFIG_API_RESULT_SUCCESS)
  {
    DEBUG_FUNCTION_LINE_ERR("Failed to init config api");
  }

  WUPSStorageError storageRes;
  if ((storageRes = WUPSStorageAPI::GetOrStoreDefault(LOG_FS_OPEN_CONFIG_ID, sLogFSOpen,
                                                      LOF_FS_OPEN_DEFAULT_VALUE)) !=
      WUPS_STORAGE_ERROR_SUCCESS)
  {
    DEBUG_FUNCTION_LINE_ERR("GetOrStoreDefault failed: %s (%d)",
                            WUPSStorageAPI_GetStatusStr(storageRes), storageRes);
  }
  if ((storageRes = WUPSStorageAPI::SaveStorage()) != WUPS_STORAGE_ERROR_SUCCESS)
  {
    DEBUG_FUNCTION_LINE_ERR("GetOrStoreDefault failed: %s (%d)",
                            WUPSStorageAPI_GetStatusStr(storageRes), storageRes);
  }

  // Read values from config
  WUPSStorageError err;
  int32_t emulationState = (int32_t)rensyshidGetEmulationState();
  if ((err = WUPSStorageAPI_GetInt(nullptr, "emulationState", &emulationState)) ==
      WUPS_STORAGE_ERROR_NOT_FOUND)
  {
    WUPSStorageAPI_StoreInt(nullptr, "emulationState", emulationState);
  }
  else if (err == WUPS_STORAGE_ERROR_SUCCESS)
  {
    rensyshidSetEmulationState((rensyshidEmulationState)emulationState);
  }

  if (WUPSStorageAPI_SaveStorage(true) != WUPS_STORAGE_ERROR_SUCCESS)
  {
    DEBUG_FUNCTION_LINE("Failed to close storage");
  }
}

/**
  Gets called when the plugin will be unloaded.
**/
DEINITIALIZE_PLUGIN()
{
  // Remove all button combos from this plugin.
  rensyshidSetLogHandler(nullptr);
  DEBUG_FUNCTION_LINE("DEINITIALIZE_PLUGIN of re_nsyshid!");
}

/**
  Gets called when an application starts.
**/
ON_APPLICATION_START()
{
  initLogging();

  DEBUG_FUNCTION_LINE("ON_APPLICATION_START of re_nsyshid!");

  if (!WHBLogModuleInit())
  {
    WHBLogCafeInit();
    WHBLogUdpInit();
  }
}