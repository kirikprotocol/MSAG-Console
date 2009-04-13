/* ************************************************************************** *
 * TCAP dialogue indications dispatcher.
 * ************************************************************************** */
#ifndef __ELC_TCAP_INDICATIONS_DISPATCHER_HPP
#ident "@(#)$Id$"
#define __ELC_TCAP_INDICATIONS_DISPATCHER_HPP

#include "eyeline/tcap/provd/TDlgIndications.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

//Basic abstract class for all indication dispatchers
class TDlgIndicationDispatcherAC {
public:
  TDlgIndicationDispatcherAC()
  { }
  virtual ~TDlgIndicationDispatcherAC()
  { }

  // -----------------------------------------------
  // -- TDlgIndicationDispatcherAC interface methods
  // -----------------------------------------------
  //calls appropriate dispatching method of TDlgHandler
  virtual bool dispatchTInd(TDlgHandlerIface * use_hdl) = 0;
  //
  virtual TDialogueIndicationPrimitive & TInd(void) = 0;
};

template <class _TArg /* pubic: TDialogueIndicationPrimitive */>
class TDlgIndicationDispatcherT : public TDlgIndicationDispatcherAC {
protected:
  _TArg _tInd;

public:
  TDlgIndicationDispatcherT(TCAPMessage & use_tmsg)
    : _tInd(use_tmsg)
  { }
  virtual ~TDlgIndicationDispatcherT()
  { }
  // -----------------------------------------------
  // -- TDlgIndicationDispatcherAC interface methods
  // -----------------------------------------------
  bool dispatchTInd(TDlgHandlerIface * use_hdl)
  {
    return use_hdl->updateDialogue(_tInd);
  }
  TDialogueIndicationPrimitive & TInd(void) { return _tInd; }
};

class TBeginIndDispatcher : public TDlgIndicationDispatcherT<TC_Begin_Ind> {
public:
  TBeginIndDispatcher(TCAPMessage & use_tmsg)
    : TDlgIndicationDispatcherT<TC_Begin_Ind>(use_tmsg)
  { }
  //
  void bindSUAInd(const SUAUnitdataInd & sua_ind)
  {
    _tInd.bindSUAInd(sua_ind);
  }
//  TC_Begin_Ind & TBeginInd(void) { return _tInd; }
};
//
class TContIndDispatcher : public TDlgIndicationDispatcherT<TC_Cont_Ind> {
public:
  TContIndDispatcher(TCAPMessage & use_tmsg)
    : TDlgIndicationDispatcherT<TC_Cont_Ind>(use_tmsg)
  { }
  //
  void bindSUAInd(const SUAUnitdataInd & sua_ind)
  {
    _tInd.bindSUAInd(sua_ind);
  }
//  TC_Cont_Ind & TContInd(void) { return _tInd; }
};
//
class TEndIndDispatcher : public TDlgIndicationDispatcherT<TC_End_Ind> {
public:
  TEndIndDispatcher(TCAPMessage & use_tmsg)
    : TDlgIndicationDispatcherT<TC_End_Ind>(use_tmsg)
  { }
  //
  void bindSUAInd(const SUAUnitdataInd & sua_ind)
  {
    _tInd.bindSUAInd(sua_ind);
  }
//  TC_End_Ind & TEndInd(void) { return _tInd; }
};
//
class TPAbortIndDispatcher : public TDlgIndicationDispatcherT<TC_PAbort_Ind> {
public:
  TPAbortIndDispatcher(TCAPMessage & use_tmsg)
    : TDlgIndicationDispatcherT<TC_PAbort_Ind>(use_tmsg)
  { }
  //
  void bindSUAInd(const SUAUnitdataInd & sua_ind)
  {
    _tInd.bindSUAInd(sua_ind);
  }
//  TC_PAbort_Ind & TPAbortInd(void) { return _tInd; }
};
//
class TUAbortIndDispatcher : public TDlgIndicationDispatcherT<TC_UAbort_Ind> {
public:
  TUAbortIndDispatcher(TCAPMessage & use_tmsg)
    : TDlgIndicationDispatcherT<TC_UAbort_Ind>(use_tmsg)
  { }
  //
  void bindSUAInd(const SUAUnitdataInd & sua_ind)
  {
    _tInd.bindSUAInd(sua_ind);
  }
//  TC_UAbort_Ind & TUAbortInd(void) { return _tInd; }
};
//
class TNoticeIndDispatcher : public TDlgIndicationDispatcherT<TC_Notice_Ind> {
public:
  TNoticeIndDispatcher(TCAPMessage & use_tmsg)
    : TDlgIndicationDispatcherT<TC_Notice_Ind>(use_tmsg)
  { }
  //
  void bindSUAInd(const SUANoticeInd & sua_ind)
  {
    _tInd.bindSUAInd(sua_ind);
  }
//  TC_Notice_Ind & TNoticeInd(void) { return _tInd; }
};


