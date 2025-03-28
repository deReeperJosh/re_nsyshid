#include "LogHandler.hpp"
#include "re_nsyshid/Lock.hpp"

#include <cstdarg>
#include <re_nsyshid.h>
#include <string>
#include <wums.h>

#include <coreinit/mutex.h>

static renyshsidLogHandler logHandler;
static char logBuf[0x200];
static OSMutex logMutex;

static void Log(rensyshidLogVerbosity verb, const char* fmt, va_list arg)
{
  Lock lock(&logMutex);

  std::vsnprintf(logBuf, sizeof(logBuf), fmt, arg);

  if (logHandler)
  {
    logHandler(verb, logBuf);
  }
}

void LogHandler::Init(void)
{
  OSInitMutex(&logMutex);
}

void LogHandler::Info(const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  Log(NSYSHID_LOG_VERBOSITY_INFO, fmt, args);
  va_end(args);
}

void LogHandler::Warn(const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  Log(NSYSHID_LOG_VERBOSITY_WARN, fmt, args);
  va_end(args);
}

void LogHandler::Error(const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  Log(NSYSHID_LOG_VERBOSITY_ERROR, fmt, args);
  va_end(args);
}

void rensyshidSetLogHandler(renyshsidLogHandler handler)
{
  logHandler = handler;

  LogHandler::Info("Module: Log Handler %s", handler ? "set" : "cleared");
  LogHandler::Info("Module: Version 0.1");
}

WUMS_EXPORT_FUNCTION(rensyshidSetLogHandler);
