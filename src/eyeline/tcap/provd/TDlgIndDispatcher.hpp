/* ************************************************************************** *
 * TCAP dialogue indications dispatcher.
 * ************************************************************************** */
#ifndef __ELC_TCAP_INDICATIONS_DISPATCHER_HPP
#ident "@(#)$Id$"
#define __ELC_TCAP_INDICATIONS_DISPATCHER_HPP

#include "eyeline/tcap/proto/TCMessage.hpp"
#include "eyeline/tcap/provd/SCSPIndications.hpp"
//#include "eyeline/tcap/provd/TDlgIndComposers.hpp"
#include "eyeline/tcap/provd/TDlgIndProcessor.hpp"

#include "eyeline/util/ChoiceOfT.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

//Basic abstract class for all TC indication dispatchers
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
  virtual bool dispatchTInd(TDlgIndProcessorIface * use_hdl, unsigned int scsp_link_num) = 0;
};


template <class T_IND_COMPOSER_Arg /* pubic: TDialogueIndicationComposerT<> */>
class TDlgIndicationDispatcherT : public TDlgIndicationDispatcherAC {
protected:
  T_IND_COMPOSER_Arg _indComposer;

public:
  TDlgIndicationDispatcherT() : TDlgIndicationDispatcherAC()
  { }
  virtual ~TDlgIndicationDispatcherT()
  { }

  // -----------------------------------------------
  // -- TDlgIndicationDispatcherAC interface methods
  // -----------------------------------------------
  virtual bool dispatchTInd(TDlgIndProcessorIface * use_hdl, unsigned int scsp_link_num)
  {
    _indComposer.setSCSPLink(scsp_link_num);
    use_hdl->updateDlgByIndication(_indComposer);
    return true;
  }
};

// -----------------------------------------------
// -- specialization: TR_Begin_Ind dispatcher
// -----------------------------------------------
class TBeginIndDispatcher : public TDlgIndicationDispatcherT<TBeginIndComposer> {
public:
  TBeginIndDispatcher() : TDlgIndicationDispatcherT<TBeginIndComposer>()
  { }
  ~TBeginIndDispatcher()
  { }
  //
  bool processSCSPInd(const SCSPUnitdataInd & scsp_ind, proto::TMsgBegin & use_tmsg)
    /*throw(std::exception)*/
  {
    _indComposer.init(use_tmsg); //throws
    if (_indComposer.setOrigAddress(scsp_ind.callingAddr(), scsp_ind.callingAddrLen()))
      return _indComposer.setDestAddress(scsp_ind.calledAddr(), scsp_ind.calledAddrLen());
    return false;
  }
};

// -----------------------------------------------
// -- specialization: TR_Cont_Ind dispatcher
// -----------------------------------------------
class TContIndDispatcher : public TDlgIndicationDispatcherT<TContIndComposer> {
public:
  TContIndDispatcher() : TDlgIndicationDispatcherT<TContIndComposer>()
  { }
  ~TContIndDispatcher()
  { }
  //
  bool processSCSPInd(const SCSPUnitdataInd & scsp_ind, proto::TMsgContinue & use_tmsg)
    /*throw(std::exception)*/
  {
    _indComposer.init(use_tmsg);
    return _indComposer.setOrigAddress(scsp_ind.callingAddr(), scsp_ind.callingAddrLen());
  }
};

// -----------------------------------------------
// -- specialization: TR_End_Ind dispatcher
// -----------------------------------------------
class TEndIndDispatcher : public TDlgIndicationDispatcherT<TEndIndComposer> {
public:
  TEndIndDispatcher() : TDlgIndicationDispatcherT<TEndIndComposer>()
  { }
  ~TEndIndDispatcher()
  { }
  //
  bool processSCSPInd(const SCSPUnitdataInd & scsp_ind, proto::TMsgEnd & use_tmsg)
    /*throw(std::exception)*/
  {
    _indComposer.init(use_tmsg);
    return _indComposer.setOrigAddress(scsp_ind.callingAddr(), scsp_ind.callingAddrLen());
  }
};

// -----------------------------------------------
// -- specialization: TR_PAbort_Ind dispatcher
// -----------------------------------------------
class TPAbortIndDispatcher : public TDlgIndicationDispatcherT<TPAbortIndComposer> {
public:
  TPAbortIndDispatcher() : TDlgIndicationDispatcherT<TPAbortIndComposer>()
  { }
  ~TPAbortIndDispatcher()
  { }
  //
  bool processSCSPInd(const SCSPUnitdataInd & scsp_ind, proto::TMsgAbort & use_tmsg)
    /*throw(std::exception)*/
  {
    _indComposer.init(use_tmsg);
    return _indComposer.setOrigAddress(scsp_ind.callingAddr(), scsp_ind.callingAddrLen());
  }
};

