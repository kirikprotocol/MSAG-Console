/* ************************************************************************** *
 * TCAP dialogue requests senders (serialization and sending).
 * ************************************************************************** */
#ifndef __ELC_TCAP_REQUESTS_SENDER_HPP
#ident "@(#)$Id$"
#define __ELC_TCAP_REQUESTS_SENDER_HPP

#include "eyeline/tcap/TDialogueRequestPrimitives.hpp"
#include "eyeline/tcap/provd/SUARequests.hpp"
#include "eyeline/tcap/provd/TDlgPAbortReq.hpp"
#include "eyeline/sua/libsua/SuaApi.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

using eyeline::sua::libsua::SuaApi;

class TDlgRequestSenderAC {
public:
  //NOTE: srlzBadComponentPortion error enforces TC_L_Reject_Ind,
  //      all other errors enforce TC_Notice_Ind
  enum SerializationResult_e {
      srlzOk = 0
    , srlzError                   //internal serializer error
    , srlzBadSrcAddr              //
    , srlzBadDstAddr              //
    , srlzBadTransactionPortion   //
    , srlzBadDialoguePortion      //
    , srlzBadComponentPortion     //enforces TC_L_Reject_Ind
  };

protected:
  SUAUnitdataReq  _udt;

public:
  TDlgRequestSenderAC()
  { }
  virtual ~TDlgRequestSenderAC()
  { }

  // ----------------------------------------------
  // -- TDlgRequestSenderAC interface methods
  // ----------------------------------------------
  virtual SerializationResult_e serialize2UDT(const SCCPAddress & src_addr,
                                              const SCCPAddress & dst_addr) = 0;

  // ----------------------------------------------
  SuaApi::CallResult sendMessage(SuaApi * sua_iface) const;

  SuaApi::CallResult sendMessage(SuaApi * sua_iface, unsigned int link_num) const;

  //These methods only for 1st response to T_Begin_Ind
  void rejectDlgByProvider(TDialogueAssociate::DiagnosticProvider_e use_cause =
                           TDialogueAssociate::dsp_null)
  { /*TODO: */  }

};


template <class _TArg /* pubic: TDialogueRequestPrimitive */>
class TDlgRequestSenderT : public TDlgRequestSenderAC {
protected:
  const _TArg & _tReq;

public:
  TDlgRequestSenderT(const _TArg & use_treq)
    : _tReq(use_treq)
  { }
  ~TDlgRequestSenderT()
  { }
};

/* ************************************************************************* *
 * TDlgRequestSenderT<> specializations:
 * ************************************************************************* */
//
class TBeginReqSender : public TDlgRequestSenderT<TC_Begin_Req> {
public:
  TBeginReqSender(const TC_Begin_Req & use_treq)
    : TDlgRequestSenderT<TC_Begin_Req>(use_treq)
  { }

  // ----------------------------------------------
  // -- TDlgRequestSenderAC interface methods
  // ----------------------------------------------
  virtual SerializationResult_e serialize2UDT(const SCCPAddress & src_addr,
                                              const SCCPAddress & dst_addr);
};
//
class TContReqSender : public TDlgRequestSenderT<TC_Cont_Req> {
public:
  TContReqSender(const TC_Cont_Req & use_treq)
    : TDlgRequestSenderT<TC_Cont_Req>(use_treq)
  { }
  // ----------------------------------------------
  // -- TDlgRequestSenderAC interface methods
  // ----------------------------------------------
  virtual SerializationResult_e serialize2UDT(const SCCPAddress & src_addr,
                                              const SCCPAddress & dst_addr);
};
//
class TEndReqSender : public TDlgRequestSenderT<TC_End_Req> {
public:
  TEndReqSender(const TC_End_Req & use_treq)
    : TDlgRequestSenderT<TC_End_Req>(use_treq)
  { }
  // ----------------------------------------------
  // -- TDlgRequestSenderAC interface methods
  // ----------------------------------------------
  virtual SerializationResult_e serialize2UDT(const SCCPAddress & src_addr,
                                              const SCCPAddress & dst_addr);
};
//
class TPAbortReqSender : public TDlgRequestSenderT<TC_PAbort_Req> {
public:
  TPAbortReqSender(const TC_PAbort_Req & use_treq)
    : TDlgRequestSenderT<TC_PAbort_Req>(use_treq)
  { }
  // ----------------------------------------------
  // -- TDlgRequestSenderAC interface methods
  // ----------------------------------------------
  virtual SerializationResult_e serialize2UDT(const SCCPAddress & src_addr,
                                              const SCCPAddress & dst_addr);
};
//
class TUAbortReqSender : public TDlgRequestSenderT<TC_UAbort_Req> {
public:
  TUAbortReqSender(const TC_UAbort_Req & use_treq)
    : TDlgRequestSenderT<TC_UAbort_Req>(use_treq)
  { }
  // ----------------------------------------------
  // -- TDlgRequestSenderAC interface methods
  // ----------------------------------------------
  virtual SerializationResult_e serialize2UDT(const SCCPAddress & src_addr,
                                              const SCCPAddress & dst_addr);
};

} //provd
} //tcap
} //eyeline

#endif /* __ELC_TCAP_REQUESTS_SENDER_HPP */

