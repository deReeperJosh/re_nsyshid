#include "USBDeviceManager.hpp"
#include "re_nsyshid.hpp"

namespace re::nsyshid
{
USBDeviceManager::USBDeviceManager()
{
    emulationState = NSYSHID_EMULATION_OFF;
}

USBDeviceManager::~USBDeviceManager() = default;
}  // namespace re::nsyshid