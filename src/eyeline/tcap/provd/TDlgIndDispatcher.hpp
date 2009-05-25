/* ************************************************************************** *
 * TCAP dialogue indications dispatcher.
 * ************************************************************************** */
#ifndef __ELC_TCAP_INDICATIONS_DISPATCHER_HPP
#ident "@(#)$Id$"
#define __ELC_TCAP_INDICATIONS_DISPATCHER_HPP

#include "eyeline/tcap/TDialogueIndicationPrimitives.hpp"
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

template <class _TArg /* pubic: TDialogueIndicationPrimitive */>
class TDlgIndicationComposerT : public _TArg {
public:
  TDlgIndicationComposerT(const EncodedOID * use_ac = NULL,
                          ROSComponentsList * use_comps = NULL);

  _TArg & TInd(void);
};

template <class _TArg /* pubic: TDialogueIndicationComposerT<> */>
class TDlgIndicationDispatcherT : public TDlgIndicationDispatcherAC {
protected:
  _TArg _tInit; //indication composer

public:
  TDlgIndicationDispatcherT(TCAPMessage & use_tmsg)
    : TDlgIndicationDispatcherAC(use_tmsg)
    , _tInit(use_tmsg.ACDefined(), use_tmsg.CompList().empty() ? NULL : &use_tmsg.CompList())
  {
    if (_tMsg.usrInfo() && !_tMsg.usrInfo()->empty())
      _tMsg.usrInfo()->export2TDlgUI(_tInit.TInd().UserInfo());
  }
  virtual ~TDlgIndicationDispatcherT()
  { }

  // -----------------------------------------------
  // -- TDlgIndicationDispatcherAC interface methods
  // -----------------------------------------------
  bool dispatchTInd(TCAPIndicationsProcessor * use_hdl, unsigned int suaConnectNum)
  {
    return use_hdl->updateDialogue(_tInit.TInd(), suaConnectNum);
  }

};

// -----------------------------------------------
// -- specialization: TC_Begin_Ind dispatcher
// -----------------------------------------------
template <>
class TDlgIndicationComposerT<TC_Begin_Ind> : public TC_Begin_Ind {
public:
  TDlgIndicationComposerT(const EncodedOID * use_ac, ROSComponentsList * use_comps)
    : TC_Begin_Ind(use_ac, use_comps)
  { }

  TC_Begin_Ind & TInd(void) { return *this; }
};
//
class TBeginIndComposer : public TDlgIndicationComposerT<TC_Begin_Ind> {
public:
  TBeginIndComposer(const EncodedOID * use_ac = NULL,
                    ROSComponentsList * use_comps = NULL)
    : TDlgIndicationComposerT<TC_Begin_Ind>(use_ac, use_comps)
  { }

  void setOrigAddress(const SCCPAddress & use_adr) { _orgAdr = use_adr; }
  void setDestAddress(const SCCPAddress & use_adr) { _dstAdr = use_adr; }
};

class TBeginIndDispatcher : public TDlgIndicationDispatcherT<TBeginIndComposer> {
public:
  TBeginIndDispatcher(TCAPMessage & use_tmsg)
    : TDlgIndicationDispatcherT<TBeginIndComposer>(use_tmsg)
  { }
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
template <>
class TDlgIndicationComposerT<TC_Cont_Ind> : public TC_Cont_Ind {
public:
  TDlgIndicationComposerT(const EncodedOID * use_ac, ROSComponentsList * use_comps)
    : TC_Cont_Ind(use_ac, use_comps)
  { }

  TC_Cont_Ind & TInd(void) { return *this; }
};
//
class TContIndComposer : public TDlgIndicationComposerT<TC_Cont_Ind> {
public:
  TContIndComposer(const EncodedOID * use_ac = NULL,
                    ROSComponentsList * use_comps = NULL)
    : TDlgIndicationComposerT<TC_Cont_Ind>(use_ac, use_comps)
  { }

