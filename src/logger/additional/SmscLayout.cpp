#ifndef LOGGER_LIB_LOG4CPP

#if 0

#include <stdio.h>
#include "SmscLayout.h"
#include "core/buffers/TmpBuf.hpp"

namespace smsc {
namespace logger {

using namespace log4cplus;
using smsc::core::buffers::TmpBuf;

void SmscLayout::formatAndAppend(log4cplus::tostream& output, const log4cplus::spi::InternalLoggingEvent& event)
{
  // format:
  // "D dd-mm hh:mm:ss,sss TTT CatLast___:message

  //char[40] header = "L dd-mm hh:mm:ss,SSS TTT CCCCCCCCCC:";

  //log level
  char logLevel = '*';
  switch (event.getLogLevel())
  {
  case FATAL_LOG_LEVEL:
    logLevel = 'F';
    break;
  case ERROR_LOG_LEVEL:
    logLevel = 'E';
    break;
  case WARN_LOG_LEVEL:
    logLevel = 'W';
    break;
  case INFO_LOG_LEVEL:
    logLevel = 'I';
    break;
  case DEBUG_LOG_LEVEL:
    logLevel = 'D';
    break;
  case TRACE_LOG_LEVEL:
    logLevel = 'T';
    break;
  default:
    logLevel = '*';
    break;
  }

  //date&time
  char timeStr[32];
  const log4cplus::helpers::Time& time = event.getTimestamp();
  ::tm t;
  time.localtime(&t);
  const size_t timeStrLength = strftime(timeStr, sizeof(timeStr)/sizeof(timeStr[0]), "%d-%m %H:%M:%S", &t);
  timeStr[timeStrLength] = 0;

  const log4cplus::tstring& message = event.getMessage();
  //char buffer[1+1+timeStrLength+1+3+1+3+10+2+message.length()+2+16];
  const size_t bufferSize = 1+1+timeStrLength+1+3+1+3+10+2+message.length()+2+16;
  TmpBuf<char,128> buffer(bufferSize);

  const log4cplus::tstring& category = event.getLoggerName();
  const size_t categoryLength = category.length();

  //  const log4cplus::tstring& threadName = event.getThread();
  //  const size_t threadNameLength = threadName.length();
  unsigned currThread = LOG4CPLUS_GET_CURRENT_THREAD;

  const size_t printed = snprintf(buffer, bufferSize, "%c %s,%3.3u % 3.3u %10.10s: %s\n",
    logLevel, timeStr, time.usec()/1000,
    //threadNameLength > 3 ? threadName.c_str() + (threadNameLength - 3) : threadName.c_str(),
    currThread,
    categoryLength > 10 ? category.c_str() + (categoryLength - 10) : category.c_str(),
    message.c_str());

  buffer[printed >= bufferSize ? bufferSize-1 : printed] = 0;

  output << buffer;
}


std::auto_ptr<Layout> SmscLayoutFactory::createObject(const log4cplus::helpers::Properties& props)
{
  std::auto_ptr<Layout> layout(new SmscLayout());
  return layout;
}

/**
* Returns the typename of the "Layout" objects this factory creates.
*/
log4cplus::tstring SmscLayoutFactory::getTypeName()
{
  return LOG4CPLUS_TEXT("smsc::logger::SmscLayout");
}

}
}

#endif //if 0

#endif //LOGGER_LIB_LOG4CPP
