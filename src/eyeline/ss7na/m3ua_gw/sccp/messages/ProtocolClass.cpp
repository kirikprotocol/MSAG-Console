#include "ProtocolClass.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace messages {

std::string
ProtocolClass::toString() const
{
  std::string strBuf;
  if ( _protocolClass == CLASS0_CONNECIONLESS )
    strBuf = "Class 0 (connectionless service)";
  else if ( _protocolClass == CLASS1_CONNECIONLESS )
    strBuf = "Class 1 (connectionless service)";
  if ( _protocolClass == CLASS2_CONNECION_ORIENTED )
    strBuf = "Class 2(connection-oriented service)";
  if ( _protocolClass == CLASS3_CONNECION_ORIENTED )
    strBuf = "Class 3(connection-oriented service)";

  if ( _retOnErr )
    strBuf += ",Return message on error";
  else
    strBuf += ",No special options";

  return strBuf;
}

}}}}}
