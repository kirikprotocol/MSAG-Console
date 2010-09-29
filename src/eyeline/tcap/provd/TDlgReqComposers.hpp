/* ************************************************************************** *
 * TCAP dialogue requests composers.
 * ************************************************************************** */
#ifndef __ELC_TCAP_REQUESTS_COMPOSERS_HPP
#ident "@(#)$Id$"
#define __ELC_TCAP_REQUESTS_COMPOSERS_HPP

#include "logger/Logger.h"

#include "eyeline/tcap/TcapErrors.hpp"
#include "eyeline/tcap/TDlgRequestPrimitives.hpp"
#include "eyeline/tcap/provd/TDlgPAbortReq.hpp"
#include "eyeline/tcap/provd/TransactionId.hpp"
#include "eyeline/tcap/provd/SCSPRequests.hpp"


namespace eyeline {
namespace tcap {
namespace provd {

using smsc::logger::Logger;

class TDlgRequestComposerAC {
protected:
  struct UDTStatus {
    TCRCode_e  _status;
    uint16_t   _maxDataSz;

    UDTStatus() : _status(TCError::dlgOk), _maxDataSz(0)
    { }
  };
  /* - */
  bool                _dlgResp; //request is a response to a T_Begin
  const EncodedOID *  _acOId; //inherits or overwrites the OID from T_REQ
  TransactionId       _trId;
  Logger *            _logger;

  /* - */
  UDTStatus
    initUDT(SCSPUnitdataReq & use_udt, const TDlgRequestPrimitive & use_req,
            const SCCPAddress & src_addr, const SCCPAddress & dst_addr) const;

public:
  TDlgRequestComposerAC(const EncodedOID * use_acid = NULL)
    : _dlgResp(false), _acOId(use_acid)
    , _logger(Logger::getInstance("tcap.provd"))
  { }
  virtual ~TDlgRequestComposerAC()
  { }

  //
  void setTransactionId(const TransactionId & use_id) { _trId = use_id; }
  const TransactionId & getTransactionId(void) const { return _trId; }

  //Indicates that this request is a response to a T_Begin
  void setDialogueResponse(void) { _dlgResp = true; }
  bool isDialogueResponse(void) const { return _dlgResp; }

  //NOTE: overwrites the OID inherited from T_REQ
  void setAppCtx(const EncodedOID & use_acid) { _acOId = &use_acid; }
  const EncodedOID * getAppCtx(void) const { return _acOId; }

  // ----------------------------------------------
  // -- TDlgRequestComposerAC interface methods
  // ----------------------------------------------
  virtual TCRCode_e
    serialize2UDT(SCSPUnitdataReq & use_udt, const SCCPAddress & src_addr,
                                            const SCCPAddress & dst_addr) const
      /*throw(std::exception)*/ = 0;
};

template <class T_DLG_REQ_Arg /* pubic: TDialogueRequestPrimitive */>
class TDlgRequestComposerT : public TDlgRequestComposerAC {
protected:
  const T_DLG_REQ_Arg & _tReq;

public:
  TDlgRequestComposerT(const T_DLG_REQ_Arg & use_req)
    : TDlgRequestComposerAC(use_req.getAppCtx()), _tReq(use_req)
  { }
  ~TDlgRequestComposerT()
  { }

  const T_DLG_REQ_Arg & TReq(void) const { return _tReq; }
};


// -----------------------------------------------
// -- specialization: TR_Begin_Req composer
// -----------------------------------------------
class TBeginReqComposer : public TDlgRequestComposerT<TR_Begin_Req> {
public:
  TBeginReqComposer(const TR_Begin_Req & use_req)
    : TDlgRequestComposerT<TR_Begin_Req>(use_req)
  { }
  ~TBeginReqComposer()
  { }

  // ----------------------------------------------
  // -- TDlgRequestComposerAC interface methods
  // ----------------------------------------------
  virtual TCRCode_e
    serialize2UDT(SCSPUnitdataReq & use_udt, const SCCPAddress & src_addr,
                                            const SCCPAddress & dst_addr) const
    /*throw(std::exception)*/;
};

// -----------------------------------------------
// -- specialization: TR_Cont_Req composer
// -----------------------------------------------
class TContReqComposer : public TDlgRequestComposerT<TR_Cont_Req> {
public:
  TContReqComposer(const TR_Cont_Req & use_req)
    : TDlgRequestComposerT<TR_Cont_Req>(use_req)
  { }
  ~TContReqComposer()
  { }

  // ----------------------------------------------
  // -- TDlgRequestComposerAC interface methods
  // ----------------------------------------------
  virtual TCRCode_e
    serialize2UDT(SCSPUnitdataReq & use_udt, const SCCPAddress & src_addr,
                                            const SCCPAddress & dst_addr) const
    /*throw(std::exception)*/;
};

// -----------------------------------------------
// -- specialization: TR_End_Req composer
// -----------------------------------------------
class TEndReqComposer : public TDlgRequestComposerT<TR_End_Req> {
public:
  TEndReqComposer(const TR_End_Req & use_req)
    : TDlgRequestComposerT<TR_End_Req>(use_req)
  { }
  ~TEndReqComposer()
  { }

  // ----------------------------------------------
  // -- TDlgRequestComposerAC interface methods
  // ----------------------------------------------
  virtual TCRCode_e
    serialize2UDT(SCSPUnitdataReq & use_udt, const SCCPAddress & src_addr,
                                            const SCCPAddress & dst_addr) const
    /*throw(std::exception)*/;
};

// -----------------------------------------------
// -- specialization: TR_UAbort_Req composer
// -----------------------------------------------
// -> TCMessage::Abort { reason : u-abortCause : AS{_ac_tcap_strDialogue_as::AARE} }
class TUAbortReqComposer : public TDlgRequestComposerT<TR_UAbort_Req> {
public:
  TUAbortReqComposer(const TR_UAbort_Req & use_req)
    : TDlgRequestComposerT<TR_UAbort_Req>(use_req)
  { }
  ~TUAbortReqComposer()
  { }

  // ----------------------------------------------
  // -- TDlgRequestComposerAC interface methods
  // ----------------------------------------------
  virtual TCRCode_e
    serialize2UDT(SCSPUnitdataReq & use_udt, const SCCPAddress & src_addr,
                                            const SCCPAddress & dst_addr) const
    /*throw(std::exception)*/;
};


// -----------------------------------------------
// -- specialization: TR_PAbort_Req composer
// -----------------------------------------------
// -> TCMessage::Abort { reason : p-abortCause }
class TPAbortReqComposer : public TDlgRequestComposerT<TR_PAbort_Req> {
public:
  TPAbortReqComposer(const TR_PAbort_Req & use_req)
    : TDlgRequestComposerT<TR_PAbort_Req>(use_req)
  { }
  ~TPAbortReqComposer()
  { }

  // ----------------------------------------------
  // -- TDlgRequestComposerAC interface methods
  // ----------------------------------------------
  virtual TCRCode_e
    serialize2UDT(SCSPUnitdataReq & use_udt, const SCCPAddress & src_addr,
                                            const SCCPAddress & dst_addr) const
    /*throw(std::exception)*/;
};

} //provd
} //tcap
} //eyeline

#endif /* __ELC_TCAP_REQUESTS_COMPOSERS_HPP */

