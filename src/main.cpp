#include <coreinit/debug.h>
#include <wums.h>
#include "utils/logger.h"

#include "re_nsyshid.h"
#include "re_nsyshid/USBDeviceManager.hpp"
#include "re_nsyshid/re_nsyshid.hpp"

#include "utils/LogHandler.hpp"

WUMS_MODULE_EXPORT_NAME("nsyshid");
WUMS_MODULE_AUTHOR("deReeperJosh");
WUMS_MODULE_VERSION("0.1");
WUMS_MODULE_LICENSE("GPL");
WUMS_MODULE_DESCRIPTION("A nsyshid reimplementation with support for HID Device Emulation");

/**
 * If this modules depends on another module (e.g. the FunctionPatcherModule) you can add a
 * dependency to that module. This will enforce that the other module has been loaded and
 * initialized before this module is initialized. If the other module is not loaded, this module
 * also fails to load. Usage: WUMS_DEPENDS_ON(export_name) Example:
 * WUMS_DEPENDS_ON(homebrew_functionpatcher)
 */

WUMS_INITIALIZE(/*wums_app_init_args_t*/ args)
{
  initLogging();
  LogHandler::Info("WUMS_INITIALIZE of re_nsyshid!");

  // Information about the module can be get via the (optional) argument
  module_information_t* module_information = args.module_information;

  if (module_information == nullptr)
  {
    LogHandler::Info("Failed to get module_information pointer.");
  }
  // Make sure the module is using a compatible version with the loader
  if (module_information->version != MODULE_INFORMATION_VERSION)
  {
    LogHandler::Info("The module information struct version does not match.");
  }
}

WUMS_APPLICATION_STARTS()
{
  /* Called whenever a new application has been started */
  initLogging();
  LogHandler::Info("WUMS_APPLICATION_STARTS of re_nsyshid!");
}

WUMS_APPLICATION_REQUESTS_EXIT()
{
  /* Called whenever a application wants to exit */
  LogHandler::Info("WUMS_APPLICATION_REQUESTS_EXIT of re_nsyshid!");
}

WUMS_APPLICATION_ENDS()
{
  /* Called whenever a application actually ends */
  LogHandler::Info("WUMS_APPLICATION_ENDS of re_nsyshid!");
  deinitLogging();
}

WUMS_RELOCATIONS_DONE()
{
  /* Called whenever the relocations have been updated, but before WUMS_APPLICATION_STARTS() */
  LogHandler::Info("WUMS_RELOCATIONS_DONE of re_nsyshid!");
}

rensyshidEmulationState rensyshidGetEmulationState(void)
{
  return re::nsyshid::usbDeviceManager.GetEmulationState();
}

void rensyshidSetEmulationState(rensyshidEmulationState state)
{
  LogHandler::Info("Module: Updated emulation state to: %d", state);
  re::nsyshid::usbDeviceManager.SetEmulationState(state);
}

WUMS_EXPORT_FUNCTION(rensyshidGetEmulationState);
WUMS_EXPORT_FUNCTION(rensyshidSetEmulationState);