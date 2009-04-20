/* ************************************************************************** *
 * Classes, which implement SUA message requests.
 * ************************************************************************** */
#ifndef __ELC_TCAP_SUA_REQUESTS_HPP
#ident "@(#)$Id$"
#define __ELC_TCAP_SUA_REQUESTS_HPP

//#include "eyeline/sua/libsua/MessageInfo.hpp"
#include "eyeline/sua/libsua/MessageProperties.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

using eyeline::sua::libsua::MessageProperties;
//using eyeline::sua::libsua::MessageInfo::msg_buffer_t;


class SUAUnitdataReq {
protected:
  MessageProperties _prop;
  //msg_buffer_t _dataBuf;
  //SCCPAddress  _calledParty;  //TODO:
  //SCCPAddress  _callingParty; //TODO:
  
public:
  SUAUnitdataReq()
  { }
  ~SUAUnitdataReq()
  { }

  //TODO: setters for addresses and dataBuf


  uint8_t calledAddrLen(void) const;
  const uint8_t * calledAddr(void) const;
  uint8_t callingAddrLen(void) const;
  const uint8_t * callingAddr(void) const;
  uint16_t userDataLen(void) const;
  const uint8_t * userData(void) const;

  //
  void setMsgProperties(const MessageProperties & use_prop) { _prop = use_prop; }
  //
  const MessageProperties & msgProperties(void) const { return _prop; }
};


} //provd
} //tcap
} //eyeline

#endif /* __ELC_TCAP_SUA_REQUESTS_HPP */

