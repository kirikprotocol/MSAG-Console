#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/sccp/SCCPDefs.hpp"
#include "eyeline/tcap/provd/TDlgReqComposers.hpp"
#include "eyeline/tcap/provd/TDlgFSMRegistry.hpp"

#include "eyeline/tcap/proto/enc/TEMsgTBegin.hpp"
#include "eyeline/tcap/proto/enc/TEMsgTContinue.hpp"
#include "eyeline/tcap/proto/enc/TEMsgTEnd.hpp"
#include "eyeline/tcap/proto/enc/TEMsgTAbort.hpp"

#include "eyeline/utilx/Exception.hpp"

//NOTE.1: Providing in-sequence delivery functionality for locally initiated
//        TCAP dialogue.
//  TransactionId.localId is used as 'sequence control number' in order to
//  provide in-sequence delivery by SCCP layer. This forces the selection
//  of the same signalling route for all dialogue primitives of transaction,
//  because of SCCP selects signalling route basing on pair 
//  { sequence_control_number, destination_address }

namespace eyeline {
namespace tcap {
namespace provd {

using eyeline::sccp::UDTParms;

TDlgRequestComposerAC::UDTStatus
  TDlgRequestComposerAC::initUDT(SCSPUnitdataReq & use_udt,
                                  const TDlgRequestPrimitive & use_req,
                                  const SCCPAddress & src_addr,
                                  const SCCPAddress & dst_addr) const
{
  UDTStatus rval;
  if (!use_udt.setCallingAddr(src_addr)) {
    rval._status = TCError::errSrcAddress;
    return rval;
  }

  if (!use_udt.setCalledAddr(dst_addr)) {
    rval._status = TCError::errDstAddress;
    return rval;
  }

  //TODO: implement some logic to determine required values of
  //  'importance' and/or 'hopCount' field(s) if necessary.
  //use_udt.setImportance(/*TODO:*/);
  //use_udt.setHopCount(/*TODO:*/); 

  use_udt.setReturnOnError(use_req.getReturnOnError());
  if (use_req.getInSequenceDelivery())
    use_udt.setSequenceControl(getTransactionId().getIdLocal()); //read NOTE.1

  //check for overall data length
  uint8_t optionalsMask = (use_udt.getHopCount() ? UDTParms::has_HOPCOUNT : 0)
                          | (use_udt.getImportance() ? UDTParms::has_IMPORTANCE : 0);

  rval._maxDataSz = UDTParms::calculateMaxDataSz(
                          use_udt.getSCCPStandard(), optionalsMask,
                          use_udt.calledAddrLen(),  use_udt.callingAddrLen());
  if (rval._maxDataSz) {
    use_udt.dataBuf().resize(rval._maxDataSz);
  } else
    rval._status = TCError::errDialoguePortion;
  return rval;
}

/* ************************************************************************* *
 * class TBeginReqComposer implementation
 * ************************************************************************* */
TCRCode_e
  TBeginReqComposer::serialize2UDT(SCSPUnitdataReq & use_udt,
                                   const SCCPAddress& src_addr,
                                   const SCCPAddress& dst_addr) const
{
  UDTStatus rval = initUDT(use_udt, _tReq, src_addr, dst_addr);
  if (rval._status != TCError::dlgOk)
    return rval._status;

  proto::enc::TETBegin encTMsg(_trId.getIdLocal());
  //Initialize DialoguePortion if required
  if (getAppCtx() && (*getAppCtx() != _ac_contextless_ops)) {
    //AARQ_Apdu must be present if TCUser suggests appCtx
    proto::enc::TEAPduAARQ * pdu = encTMsg.initDlgRequest(*getAppCtx());
    pdu->addUIList(_tReq.getUserInfo());
  }

  //Initialize ComponentPortion (verified by DialogueFSM) if required
  encTMsg.addComponents(_tReq.getCompList());

  //Encode composed message
  asn1::ENCResult eRes = encTMsg.encode(use_udt.dataBuf().get(), rval._maxDataSz);
  if (!eRes.isOk()) {
    if (eRes.status == asn1::ENCResult::encMoreMem) {
      smsc_log_error(_logger, "TDlg[%s]: %s::serialize2UDT(): message length limit(%u) is exceeded",
                    _trId.toString().c_str(), _tReq.getIdent(), rval._maxDataSz);
      
      if (_tReq.getCompList().empty())
        return TCError::errComponentPortion; //UsrInfo is too long
      //check for srlzTooMuchComponents
      encTMsg.clearCompPortion();
      asn1::ENCResult nRes = encTMsg.calculate();
      if (nRes.isOk())
        return TCError::errTooMuchComponents;
    }
    return TCError::errTCAPUnknown;
  }
  use_udt.dataBuf().setDataSize(eRes.nbytes);
  use_udt.dataBuf().setPos(0);
  return TCError::dlgOk;
}

/* ************************************************************************* *
 * class TContReqComposer implementation
 * ************************************************************************* */
TCRCode_e
  TContReqComposer::serialize2UDT(SCSPUnitdataReq & use_udt,
                                  const SCCPAddress& src_addr,
                                  const SCCPAddress& dst_addr) const
{
  UDTStatus rval = initUDT(use_udt, _tReq, src_addr, dst_addr);
  if (rval._status != TCError::dlgOk)
    return rval._status;

  proto::enc::TETContinue encTMsg(_trId.getIdLocal(), _trId.getIdRemote());

  //Initialize DialoguePortion if required
  if (isDialogueResponse()) { //AARE_Apdu must be present if TCont is response to TBegin 
                              //and either appCtx was used while dialog establishment
                              //or TCUser suggests new appCtx
    if (getAppCtx() && (*getAppCtx() != _ac_contextless_ops)) {
      proto::enc::TEAPduAARE * pdu = encTMsg.initDlgResponse(*getAppCtx());
      pdu->acceptByUser();
      pdu->addUIList(_tReq.getUserInfo());
    }
  }
  //Initialize ComponentPortion (verified by DialogueFSM) if required
  encTMsg.addComponents(_tReq.getCompList());

  //Encode composed message
  asn1::ENCResult eRes = encTMsg.encode(use_udt.dataBuf().get(), rval._maxDataSz);
  if (!eRes.isOk()) {
    if (eRes.status == asn1::ENCResult::encMoreMem) {
      //TODO: If serialized data is too long perform TCAP segmenting: split the
      //UserInfo and/or ComponentsList into several TContRequests preserving the
      //order of components and forcedly setting the in-sequence-delivery
      //transferring by calling 
      //  use_udt.setSequenceControl(getTransactionId().localId()); //read NOTE.1
      //
      smsc_log_error(_logger, "TDlg[%s]: %s::serialize2UDT(): message length limit(%u) is exceeded",
                    _trId.toString().c_str(), _tReq.getIdent(), rval._maxDataSz);

      if (_tReq.getCompList().empty())
        return TCError::errDialoguePortion; //UsrInfo is too long
      //check for srlzTooMuchComponents
      encTMsg.clearCompPortion();
      asn1::ENCResult nRes = encTMsg.calculate();
      if (nRes.isOk())
        return TCError::errTooMuchComponents;
    }
    return TCError::errTCAPUnknown;
  }
  use_udt.dataBuf().setDataSize(eRes.nbytes);
  use_udt.dataBuf().setPos(0);
  return TCError::dlgOk;
}

/* ************************************************************************* *
 * class TEndReqComposer implementation
 * ************************************************************************* */
TCRCode_e
  TEndReqComposer::serialize2UDT(SCSPUnitdataReq & use_udt,
                                 const SCCPAddress& src_addr,
                                 const SCCPAddress& dst_addr) const
{
  UDTStatus rval = initUDT(use_udt, _tReq, src_addr, dst_addr);
  if (rval._status != TCError::dlgOk)
    return rval._status;

  proto::enc::TETEnd encTMsg(_trId.getIdRemote());
  //Initialize DialoguePortion if required
  if (isDialogueResponse()) { //AARE_Apdu may be present if TEnd is response to TBegin
                              //and appCtx was used while dialog establishment
    if (getAppCtx() && (*getAppCtx() != _ac_contextless_ops)) {
      proto::enc::TEAPduAARE * pdu = encTMsg.initDlgResponse(*getAppCtx());
      pdu->acceptByUser();
      pdu->addUIList(_tReq.getUserInfo());
    }
  }
  //Initialize ComponentPortion (verified by DialogueFSM) if required
  encTMsg.addComponents(_tReq.getCompList());

  //Encode composed message
  asn1::ENCResult eRes = encTMsg.encode(use_udt.dataBuf().get(), rval._maxDataSz);
  if (!eRes.isOk()) {
    if (eRes.status == asn1::ENCResult::encMoreMem) {
      smsc_log_error(_logger, "TDlg[%s]: %s::serialize2UDT(): message length limit(%u) is exceeded",
                    _trId.toString().c_str(), _tReq.getIdent(), rval._maxDataSz);

      if (_tReq.getCompList().empty())
        return TCError::errDialoguePortion; //UsrInfo is too long
      //check for srlzTooMuchComponents
      encTMsg.clearCompPortion();
      asn1::ENCResult nRes = encTMsg.calculate();
      if (nRes.isOk())
        return TCError::errTooMuchComponents;
    }
    return TCError::errTCAPUnknown;
  }
  use_udt.dataBuf().setDataSize(eRes.nbytes);
  use_udt.dataBuf().setPos(0);
  return TCError::dlgOk;
}

/* ************************************************************************* *
 * class TUAbortReqComposer implementation
 * ************************************************************************* */
TCRCode_e
  TUAbortReqComposer::serialize2UDT(SCSPUnitdataReq & use_udt,
                                  const SCCPAddress& src_addr,
                                  const SCCPAddress& dst_addr) const
    /*throw(std::exception)*/
{
  UDTStatus rval = initUDT(use_udt, _tReq, src_addr, dst_addr);
  if (rval._status != TCError::dlgOk)
    return rval._status;

  proto::enc::TETAbort encTMsg(_trId.getIdRemote());

  switch(_tReq.getAbortKind()) {
  case TR_UAbort_Req::uabrtAssociation: {
    if (!isDialogueResponse()) //assertion!!!
      throw smsc::util::Exception("uabrtAssociation && !isDialogueResponse");
    //AARE_Apdu may be present if TAbort is response to TBegin
    //and appCtx was used while dialog establishment
    if (getAppCtx() && (*getAppCtx() != _ac_contextless_ops)) {

      proto::enc::TEAPduAARE * pdu = encTMsg.setUserReject(*getAppCtx(), _tReq.getAssociateDiagnostic());
      pdu->addUIList(_tReq.getUserInfo());
    }
  } break;

  case TR_UAbort_Req::uabrtDialogueUI: {
    if (isDialogueResponse()) //assertion!!!
      throw smsc::util::Exception("uabrtDialogueUI && isDialogueResponse");

    proto::enc::TEAPduABRT * pdu = encTMsg.setUserAbort();
    pdu->addUIList(_tReq.getUserInfo());
  } break;

  //case TR_UAbort_Req::uabrtDialogueEXT:
  default:
    if (isDialogueResponse()) //assertion!!!
      throw smsc::util::Exception("uabrtDialogueEXT && isDialogueResponse");

    if (_tReq.getUserInfo().empty())
      return TCError::errDialoguePortion;
    encTMsg.setUserAbort(*_tReq.getUserInfo().front());
  } /*eosw*/

  //Encode composed message
  asn1::ENCResult eRes = encTMsg.encode(use_udt.dataBuf().get(), rval._maxDataSz);
  if (!eRes.isOk()) {
    if (eRes.status == asn1::ENCResult::encMoreMem) {
      smsc_log_error(_logger, "TDlg[%s]: %s::serialize2UDT(): message length limit(%u) is exceeded",
                    _trId.toString().c_str(), _tReq.getIdent(), rval._maxDataSz);
    }
    return TCError::errTCAPUnknown;
  }
  use_udt.dataBuf().setDataSize(eRes.nbytes);
  use_udt.dataBuf().setPos(0);
  return TCError::dlgOk;
}
   
/* ************************************************************************* *
 * class TPAbortReqComposer implementation
 * ************************************************************************* */
TCRCode_e
  TPAbortReqComposer::serialize2UDT(SCSPUnitdataReq & use_udt,
                                  const SCCPAddress& src_addr,
                                  const SCCPAddress& dst_addr) const
{
  UDTStatus rval = initUDT(use_udt, _tReq, src_addr, dst_addr);
  if (rval._status != TCError::dlgOk)
    return rval._status;

  proto::enc::TETAbort encTMsg(_trId.getIdRemote());
  encTMsg.setPrvdAbort(_tReq.getAbortCause());

  //Encode composed message
  asn1::ENCResult eRes = encTMsg.encode(use_udt.dataBuf().get(), rval._maxDataSz);
  if (!eRes.isOk()) {
    if (eRes.status == asn1::ENCResult::encMoreMem) {
      smsc_log_error(_logger, "TDlg[%s]: %s::serialize2UDT(): message length limit(%u) is exceeded",
                    _trId.toString().c_str(), _tReq.getIdent(), rval._maxDataSz);
    }
    return TCError::errTCAPUnknown;
  }
  use_udt.dataBuf().setDataSize(eRes.nbytes);
  use_udt.dataBuf().setPos(0);
  return TCError::dlgOk;
}
  
   
} //provd
} //tcap
} //eyeline

