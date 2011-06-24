#include <sys/types.h>
#include <time.h>
#include <string>
#include <ctype.h>
#include <util/Exception.hpp>

#include "callproc.hpp"
#include <logger/Logger.h>

// for test
#include <sys/types.h>
#include <string>
//

namespace smsc{
namespace misscall{

static int caseInsensitiveCompare(const std::string& l, const std::string r)
{
  std::string::size_type lsz = l.size(), rsz = r.size();

  for(int i=0; i<lsz && i<rsz; ++i) {
    int cmp = ::tolower(l[i]) - ::tolower(l[i]);
    if ( cmp ) return cmp;
  }

  return lsz - rsz;
}

static MissedCallEvent
stringParser(const std::string& inputStr)
{
  std::string::size_type pos = inputStr.find_first_of(", ");
  if ( pos == std::string::npos ) throw smsc::util::Exception("stringParser::: wrong input string format - calling num is absent");
  MissedCallEvent event;
  event.from = inputStr.substr(0, pos);

  printf("from=[%s]\n", event.from.c_str());
  while (inputStr[pos] == ' ' || inputStr[pos] == ',') ++pos;

  std::string::size_type oldPos = pos;
  pos = inputStr.find_first_of(", ", oldPos);
  if ( pos == std::string::npos ) throw smsc::util::Exception("stringParser::: wrong input string format - called num is absent");
  event.to = inputStr.substr(oldPos, pos - oldPos);
  printf("to=[%s]\n", event.to.c_str());
  while (inputStr[pos] == ' ' || inputStr[pos] == ',') ++pos;

  oldPos = pos;
  pos = inputStr.find_first_of(", ", oldPos);

  if ( pos != std::string::npos ) throw smsc::util::Exception("stringParser::: wrong input string format - unexpected delimeter found at end of string");
  std::string eventType = inputStr.substr(oldPos);
  while(!eventType.empty() && *eventType.rbegin()<=32)
  {
    eventType.erase(eventType.length()-1);
  }
  printf("eventType=[%s]\n", eventType.c_str());
  //eventType.erase(eventType.size()-2);

  if ( !caseInsensitiveCompare(eventType, "ABSENT") )
    event.cause = ABSENT;
  else if ( !caseInsensitiveCompare(eventType, "BUSY") )
    event.cause = BUSY;
  else if ( !caseInsensitiveCompare(eventType, "NOREPLY") )
    event.cause = NOREPLY;
  else if ( !caseInsensitiveCompare(eventType, "UNCOND") )
    event.cause = UNCOND;
  else if ( !caseInsensitiveCompare(eventType, "DETACH") )
    event.cause = DETACH;
  else if ( !caseInsensitiveCompare(eventType, "ALL") )
    event.cause = ALL;
  else
    throw smsc::util::Exception("stringParser::: wrong input string format - wrong event type value");

  event.time = ::time(NULL);

  return event;
}

}}

/*int main(int argc, char** argv)
{
  smsc::logger::Logger::Init();
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("test");

  char too_long_buf[256];
  while ( gets(too_long_buf) ) {
    try {
    smsc::misscall::MissedCallEvent event = smsc::misscall::stringParser(too_long_buf);
      printf("generated event=[%s,%s,%x]\n", event.from.c_str(), event.to.c_str(), event.cause);
    } catch (std::exception& ex) {
      smsc_log_error(logger, "Error: caught exception [%s]", ex.what());
    }
  }
  }*/

#include "callproc.hpp"

namespace smsc{
namespace misscall{

void
MissedCallProcessorEmulator::setHost(const std::string& host)
{
  _host = host;
}

void
MissedCallProcessorEmulator::setPort(int port)
{
  _port = (in_port_t)port;
}

MissedCallProcessorEmulator::MissedCallProcessorEmulator()
  : _logger(smsc::logger::Logger::getInstance("smsc.misscall"))
{
  smsc_log_info(_logger, "Initializing MissedCallProcessorEmulator");
  if (_serverSocket.InitServer(_host.c_str(), _port, 0, 0, 1) ||
      _serverSocket.StartServer())
  {
    char errString[128];
    sprintf(errString, "MissedCallProcessorEmulator:: Failed to init server socket %s:%u", _host.c_str(), _port);
    throw smsc::util::SystemError(errString);
  }
}

int MissedCallProcessorEmulator::run()
{
  _serverSocket.StartServer();
  core::network::Socket* clntSocket=NULL;
  char cmdBuf[128];
  try {
    while (true) {
      if(clntSocket) delete clntSocket;
      clntSocket = _serverSocket.Accept();
      if (clntSocket)
      {
        smsc_log_info(_logger, "Connection accepted" );

        while ( clntSocket->Gets(cmdBuf, sizeof(cmdBuf)) > 0 ) {
          smsc::misscall::MissedCallEvent event = stringParser(cmdBuf);
          fireMissedCallEvent(event);
        }
      }
    }
  } catch (std::exception &ex) {
    smsc_log_error(_logger, "MissedCallProcessorEmulator::run::: caught exception [%s]", ex.what());
  }
  return 0;
}

std::string MissedCallProcessorEmulator::_host;
in_port_t MissedCallProcessorEmulator::_port;

}}
