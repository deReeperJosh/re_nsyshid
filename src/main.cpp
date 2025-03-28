#include <coreinit/debug.h>
#include <wums.h>
#include "utils/logger.h"

#include "re_nsyshid/re_nsyshid.hpp"
#include "re_nsyshid/USBDeviceManager.hpp"
#include "re_nsyshid.h"

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
  DEBUG_FUNCTION_LINE("WUMS_INITIALIZE of re_nsyshid!");
}

WUMS_APPLICATION_STARTS()
{
  /* Called whenever a new application has been started */
  initLogging();
  DEBUG_FUNCTION_LINE("WUMS_APPLICATION_STARTS of re_nsyshid!");
}

WUMS_APPLICATION_REQUESTS_EXIT()
{
  /* Called whenever a application wants to exit */
  DEBUG_FUNCTION_LINE("WUMS_APPLICATION_REQUESTS_EXIT of re_nsyshid!");
}

WUMS_APPLICATION_ENDS()
{
  /* Called whenever a application actually ends */
  DEBUG_FUNCTION_LINE("WUMS_APPLICATION_ENDS of re_nsyshid!");
  deinitLogging();
}

WUMS_RELOCATIONS_DONE()
{
  /* Called whenever the relocations have been updated, but before WUMS_APPLICATION_STARTS() */
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