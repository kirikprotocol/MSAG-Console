/* ************************************************************************** *
 * Classes, which implement SUA message indications parsing.
 * ************************************************************************** */
#ifndef __ELC_TCAP_SUA_IND_PARSERS_HPP
#ident "@(#)$Id$"
#define __ELC_TCAP_SUA_IND_PARSERS_HPP

#include "eyeline/sua/libsua/MessageInfo.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

using eyeline::sua::libsua::MessageInfo;
using eyeline::sua::libsua::SUAMessageId;

//SUA message indication parser abstract class
class SUAMessageIndAC {
protected:
  const MessageInfo * _suaMsg;
  uint8_t         _calledAddrLen;
  const uint8_t * _calledAddr;   //encoded SCCP address, refers to MessageInfo::msgData buffer
  uint8_t         _callingAddrLen;
  const uint8_t * _callingAddr;  //encoded SCCP address, refers to MessageInfo::msgData buffer
  uint16_t        _userDataLen;
  const uint8_t * _userData;     //encoded user data, refers to MessageInfo::msgData buffer

  // -------------------------------------
  // -- SUAMessageIndAC interface methods
  // -------------------------------------
  //Parses MessageInfo::msgData buffer,
  //NOTE: sets references to MessageInfo::msgData buffer
  virtual bool _parseMsgInfo(void) = 0;

public:
  SUAMessageIndAC() : _suaMsg(0)
    , _calledAddrLen(0), _calledAddr(0), _callingAddrLen(0)
    , _callingAddr(0), _userDataLen(0), _userData(0)
  { }
  virtual ~SUAMessageIndAC()
  { }

  SUAMessageId::MsgCode_e msgType(void) const
  {
    return _suaMsg ? _suaMsg->getMsgId() : SUAMessageId::UNKNOWN_MSGCODE;
  }

  //TODO: ???
  uint8_t * msgBuffer(void) const
  {
    return _suaMsg ? _suaMsg->msgData.get() : 0; 
  }

  uint8_t calledAddrLen(void) const { return _calledAddrLen; }
  const uint8_t * calledAddr(void) const { return _calledAddr; }
  uint8_t callingAddrLen(void) const { return _callingAddrLen; }
  const uint8_t * callingAddr(void) const { return _callingAddr; }
  uint16_t userDataLen(void) const { return _userDataLen; }
  const uint8_t * userData(void) const { return _userData; }

  //returns false if calledAdr cann't be represented in .ton.npi.addr form
//  bool getCalledAdr(TonNpiAddress & use_tnadr) const; //TODO:
  //returns false if callingAdr cann't be represented in .ton.npi.addr form
//  bool getCallingAdr(TonNpiAddress & use_tnadr) const; //TODO:

  //Parses MessageInfo::msgData buffer,
  //NOTE: sets references to MessageInfo::msgData buffer
  bool parseMsgInfo(const MessageInfo & use_buf)
  {
    _suaMsg = &use_buf;
    return _parseMsgInfo();
  }
};

class SUAUnitdataInd : public SUAMessageIndAC {
protected:
  enum { has_SEQUENCE_CONTROL = 0x01 };
  uint8_t   _fieldsMask;
  uint32_t  _sequenceControl;

  // -------------------------------------
  // -- SUAMessageIndAC interface methods
  // -------------------------------------
  //Parses MessageInfo::msgData buffer,
  //NOTE: sets references to MessageInfo::msgData buffer
  bool _parseMsgInfo(void);

public:
  SUAUnitdataInd()
      : _fieldsMask(0), _sequenceControl(0)
  { }
  ~SUAUnitdataInd()
  { }

  uint32_t sequenceControl(void) const
  {
    return (_fieldsMask & has_SEQUENCE_CONTROL) ? _sequenceControl : 0;
  }
};


class SUANoticeInd : public SUAMessageIndAC {
protected:
  enum { has_RETURN_REASON = 0x01, has_IMPORTANCE = 0x02 };
  uint8_t _fieldsMask;
  uint8_t _returnReason;
  uint8_t _importance;

  // -------------------------------------
  // -- SUAMessageIndAC interface methods
  // -------------------------------------
  //Parses MessageInfo::msgData buffer,
  //NOTE: sets references to MessageInfo::msgData buffer
  bool _parseMsgInfo(void);

public:
  SUANoticeInd()
      : _fieldsMask(0), _returnReason(0), _importance(0) 
  { }
  ~SUANoticeInd()
  { }

  uint32_t returnReason(void) const
  {
    return (_fieldsMask & has_RETURN_REASON) ? _returnReason : 0;
  }
  uint32_t sequenceControl(void) const
  {
    return (_fieldsMask & has_IMPORTANCE) ? _importance : 0;
  }
};

class SUAIndHandlerIface {
public:
  // ----------------------------------------
  // -- SUAIndHandlerIface interface methods
  // ----------------------------------------
  virtual bool processSuaInd(const SUAUnitdataInd & sua_ind) = 0;
  virtual bool processSuaInd(const SUANoticeInd & sua_ind) = 0;
};

} //provd
} //tcap
} //eyeline

#endif /* __ELC_TCAP_SUA_IND_PARSERS_HPP */