  void setOrigAddress(const SCCPAddress & use_adr) { _orgAdr = use_adr; }
  void setDestAddress(const SCCPAddress & use_adr) { _dstAdr = use_adr; }
};
//
class TContIndDispatcher : public TDlgIndicationDispatcherT<TContIndComposer> {
public:
  TContIndDispatcher(TCAPMessage & use_tmsg)
    : TDlgIndicationDispatcherT<TContIndComposer>(use_tmsg)
  { }
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
template <>
class TDlgIndicationComposerT<TC_End_Ind> : public TC_End_Ind {
public:
  TDlgIndicationComposerT(const EncodedOID * use_ac, ROSComponentsList * use_comps)
    : TC_End_Ind(use_ac, use_comps)
  { }

  TC_End_Ind & TInd(void) { return *this; }
};
//
class TEndIndComposer : public TDlgIndicationComposerT<TC_End_Ind> {
public:
  TEndIndComposer(const EncodedOID * use_ac = NULL,
                    ROSComponentsList * use_comps = NULL)
    : TDlgIndicationComposerT<TC_End_Ind>(use_ac, use_comps)
  { }
};
//
class TEndIndDispatcher : public TDlgIndicationDispatcherT<TEndIndComposer> {
public:
  TEndIndDispatcher(TCAPMessage & use_tmsg)
    : TDlgIndicationDispatcherT<TEndIndComposer>(use_tmsg)
  { }
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
template <>
class TDlgIndicationComposerT<TC_PAbort_Ind> : public TC_PAbort_Ind {
public:
  TDlgIndicationComposerT(const EncodedOID * use_ac, ROSComponentsList * use_comps)
    : TC_PAbort_Ind(use_ac)
  { }

  TC_PAbort_Ind & TInd(void) { return *this; }
};
//
class TPAbortIndComposer : public TDlgIndicationComposerT<TC_PAbort_Ind> {
public:
  TPAbortIndComposer(const EncodedOID * use_ac = NULL,
                    ROSComponentsList * use_comps = NULL)
    : TDlgIndicationComposerT<TC_PAbort_Ind>(use_ac, use_comps)
  { }
};
//
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
template <>
class TDlgIndicationComposerT<TC_UAbort_Ind> : public TC_UAbort_Ind {
public:
  TDlgIndicationComposerT(const EncodedOID * use_ac, ROSComponentsList * use_comps)
    : TC_UAbort_Ind(use_ac)
  { }

  TC_UAbort_Ind & TInd(void) { return *this; }
};
//
class TUAbortIndComposer : public TDlgIndicationComposerT<TC_UAbort_Ind> {
public:
  TUAbortIndComposer(const EncodedOID * use_ac = NULL,
                    ROSComponentsList * use_comps = NULL)
    : TDlgIndicationComposerT<TC_UAbort_Ind>(use_ac, use_comps)
  { }
};
//
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
template <>
class TDlgIndicationComposerT<TC_Notice_Ind> : public TC_Notice_Ind {
public:
  TDlgIndicationComposerT(const EncodedOID * use_ac, ROSComponentsList * use_comps)
    : TC_Notice_Ind(use_ac, use_comps)
  { }

  TC_Notice_Ind & TInd(void) { return *this; }
};
//
class TNoticeIndComposer : public TDlgIndicationComposerT<TC_Notice_Ind> {
public:
  TNoticeIndComposer(const EncodedOID * use_ac = NULL,
                    ROSComponentsList * use_comps = NULL)
    : TDlgIndicationComposerT<TC_Notice_Ind>(use_ac, use_comps)
  { }
};
//
class TNoticeIndDispatcher : public TDlgIndicationDispatcherT<TNoticeIndComposer> {
public:
  TNoticeIndDispatcher(TCAPMessage & use_tmsg)
    : TDlgIndicationDispatcherT<TNoticeIndComposer>(use_tmsg)
  { }
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

