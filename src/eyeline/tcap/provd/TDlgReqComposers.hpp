/* ************************************************************************** *
 * TCAP dialogue requests composers.
 * ************************************************************************** */
#ifndef __ELC_TCAP_REQUESTS_COMPOSERS_HPP
#ident "@(#)$Id$"
#define __ELC_TCAP_REQUESTS_COMPOSERS_HPP

#include "eyeline/tcap/TDialogueRequestPrimitives.hpp"
#include "eyeline/tcap/proto/TransactionId.hpp"
#include "eyeline/tcap/provd/TDlgPAbortReq.hpp"
#include "eyeline/tcap/provd/SUARequests.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

using eyeline::tcap::proto::TransactionId;

class TDlgRequestComposerAC {
protected:
  bool            _dlgResp; //request is a response to a T_Begin
  TransactionId   _trId;
  const EncodedOID * _acOId;

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

  TDlgRequestComposerAC(const EncodedOID * use_acid = NULL)
    : _acOId(use_acid)
  { }
  virtual ~TDlgRequestComposerAC()
  { }

  //NOTE: overwrites the OID inherited from T_REQ
  void setAppCtx(const EncodedOID & use_acid) { _acOId = &use_acid; }
  const EncodedOID * getAppCtx(void) const { return _acOId; }
  //
  void setTransactionId(const TransactionId & use_id) { _trId = use_id; }
  const TransactionId & getTransactionId(void) const { return _trId; }

  //Indicates that this request is a response to a T_Begin
  void setDialogueResponse(void) { _dlgResp = true; }
  bool isDialogueResponse(void) const { return _dlgResp; }

  // ----------------------------------------------
  // -- TDlgRequestComposerAC interface methods
  // ----------------------------------------------
  virtual SerializationResult_e 
    serialize2UDT(SUAUnitdataReq & use_udt, const SCCPAddress & src_addr,
                  const SCCPAddress & dst_addr) const = 0;
};

template <class T_DLG_REQ_Arg /* pubic: TDialogueRequestPrimitive */>
class TDlgRequestComposerT : public TDlgRequestComposerAC {
protected:
  const T_DLG_REQ_Arg & _tReq;

public:
  TDlgRequestComposerT(const T_DLG_REQ_Arg & use_req)
   : TDlgRequestComposerAC(use_req.getAppCtx()), _tReq(use_req)
  { }

  const T_DLG_REQ_Arg & TReq(void) const { return _tReq; }
};


// -----------------------------------------------
// -- specialization: TC_Begin_Req composer
// -----------------------------------------------
class TBeginReqComposer : public TDlgRequestComposerT<TC_Begin_Req> {
public:
  TBeginReqComposer(const TC_Begin_Req & use_req)
    : TDlgRequestComposerT<TC_Begin_Req>(use_req)
  { }

  // ----------------------------------------------
  // -- TDlgRequestComposerAC interface methods
  // ----------------------------------------------
  virtual SerializationResult_e
    serialize2UDT(SUAUnitdataReq & use_udt, const SCCPAddress & src_addr,
                                            const SCCPAddress & dst_addr) const;
};

// -----------------------------------------------
// -- specialization: TC_Cont_Req composer
// -----------------------------------------------
class TContReqComposer : public TDlgRequestComposerT<TC_Cont_Req> {
public:
  TContReqComposer(const TC_Cont_Req & use_req)
    : TDlgRequestComposerT<TC_Cont_Req>(use_req)
  { }

  // ----------------------------------------------
  // -- TDlgRequestComposerAC interface methods
  // ----------------------------------------------
  virtual SerializationResult_e
    serialize2UDT(SUAUnitdataReq & use_udt, const SCCPAddress & src_addr,
                                            const SCCPAddress & dst_addr) const;
};

// -----------------------------------------------
// -- specialization: TC_End_Req composer
// -----------------------------------------------
class TEndReqComposer : public TDlgRequestComposerT<TC_End_Req> {
public:
  TEndReqComposer(const TC_End_Req & use_req)
    : TDlgRequestComposerT<TC_End_Req>(use_req)
  { }

  // ----------------------------------------------
  // -- TDlgRequestComposerAC interface methods
  // ----------------------------------------------
  virtual SerializationResult_e
    serialize2UDT(SUAUnitdataReq & use_udt, const SCCPAddress & src_addr,
                                            const SCCPAddress & dst_addr) const;
};

// -----------------------------------------------
// -- specialization: TC_PAbort_Req composer
// -----------------------------------------------
// -> TCMessage::Abort { reason : p-abortCause }
class TPAbortReqComposer : public TDlgRequestComposerT<TC_PAbort_Req> {
public:
  TPAbortReqComposer(const TC_PAbort_Req & use_req)
    : TDlgRequestComposerT<TC_PAbort_Req>(use_req)
  { }

  // ----------------------------------------------
  // -- TDlgRequestComposerAC interface methods
  // ----------------------------------------------
  virtual SerializationResult_e
    serialize2UDT(SUAUnitdataReq & use_udt, const SCCPAddress & src_addr,
                                            const SCCPAddress & dst_addr) const;
};

// -----------------------------------------------
// -- specialization: TC_UAbort_Req composer
// -----------------------------------------------
// -> TCMessage::Abort { reason : u-abortCause : AS{_ac_tcap_strDialogue_as::AARE} }
class TUAbortReqComposer : public TDlgRequestComposerT<TC_UAbort_Req> {
public:
  TUAbortReqComposer(const TC_UAbort_Req & use_req)
    : TDlgRequestComposerT<TC_UAbort_Req>(use_req)
  { }

  // ----------------------------------------------
  // -- TDlgRequestComposerAC interface methods
  // ----------------------------------------------
  virtual SerializationResult_e
    serialize2UDT(SUAUnitdataReq & use_udt, const SCCPAddress & src_addr,
                                            const SCCPAddress & dst_addr) const;
};

} //provd
} //tcap
} //eyeline

#endif /* __ELC_TCAP_REQUESTS_COMPOSERS_HPP */

