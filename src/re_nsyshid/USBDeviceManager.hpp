#pragma once

#include "re_nsyshid.h"

namespace re::nsyshid
{

class USBDeviceManager
{
public:
  USBDeviceManager();
  virtual ~USBDeviceManager();

private:
  void Reset();

public:  // custom
  void SetEmulationState(rensyshidEmulationState state)
  {
    emulationState = state;
    pendingRemove = state == NSYSHID_EMULATION_OFF;
  }

  rensyshidEmulationState GetEmulationState() const { return emulationState; }

private:  // custom
  rensyshidEmulationState emulationState;
  bool pendingRemove;
};

}  // namespace re::nsyshid