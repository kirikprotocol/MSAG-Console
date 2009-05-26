/* ************************************************************************** *
 * TCAP dialogue indications dispatcher.
 * ************************************************************************** */
#ifndef __ELC_TCAP_INDICATIONS_DISPATCHER_HPP
#ident "@(#)$Id$"
#define __ELC_TCAP_INDICATIONS_DISPATCHER_HPP

#include "eyeline/tcap/provd/TDlgIndComposers.hpp"
#include "eyeline/tcap/provd/SUAIndications.hpp"
#include "eyeline/tcap/provd/TCAPIndicationsProcessor.hpp"
#include "eyeline/tcap/proto/TCAPMessage.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

using eyeline::tcap::proto::TCAPMessage;

//Basic abstract class for all indication dispatchers
class TDlgIndicationDispatcherAC {
protected:
  TCAPMessage & _tMsg;

public:
  TDlgIndicationDispatcherAC(TCAPMessage & use_tmsg)
    : _tMsg(use_tmsg)
  { }
  virtual ~TDlgIndicationDispatcherAC()
  { }

  // -----------------------------------------------
  // -- TDlgIndicationDispatcherAC interface methods
  // -----------------------------------------------
  //calls appropriate dispatching method of TDlgHandler
  virtual bool dispatchTInd(TCAPIndicationsProcessor * use_hdl, unsigned int suaConnectNum) = 0;
};


template <class T_IND_COMPOSER_Arg /* pubic: TDialogueIndicationComposerT<> */>
class TDlgIndicationDispatcherT : public TDlgIndicationDispatcherAC {
protected:
  T_IND_COMPOSER_Arg _tInit;

public:
  TDlgIndicationDispatcherT(TCAPMessage & use_tmsg)
    : TDlgIndicationDispatcherAC(use_tmsg)
    , _tInit(use_tmsg.ACDefined())
  {
    if (_tMsg.usrInfo() && !_tMsg.usrInfo()->empty())
      _tMsg.usrInfo()->export2TDlgUI(_tInit.getUserInfo());
  }
  virtual ~TDlgIndicationDispatcherT()
  { }

  // -----------------------------------------------
  // -- TDlgIndicationDispatcherAC interface methods
  // -----------------------------------------------
  bool dispatchTInd(TCAPIndicationsProcessor * use_hdl, unsigned int suaConnectNum)
  {
    return use_hdl->updateDialogue(_tInit, suaConnectNum);
  }

};

// -----------------------------------------------
// -- specialization: TC_Begin_Ind dispatcher
// -----------------------------------------------
class TBeginIndDispatcher : public TDlgIndicationDispatcherT<TBeginIndComposer> {
public:
  TBeginIndDispatcher(TCAPMessage & use_tmsg)
    : TDlgIndicationDispatcherT<TBeginIndComposer>(use_tmsg)
  {
    if (!use_tmsg.CompList().empty())
      _tInit.setCompList(&use_tmsg.CompList());
  }
  //
  bool bindSUAInd(const SUAUnitdataInd & sua_ind)
  {
    //TODO: Compose TC_Begin_Ind
    return false;
  }
};

// -----------------------------------------------
// -- specialization: TC_Cont_Ind dispatcher
// -----------------------------------------------
class TContIndDispatcher : public TDlgIndicationDispatcherT<TContIndComposer> {
public:
  TContIndDispatcher(TCAPMessage & use_tmsg)
    : TDlgIndicationDispatcherT<TContIndComposer>(use_tmsg)
  {
    if (!use_tmsg.CompList().empty())
      _tInit.setCompList(&use_tmsg.CompList());
  }
  //
  bool bindSUAInd(const SUAUnitdataInd & sua_ind)
  {
    //TODO: Compose TC_Cont_Ind 
    return false;
  }
};

// -----------------------------------------------
// -- specialization: TC_End_Ind dispatcher
// -----------------------------------------------
class TEndIndDispatcher : public TDlgIndicationDispatcherT<TEndIndComposer> {
public:
  TEndIndDispatcher(TCAPMessage & use_tmsg)
    : TDlgIndicationDispatcherT<TEndIndComposer>(use_tmsg)
  {
    if (!use_tmsg.CompList().empty())
      _tInit.setCompList(&use_tmsg.CompList());
  }
  //
  bool bindSUAInd(const SUAUnitdataInd & sua_ind)
  {
    //TODO: Compose TC_End_Ind 
    return false;
  }
};

// -----------------------------------------------
// -- specialization: TC_PAbort_Ind dispatcher
// -----------------------------------------------
class TPAbortIndDispatcher : public TDlgIndicationDispatcherT<TPAbortIndComposer> {
public:
  TPAbortIndDispatcher(TCAPMessage & use_tmsg)
    : TDlgIndicationDispatcherT<TPAbortIndComposer>(use_tmsg)
  { }
  //
  bool bindSUAInd(const SUAUnitdataInd & sua_ind)
  {
    //TODO: Compose TC_PAbort_Ind 
    return false;
  }
};

// -----------------------------------------------
// -- specialization: TC_UAbort_Ind dispatcher
// -----------------------------------------------
class TUAbortIndDispatcher : public TDlgIndicationDispatcherT<TUAbortIndComposer> {
public:
  TUAbortIndDispatcher(TCAPMessage & use_tmsg)
    : TDlgIndicationDispatcherT<TUAbortIndComposer>(use_tmsg)
  { }
  //
  bool bindSUAInd(const SUAUnitdataInd & sua_ind)
  {
    //TODO: Compose TC_UAbort_Ind 
    return false;
  }
};

// -----------------------------------------------
// -- specialization: TC_Notice_Ind dispatcher
// -----------------------------------------------
class TNoticeIndDispatcher : public TDlgIndicationDispatcherT<TNoticeIndComposer> {
public:
  TNoticeIndDispatcher(TCAPMessage & use_tmsg)
    : TDlgIndicationDispatcherT<TNoticeIndComposer>(use_tmsg)
  {
    if (!use_tmsg.CompList().empty())
      _tInit.setCompList(&use_tmsg.CompList());
  }
  //
  bool bindSUAInd(const SUANoticeInd & sua_ind)
  {
    //TODO: Compose TC_Notice_Ind 
    return false;
  }
};


//TCAP dialogue indications dispatcher: implements functionality on creating
//  TCAP dialogue indications from received SUA message and dispatching the
//  former to appropriate TCAP dialogue handler.
class TDlgIndicationDispatcher : SUAIndHandlerIface {
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

  // ----------------------------------------
  // -- SUAIndHandlerIface interface methods
  // ----------------------------------------
  bool processSuaInd(const SUAUnitdataInd & sua_ind);
  bool processSuaInd(const SUANoticeInd & sua_ind);

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

  MessageInfo * suaMsgBuf(void) { return &_msgSUA; }

  //Parses SUA mesage buffer, decodes TCAP message, composes appropriate TCAP
  //Indication prinitive and creates its dispatcher (by calling processSuaInd())
  bool processSuaMsgBuf(void);

  // ------------------------------------------------------------
  // NOTE: following methods should be called ONLY after one of
  // process*() methods was executed !
  // ------------------------------------------------------------
  bool dispatchTInd(TCAPIndicationsProcessor * use_hdl)
  {
    return _dsp.pAc ? _dsp.pAc->dispatchTInd(use_hdl, _msgSUA.suaConnectNum) : false;
  }
};

} //provd
} //tcap
} //eyeline

#endif /* __ELC_TCAP_INDICATIONS_DISPATCHER_HPP */

