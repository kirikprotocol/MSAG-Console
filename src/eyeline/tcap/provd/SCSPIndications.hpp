/* ************************************************************************** *
 * TCProvider: SCCP Service Provider indications parsing. 
 * ************************************************************************** */
#ifndef __ELC_TCAP_SCSP_IND_PARSERS_HPP
#ident "@(#)$Id$"
#define __ELC_TCAP_SCSP_IND_PARSERS_HPP

#include "eyeline/sccp/SCCPConsts.hpp"
#include "eyeline/ss7na/libsccp/MessageInfo.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

using eyeline::ss7na::libsccp::SccpMessageId;

typedef eyeline::ss7na::libsccp::MessageInfo SCSPMessageInfo;

//SUA message indication parser abstract class
class SCSPMessageIndAC {
protected:
  const SCSPMessageInfo * _scspMsg;
  uint8_t         _calledAddrLen;
  const uint8_t * _calledAddr;   //encoded SCCP address, refers to MessageInfo::msgData buffer
  uint8_t         _callingAddrLen;
  const uint8_t * _callingAddr;  //encoded SCCP address, refers to MessageInfo::msgData buffer
  uint16_t        _userDataLen;
  const uint8_t * _userData;     //encoded user data, refers to MessageInfo::msgData buffer

  // -------------------------------------
  // -- SCSPMessageIndAC interface methods
  // -------------------------------------
  //Parses MessageInfo::msgData buffer,
  //NOTE: sets references to MessageInfo::msgData buffer
  virtual bool _parseMsgInfo(void) = 0;

public:
  SCSPMessageIndAC() : _scspMsg(0)
    , _calledAddrLen(0), _calledAddr(0), _callingAddrLen(0)
    , _callingAddr(0), _userDataLen(0), _userData(0)
  { }
  virtual ~SCSPMessageIndAC()
  { }

  SccpMessageId::MsgCode_e msgType(void) const
  {
    return _scspMsg ? _scspMsg->getMsgId() : SccpMessageId::UNKNOWN_MSGCODE;
  }

  uint8_t * msgBuffer(void) const
  {
    return _scspMsg ? _scspMsg->msgData.get() : 0; 
  }

  uint8_t calledAddrLen(void) const { return _calledAddrLen; }
  const uint8_t * calledAddr(void) const { return _calledAddr; }

  uint8_t callingAddrLen(void) const { return _callingAddrLen; }
  const uint8_t * callingAddr(void) const { return _callingAddr; }

  uint16_t userDataLen(void) const { return _userDataLen; }
  const uint8_t * userData(void) const { return _userData; }

  //Parses MessageInfo::msgData buffer,
  //NOTE: sets references to MessageInfo::msgData buffer
  bool parseMsgInfo(const SCSPMessageInfo & use_buf)
  {
    _scspMsg = &use_buf;
    return _parseMsgInfo();
  }
};

class SCSPUnitdataInd : public SCSPMessageIndAC {
protected:
  enum Options_e { has_SEQUENCE_CONTROL = 0x01 };

  uint8_t   _fieldsMask;
  uint32_t  _sequenceControl;

  // -------------------------------------
  // -- SCSPMessageIndAC interface methods
  // -------------------------------------
  //Parses MessageInfo::msgData buffer,
  //NOTE: sets references to MessageInfo::msgData buffer
  bool _parseMsgInfo(void);

public:
  SCSPUnitdataInd()
      : _fieldsMask(0), _sequenceControl(0)
  { }
  ~SCSPUnitdataInd()
  { }

  uint32_t sequenceControl(void) const
  {
    return (_fieldsMask & has_SEQUENCE_CONTROL) ? _sequenceControl : 0;
  }
};


class SCSPNoticeInd : public SCSPMessageIndAC {
protected:
  enum Options_e { has_IMPORTANCE = 0x01 };

  uint8_t             _fieldsMask;
  uint8_t             _importance;  //3 bits value
  sccp::ReturnCause_t _returnReason;
  // -------------------------------------
  // -- SCSPMessageIndAC interface methods
  // -------------------------------------
  //Parses SCSPMessageInfo::msgData buffer,
  //NOTE: sets references to SCSPMessageInfo::msgData buffer
  bool _parseMsgInfo(void);

public:
  SCSPNoticeInd()
      : _fieldsMask(0), _importance(0), _returnReason(0)
  { }
  ~SCSPNoticeInd()
  { }

  sccp::ReturnCause_e getCauseId(void) const
  {
    return sccp::ReturnCause::value2Id(_returnReason);
  }
  sccp::ReturnCause_t getCauseValue(void) const
  {
    return _returnReason;
  }
  uint8_t hasImportance(void) const
  {
    return (_fieldsMask & has_IMPORTANCE) ? _importance : 0;
  }
};

class SCSPIndHandlerIface {
protected:
  virtual ~SCSPIndHandlerIface();

public:
  // ----------------------------------------
  // -- SCSPIndHandlerIface interface methods
  // ----------------------------------------
  virtual bool processSCSPInd(const SCSPUnitdataInd & scsp_ind)
    /*throw(std::exception)*/ = 0;
  virtual bool processSCSPInd(const SCSPNoticeInd & scsp_ind)
    /*throw(std::exception)*/ = 0;
};

} //provd
} //tcap
} //eyeline

#endif /* __ELC_TCAP_SCSP_IND_PARSERS_HPP */

