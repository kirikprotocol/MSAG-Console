#include "ProtocolClass.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {
namespace messages {

ProtocolClass::ProtocolClass(uint32_t value)
{
  _protocolClass = protocol_class_t(value & 0x03); _retOnErr = ret_on_err_ind_t(value & 0x80);
}

ProtocolClass::ProtocolClass(protocol_class_t protocol_class, ret_on_err_ind_t ret_on_err)
  : _protocolClass(protocol_class), _retOnErr(ret_on_err)
{}

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

protocol_class_t
ProtocolClass::getProtocolClassValue() const
{
  return _protocolClass;
}

ret_on_err_ind_t
ProtocolClass::getRetOnErrorIndication() const
{
  return _retOnErr;
}

ProtocolClass::operator uint32_t() const
{
  return  _protocolClass | (_retOnErr << 7);
}

}}}}}
