#include "re_nsyshid.hpp"
#include "utils/logger.h"

#include <list>
#include <whb/log.h>
#include <wums.h>

namespace re::nsyshid
{
std::list<HIDClient*> clientList;

int32_t Setup()
{
  DEBUG_FUNCTION_LINE("nsyshid:HIDSetup Called");
  return 0;
}

int32_t Teardown()
{
  DEBUG_FUNCTION_LINE("nsyshid:HIDTeardown Called");
  return 0;
}

int32_t AddClient(HIDClient* client, HIDAttachCallback attachCallback)
{
  DEBUG_FUNCTION_LINE("nsyshid:HIDAddClient Called");
  client->attachCallback = attachCallback;

  clientList.push_front(client);

  return 0;
}

int32_t DelClient(HIDClient* client)
{
  DEBUG_FUNCTION_LINE("nsyshid:HIDDelClient Called");
  clientList.remove(client);

  return 0;
}

int32_t GetDescriptor(uint32_t handle, uint8_t descriptorType, uint8_t descriptorIndex,
                      uint16_t languageId, uint8_t* buffer, uint32_t bufferLength,
                      HIDCallback callback, void* userContext)
{
  // TO DO
  DEBUG_FUNCTION_LINE("nsyshid:HIDGetDescriptor Called");
  return 0;
}

int32_t SetDescriptor(uint32_t handle, uint8_t descriptorType, uint8_t descriptorIndex,
                      uint16_t languageId, uint8_t* buffer, uint32_t bufferLength,
                      HIDCallback callback, void* userContext)
{
  // TO DO
  DEBUG_FUNCTION_LINE("nsyshid:HIDSetDescriptor Called");
  return 0;
}

int32_t GetReport(uint32_t handle, uint8_t reportType, uint8_t reportId, uint8_t* buffer,
                  uint32_t bufferLength, HIDCallback callback, void* userContext)
{
  // TO DO
  DEBUG_FUNCTION_LINE("nsyshid:HIDGetReport Called");
  return 0;
}

int32_t SetReport(uint32_t handle, uint8_t reportType, uint8_t reportId, uint8_t* buffer,
                  uint32_t bufferLength, HIDCallback callback, void* userContext)
{
  // TO DO
  DEBUG_FUNCTION_LINE("nsyshid:HIDSetReport Called");
  return 0;
}

int32_t GetIdle(uint32_t handle, uint8_t interfaceIndex, uint8_t reportId, uint8_t* duration,
                HIDCallback callback, void* userContext)
{
  // TO DO
  DEBUG_FUNCTION_LINE("nsyshid:HIDGetIdle Called");
  return 0;
}

int32_t SetIdle(uint32_t handle, uint8_t interfaceIndex, uint8_t reportId, uint8_t duration,
                HIDCallback callback, void* userContext)
{
  // TO DO
  DEBUG_FUNCTION_LINE("nsyshid:HIDSetIdle Called");
  return 0;
}

int32_t GetProtocol(uint32_t handle, uint8_t interfaceIndex, uint8_t* protocol,
                    HIDCallback callback, void* userContext)
{
  // TO DO
  DEBUG_FUNCTION_LINE("nsyshid:HIDGetProtocol Called");
  return 0;
}

int32_t SetProtocol(uint32_t handle, uint8_t interfaceIndex, uint8_t protocol, HIDCallback callback,
                    void* userContext)
{
  // TO DO
  DEBUG_FUNCTION_LINE("nsyshid:HIDSetProtocol Called");
  return 0;
}

int32_t Read(uint32_t handle, uint8_t* buffer, uint32_t bufferLength, HIDCallback callback,
             void* userContext)
{
  // TO DO
  DEBUG_FUNCTION_LINE("nsyshid:HIDRead Called");
  return 0;
}

int32_t Write(uint32_t handle, uint8_t* buffer, uint32_t bufferLength, HIDCallback hc,
              void* userContext)
{
  // TO DO
  DEBUG_FUNCTION_LINE("nsyshid:HIDWrite Called");
  return 0;
}
}  // namespace re::nsyshid

WUMS_EXPORT(WUMS_FUNCTION_EXPORT, HIDSetup, re::nsyshid::Setup);
WUMS_EXPORT(WUMS_FUNCTION_EXPORT, HIDTeardown, re::nsyshid::Teardown);
WUMS_EXPORT(WUMS_FUNCTION_EXPORT, HIDAddClient, re::nsyshid::AddClient);
WUMS_EXPORT(WUMS_FUNCTION_EXPORT, HIDDelClient, re::nsyshid::DelClient);
WUMS_EXPORT(WUMS_FUNCTION_EXPORT, HIDGetDescriptor, re::nsyshid::GetDescriptor);
WUMS_EXPORT(WUMS_FUNCTION_EXPORT, HIDSetDescriptor, re::nsyshid::SetDescriptor);
WUMS_EXPORT(WUMS_FUNCTION_EXPORT, HIDGetReport, re::nsyshid::GetReport);
WUMS_EXPORT(WUMS_FUNCTION_EXPORT, HIDSetReport, re::nsyshid::SetReport);
WUMS_EXPORT(WUMS_FUNCTION_EXPORT, HIDGetIdle, re::nsyshid::GetIdle);
WUMS_EXPORT(WUMS_FUNCTION_EXPORT, HIDSetIdle, re::nsyshid::SetIdle);
WUMS_EXPORT(WUMS_FUNCTION_EXPORT, HIDGetProtocol, re::nsyshid::GetProtocol);
WUMS_EXPORT(WUMS_FUNCTION_EXPORT, HIDSetProtocol, re::nsyshid::SetProtocol);
WUMS_EXPORT(WUMS_FUNCTION_EXPORT, HIDRead, re::nsyshid::Read);
WUMS_EXPORT(WUMS_FUNCTION_EXPORT, HIDWrite, re::nsyshid::Write);