// -----------------------------------------------
// -- specialization: TR_UAbort_Ind dispatcher
// -----------------------------------------------
class TUAbortIndDispatcher : public TDlgIndicationDispatcherT<TUAbortIndComposer> {
public:
  TUAbortIndDispatcher() : TDlgIndicationDispatcherT<TUAbortIndComposer>()
  { }
  ~TUAbortIndDispatcher()
  { }
  //
  bool processSCSPInd(const SCSPUnitdataInd & scsp_ind, proto::TMsgAbort & use_tmsg)
    /*throw(std::exception)*/
  {
    _indComposer.init(use_tmsg);
    return _indComposer.setOrigAddress(scsp_ind.callingAddr(), scsp_ind.callingAddrLen());
  }
};

// -----------------------------------------------
// -- specialization: TR_Notice_Ind dispatcher
// -----------------------------------------------
class TNoticeIndDispatcher : public TDlgIndicationDispatcherT<TNoticeIndComposer> {
public:
  TNoticeIndDispatcher() : TDlgIndicationDispatcherT<TNoticeIndComposer>()
  { }
  ~TNoticeIndDispatcher()
  { }
  //
  bool processSCSPInd(const SCSPNoticeInd & scsp_ind) /*throw(std::exception)*/
  {
    _indComposer.setReturnCause(scsp_ind.getCauseValue());
    if (_indComposer.setOrigAddress(scsp_ind.callingAddr(), scsp_ind.callingAddrLen())
        && _indComposer.setDestAddress(scsp_ind.calledAddr(), scsp_ind.calledAddrLen())) {
      _indComposer.setUserData(scsp_ind.userData(), scsp_ind.userDataLen());
      return true;
    }
    return false;
  }
};


//TCAP dialogue indications dispatcher: implements functionality on creating
//  TCAP dialogue indications from received SCCP message and dispatching the
//  former to appropriate TCAP dialogue handler.
class TDlgIndicationDispatcher : SCSPIndHandlerIface {
protected:
  class AltDispatcher : public util::ChoiceOfBased6_T<
    TDlgIndicationDispatcherAC,
    TBeginIndDispatcher, TContIndDispatcher,
    TEndIndDispatcher, TUAbortIndDispatcher,
    TPAbortIndDispatcher, TNoticeIndDispatcher> {
  public:
    Alternative_T<TBeginIndDispatcher>  tBegin()  { return alternative0(); }
    Alternative_T<TContIndDispatcher>   tCont()   { return alternative1(); }
    Alternative_T<TEndIndDispatcher>    tEnd()    { return alternative2(); }
    Alternative_T<TUAbortIndDispatcher> tUAbrt()  { return alternative3(); }
    Alternative_T<TPAbortIndDispatcher> tPAbrt()  { return alternative4(); }
    Alternative_T<TNoticeIndDispatcher> tNotice() { return alternative5(); }
  };

  AltDispatcher     _dsp;
  SCSPMessageInfo   _msgSCSP;
  proto::TCMessage  _msgTC; //has references to _msgSCSP.msgData

  // --------------------------------------------------------
  // -- SCSPIndHandlerIface interface methods implementation
  // --------------------------------------------------------
  virtual bool processSCSPInd(const SCSPUnitdataInd & scsp_ind) /*throw(std::exception)*/;
  virtual bool processSCSPInd(const SCSPNoticeInd & scsp_ind) /*throw(std::exception)*/;

public:
  TDlgIndicationDispatcher()
  { }
  //
  ~TDlgIndicationDispatcher()
  {
    clear();
  }

  void clear(void)
  {
    _dsp.clear();
    _msgTC.clear();
    _msgSCSP.clear();
  }

  TDlgIndicationPrimitive::IKind_e indKind(void) const
  {
    return static_cast<TDlgIndicationPrimitive::IKind_e>(_dsp.getChoiceIdx());
  }

  SCSPMessageInfo * getSCSPMessage(void) { return &_msgSCSP; }

  //Parses SCCP mesage buffer, decodes TCAP message, composes appropriate TCAP
  //Indication prinitive and creates its dispatcher (by calling processSuaInd())
  bool processSCSPMessage(void) /*throw(std::exception)*/;

  // ------------------------------------------------------------
  // NOTE: following methods should be called ONLY after one of
  // process*() methods was executed !
  // ------------------------------------------------------------
  bool dispatchTInd(TDlgIndProcessorIface * use_hdl) /*throw(std::exception)*/
  {
    return _dsp.get() ? _dsp.get()->dispatchTInd(use_hdl, _msgSCSP.connectNum) : false;
  }
};

} //provd
} //tcap
} //eyeline

#endif /* __ELC_TCAP_INDICATIONS_DISPATCHER_HPP */

