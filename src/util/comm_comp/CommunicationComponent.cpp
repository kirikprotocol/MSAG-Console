#include <string>
#include <sstream>
#include <string.h>
#include <stdlib.h>
#include <iomanip>
#include <ios>

#include "CommunicationComponent.hpp"

namespace smsc {
namespace util {
namespace comm_comp {

static int SOCKET_FINGERPRINT_IDX = 0;

static std::string setSocketFingerPrint(smsc::core::network::Socket& readySocket)
{
  char *socket_finger_print;
  std::ostringstream obuf;
  obuf << (uint_t)readySocket.getSocket() << "."
       << std::hex << std::setfill('0') << (void*)&readySocket;

  const char * str_d = obuf.str().c_str();
  socket_finger_print = ::strdup(/*obuf.str().c_str()*/str_d);
  readySocket.setData(SOCKET_FINGERPRINT_IDX, socket_finger_print);

  return std::string((char*)readySocket.getData(SOCKET_FINGERPRINT_IDX));
}

std::string getSocketFingerPrint(smsc::core::network::Socket& readySocket)
{
  if ( !readySocket.getData(SOCKET_FINGERPRINT_IDX) ) 
    return setSocketFingerPrint(readySocket);
  else
    return std::string((char*)readySocket.getData(SOCKET_FINGERPRINT_IDX));
}

void cleanupSocketFingerPrint(smsc::core::network::Socket& readySocket)
{
  ::free(readySocket.getData(SOCKET_FINGERPRINT_IDX));
  readySocket.setData(SOCKET_FINGERPRINT_IDX, 0);
}

}
}
}
