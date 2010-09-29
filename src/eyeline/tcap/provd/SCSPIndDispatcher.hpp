/* ************************************************************************** *
 * TCProvider: SCCP Service Provider indications dispatching.
 * ************************************************************************** */
#ifndef __ELC_TCAP_SCSP_IND_DISPATCHER_HPP
#ident "@(#)$Id$"
#define __ELC_TCAP_SCSP_IND_DISPATCHER_HPP

#include "eyeline/tcap/provd/SCSPIndications.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

//Basic class for SCSPIndication dispatchers
class SCSPIndDispatcherAC {
public:
  SCSPIndDispatcherAC()
  { }
  virtual ~SCSPIndDispatcherAC()
  { }

  // ----------------------------------------
  // -- SCSPIndDispatcherAC interface methods
  // ----------------------------------------
  virtual bool parseMsgInfo(const SCSPMessageInfo & use_buf) = 0;
  virtual bool processInd(SCSPIndHandlerIface & use_hdl) = 0;
};

template <class _TArg /* : public SCSPMessageIndAC */>
class SCSPIndDispatcherT : public SCSPIndDispatcherAC {
protected:
  _TArg   _ind;

public:
  SCSPIndDispatcherT()
  { }
  virtual ~SCSPIndDispatcherT()
  { }

  // ----------------------------------------
  // -- SCSPIndDispatcherAC interface methods
  // ----------------------------------------
  //NOTE: sets references to SCSPMessageInfo data buffer !
  bool parseMsgInfo(const SCSPMessageInfo & use_buf)
  {
    return _ind.parseMsgInfo(use_buf);
  }
  bool processInd(SCSPIndHandlerIface & use_hdl)
  {
    return use_hdl.processSCSPInd(_ind);
  }
};

typedef SCSPIndDispatcherT<SCSPUnitdataInd> SCSPIndDispatcherUDT;
typedef SCSPIndDispatcherT<SCSPNoticeInd> SCSPIndDispatcherNTC;


class SCSPIndicationParser {
public:
  enum IKind_e { indNone = 0, indUDT, indNTC };

private:
  union {
    void *  _aligner;
    uint8_t _buf[eyeline::util::MaxSizeOf2_T<SCSPIndDispatcherUDT,
                                            SCSPIndDispatcherNTC >::VALUE];
  } _objMem;

  union {
    SCSPIndDispatcherAC  * ac;
    SCSPIndDispatcherUDT * udt;
    SCSPIndDispatcherNTC * ntc;
  } _ind;
  IKind_e _iKind;

protected:
  void resetObj(void)
  {
    if (_ind.ac) {
      _ind.ac->~SCSPIndDispatcherAC();
      _ind.ac = 0;
      _iKind = indNone;
    }
  }

  IKind_e Reset(SccpMessageId::MsgCode_e use_msg)
  {
    resetObj();
    switch (use_msg) {
    case SccpMessageId::N_UNITDATA_IND_MSGCODE: {
      _iKind = indUDT;
      _ind.ac = new (_objMem._buf) SCSPIndDispatcherUDT();
    } break;

    case SccpMessageId::N_NOTICE_IND_MSGCODE: {
      _iKind = indUDT;
      _ind.ac = new (_objMem._buf) SCSPIndDispatcherNTC();
    } break;

    default:; // _iKind = indNone;
    }
    return _iKind;
  }

public:
  SCSPIndicationParser() : _iKind(indNone)
  {
    _objMem._aligner = _ind.ac = 0;
  }
  virtual ~SCSPIndicationParser()
  { 
    resetObj();
  }

  IKind_e indKind(void) const { return _iKind; }

  bool parseMsgInfo(const SCSPMessageInfo & use_buf)
  {
    return (Reset(use_buf.getMsgId()) != indNone) ? 
            _ind.ac->parseMsgInfo(use_buf) : false;
  }

  bool processInd(SCSPIndHandlerIface & use_hdl)
  {
    return _ind.ac ? _ind.ac->processInd(use_hdl) : false;
  }
};

} //provd
} //tcap
} //eyeline

#endif /* __ELC_TCAP_SCSP_IND_DISPATCHER_HPP */