//TCAP dialogue indications dispatcher: implements functionality on creating
//  TCAP dialogue indications from received SUA message and dispatching the
//  former to appropriate TCAP dialogue handler.
class TDlgIndicationDispatcher : public SUAIndHandlerIface {
public:
  enum IKind_e {
    indNone = 0, indTBegin, indTCont, indTEnd, indTPAbrt, indTUAbrt, indTNotice
  };

private:
  uint8_t objMem[eyeline::util::MaxSizeOf6_T<
                                  TBeginIndDispatcher, TContIndDispatcher,
                                  TEndIndDispatcher, TUAbortIndDispatcher,
                                  TPAbortIndDispatcher, TNoticeIndDispatcher
                                >::VALUE];
  union { //NOTE: this union provides correct pointer translation 
          //only in case of SINLE inheritance from TDlgIndicationDispatcherAC
    TDlgIndicationDispatcherAC * pAc;
    TBeginIndDispatcher *  tBegin;
    TContIndDispatcher *   tCont;
    TEndIndDispatcher *    tEnd;
    TPAbortIndDispatcher * tPAbrt;
    TUAbortIndDispatcher * tUAbrt;
    TNoticeIndDispatcher * tNotice;
  }           _dsp;
  IKind_e     _kind;
  MessageInfo _msgSUA;
  TCAPMessage _msgTC; //has references to _msgSUA.msgData

protected:
  void resetObj(void)
  {
    if (_dsp.pAc) {
      _dsp.pAc->~TDlgIndicationDispatcherAC();
      _dsp.pAc = 0;
      _msgTC.Reset();
    }
  }

  void Reset(IKind_e use_ikind = indNone);

public:
  TDlgIndicationDispatcher(IKind_e use_ikind = indNone)
    : _kind(use_ikind)
  {
    _dsp.pAc = 0;
    if (_kind != indNone)
      Reset(_kind);
  }

  ~TDlgIndicationDispatcher()
  {
    resetObj();
    _kind = indNone;
  }

  IKind_e indKind(void) const { return _kind; }

  MessageInfo & suaMsgBuf(void) { return _msgSUA; }

  //Parses SUA mesage buffer, decodes TCAP message, composes appropriate TCAP
  //Indication prinitive and creates its dispatcher (by calling processSuaInd())
  bool processSuaMsgBuf(void);

  //TODO: may be there is a reason to move to protected section ?
  // ----------------------------------------
  // -- SUAIndHandlerIface interface methods
  // ----------------------------------------
  bool processSuaInd(const SUAUnitdataInd & sua_ind);
  bool processSuaInd(const SUANoticeInd & sua_ind);

  // ------------------------------------------------------------
  // NOTE: following methods should be called ONLY after one of
  // process*() methods was executed !
  // ------------------------------------------------------------
  bool dispatchTInd(TDlgHandlerIface * use_hdl)
  {
    return _dsp.pAc ? _dsp.pAc->dispatchTInd(use_hdl) : false;
  }
  //
  TDialogueIndicationPrimitive * TInd(void)
  {
    return _dsp.pAc ? &(_dsp.pAc->TInd()) : 0;
  }
};

} //provd
} //tcap
} //eyeline

#endif /* __ELC_TCAP_INDICATIONS_DISPATCHER_HPP */

