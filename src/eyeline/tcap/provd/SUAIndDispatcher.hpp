/* ************************************************************************** *
 * Classes, which implement SUA message indications dispatching.
 * ************************************************************************** */
#ifndef __ELC_TCAP_SUA_IND_DISPATCHER_HPP
#ident "@(#)$Id$"
#define __ELC_TCAP_SUA_IND_DISPATCHER_HPP

#include "eyeline/tcap/provd/SUAIndications.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

//Basic class for SUAIndication dispatchers
class SUAIndDispatcherAC {
public:
  SUAIndDispatcherAC()
  { }
  virtual ~SUAIndDispatcherAC()
  { }

  // ----------------------------------------
  // -- SUAIndDispatcherAC interface methods
  // ----------------------------------------
  virtual bool parseMsgInfo(const MessageInfo & use_buf) = 0;
  virtual bool processInd(SUAIndHandlerIface & use_hdl) = 0;
};

template <class _TArg /* : public SUAMessageIndAC */>
class SUAIndDispatcherT : public SUAIndDispatcherAC {
protected:
  _TArg   _ind;

public:
  SUAIndDispatcherT()
  { }
  virtual ~SUAIndDispatcherT()
  { }

  // ----------------------------------------
  // -- SUAIndDispatcherAC interface methods
  // ----------------------------------------
  //NOTE: sets references to MessageInfo data buffer !
  bool parseMsgInfo(const MessageInfo & use_buf)
  {
    return _ind.parseMsgInfo(use_buf);
  }
  bool processInd(SUAIndHandlerIface & use_hdl)
  {
    return use_hdl.processSuaInd(_ind);
  }
};

typedef SUAIndDispatcherT<SUAUnitdataInd> SUAIndDispatcherUDT;
typedef SUAIndDispatcherT<SUANoticeInd> SUAIndDispatcherNTC;


class SUAIndicationParser {
public:
  enum MsgKind_e { indNone = 0, indUDT, indNTC };

private:
  uint8_t objMem[eyeline::util::MaxSizeOf2_T< 
                                 SUAIndDispatcherUDT, SUAIndDispatcherNTC >::VALUE];
  union {
    SUAIndDispatcherAC  * ac;
    SUAIndDispatcherUDT * udt;
    SUAIndDispatcherNTC * ntc;
  } _ind;
  MsgKind_e _iKind;

protected:
  void resetObj(void)
  {
    if (_ind.ac) {
      _ind.ac->~SUAIndDispatcherAC();
      _ind.ac = 0;
      _iKind = indNone;
    }
  }

  MsgKind_e Reset(SUAMessageId::MsgCode_e use_msg)
  {
    resetObj();
    switch (use_msg) {
    case SUAMessageId::N_UNITDATA_IND_MSGCODE: {
      _iKind = indUDT;
      _ind.ac = new(objMem)SUAIndDispatcherUDT();
    } break;

    case SUAMessageId::N_NOTICE_IND_MSGCODE: {
      _iKind = indUDT;
      _ind.ac = new(objMem)SUAIndDispatcherNTC();
    } break;

    default:; // _iKind = indNone;
    }
    return _iKind;
  }

public:
  SUAIndicationParser() : _iKind(indNone)
  {
    _ind.ac = 0;
  }
  virtual ~SUAIndicationParser()
  { 
    resetObj();
  }

  MsgKind_e indKind(void) const { return _iKind; }

  bool parseMsgInfo(const MessageInfo & use_buf)
  {
    return (Reset(use_buf.getMsgId()) != indNone) ? 
            _ind.ac->parseMsgInfo(use_buf) : false;
  }

  bool processInd(SUAIndHandlerIface & use_hdl)
  {
    return _ind.ac ? _ind.ac->processInd(use_hdl) : false;
  }
};

} //provd
} //tcap
} //eyeline

#endif /* __ELC_TCAP_SUA_IND_DISPATCHER_HPP */

