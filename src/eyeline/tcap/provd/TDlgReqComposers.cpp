#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/sccp/SCCPDefs.hpp"
#include "TDlgReqComposers.hpp"
#include "TDialogueServiceDataRegistry.hpp"
#include "eyeline/tcap/proto/TCAPMessage.hpp"
#include "eyeline/tcap/proto/enc/AbortMessage.hpp"
#include "eyeline/tcap/proto/enc/AARE_APdu.hpp"
#include "eyeline/tcap/proto/enc/ABRT_APdu.hpp"
//#include "eyeline/tcap/proto/enc/EXTERNAL_APdu.hpp"
#include "eyeline/tcap/proto/enc/PAbortCause.hpp"
#include "eyeline/utilx/Exception.hpp"
#include "SUARequests.hpp"

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

TDlgRequestComposerAC::SerializationResult_e
  TDlgRequestComposerAC::initUDT(SUAUnitdataReq & use_udt,
                                 const TDialogueRequestPrimitive & use_req,
                                 const SCCPAddress& src_addr,
                                 const SCCPAddress& dst_addr) const
{
  if (!use_udt.setCallingAddr(src_addr))
    return TDlgRequestComposerAC::srlzBadSrcAddr;
  if (!use_udt.setCalledAddr(dst_addr))
    return TDlgRequestComposerAC::srlzBadDstAddr;

  //TODO: implement some logic to determine required values of
  //  'importance' and/or 'hopCount' field(s) if necessary.
  //use_udt.setImportance(/*TODO:*/);
  //use_udt.setHopCount(/*TODO:*/); 

  use_udt.setReturnOnError(use_req.getReturnOnError());
  if (use_req.getInSequenceDelivery())
    use_udt.setSequenceControl(getTransactionId().localId()); //read NOTE.1

  //check for overall data length
  uint8_t optionalsMask = (use_udt.getHopCount() ? UDTParms::has_HOPCOUNT : 0)
                          | (use_udt.getImportance() ? UDTParms::has_IMPORTANCE : 0);

  uint16_t maxDataSz = UDTParms::calculateMaxDataSz(
                          use_udt.getSCCPStandard(), optionalsMask,
                          use_udt.calledAddrLen(),  use_udt.callingAddrLen());
  if (!maxDataSz)
    return TDlgRequestComposerAC::srlzBadDialoguePortion;

  use_udt.dataBuf().resize(maxDataSz);

  return TDlgRequestComposerAC::srlzOk;
}

/* ************************************************************************* *
 * class TBeginReqComposer implementation
 * ************************************************************************* */
TDlgRequestComposerAC::SerializationResult_e
  TBeginReqComposer::serialize2UDT(SUAUnitdataReq & use_udt,
                                   const SCCPAddress& src_addr,
                                   const SCCPAddress& dst_addr) const
{
  TDlgRequestComposerAC::SerializationResult_e
    rval = initUDT(use_udt, _tReq, src_addr, dst_addr);

  if (rval != TDlgRequestComposerAC::srlzOk)
    return rval;

  uint16_t maxDataSz = use_udt.dataBuf().getMaxSize();

  //TODO: serialize TBeginReq to use_udt.dataBuf() taking in account 'maxDataSz'
  //If serialized data is too long determine the portion of TCAPMessage that is
  //too large (either UserInfo or ComponentsList) and return associated value
  //of SerializationResult_e
  return TDlgRequestComposerAC::srlzBadTransactionPortion;
}

/* ************************************************************************* *
 * class TContReqComposer implementation
 * ************************************************************************* */
TDlgRequestComposerAC::SerializationResult_e
  TContReqComposer::serialize2UDT(SUAUnitdataReq & use_udt,
                                  const SCCPAddress& src_addr,
                                  const SCCPAddress& dst_addr) const
{
  TDlgRequestComposerAC::SerializationResult_e
    rval = initUDT(use_udt, _tReq, src_addr, dst_addr);

  if (rval != TDlgRequestComposerAC::srlzOk)
    return rval;

  uint16_t maxDataSz = use_udt.dataBuf().getMaxSize();

  //TODO: serialize TContReq to use_udt.dataBuf() taking in account 'maxDataSz'
  //If serialized data is too long perform TCAP segmenting: split the UserInfo
  //and/or ComponentsList into several TContRequests preserving the order of
  //components and forcedly setting the in-sequence-delivery transferring by
  //calling 
  //  use_udt.setSequenceControl(getTransactionId().localId()); //read NOTE.1
  // 
  return TDlgRequestComposerAC::srlzBadTransactionPortion;
}

/* ************************************************************************* *
 * class TEndReqComposer implementation
 * ************************************************************************* */
TDlgRequestComposerAC::SerializationResult_e
  TEndReqComposer::serialize2UDT(SUAUnitdataReq & use_udt,
                                 const SCCPAddress& src_addr,
                                 const SCCPAddress& dst_addr) const
{
  TDlgRequestComposerAC::SerializationResult_e
    rval = initUDT(use_udt, _tReq, src_addr, dst_addr);

  if (rval != TDlgRequestComposerAC::srlzOk)
    return rval;

  uint16_t maxDataSz = use_udt.dataBuf().getMaxSize();
#if 0
  proto::AbortMessage tcapMsg(_tReq.getAbortCause());
  //tcapMsg.set();
  tcapMsg.set(tDlgSvcData->getTransactionId());
  const TDlgUserInfo* uInfo = _tReq.getUserInfo();
  for(TDlgUserInfo::iterator iter = uInfo->begin(), end_iter = uInfo->end();
      iter != end_iter; ++iter) {
    tcapMsg.addExternalValue(*iter);
  }
  tcapMsg.encode(use_udt.dataBuf());
  return TDlgRequestComposerAC::srlzOk;
#else
  //TODO: serialize TEndReq to use_udt.dataBuf() taking in account 'maxDataSz'
  //If serialized data is too long determine the portion of TCAPMessage that is
  //too large (either UserInfo or ComponentsList) and return associated value
  //of SerializationResult_e
  return TDlgRequestComposerAC::srlzBadTransactionPortion;
#endif
}

/* ************************************************************************* *
 * class TPAbortReqComposer implementation
 * ************************************************************************* */
TDlgRequestComposerAC::SerializationResult_e
TPAbortReqComposer::serialize2UDT(SUAUnitdataReq & use_udt,
                                  const SCCPAddress& src_addr,
                                  const SCCPAddress& dst_addr) const
{
  TDlgRequestComposerAC::SerializationResult_e
    rval = initUDT(use_udt, _tReq, src_addr, dst_addr);

  if (rval != TDlgRequestComposerAC::srlzOk)
    return rval;

  TDialogueServiceDataRegistry::registry_element_ref_t tDlgSvcData =
    TDialogueServiceDataRegistry::getInstance().getTDialogueServiceData(_tReq.getDialogueId());

  proto::enc::PAbortCause pAbortCause(_tReq.getAbortCause());
  proto::enc::AbortMessage tcapMsg(tDlgSvcData->getTransactionId(), &pAbortCause);

  return encodeMessage(tcapMsg, use_udt, "TPAbortReqComposer::serialize2UDT");
}

/* ************************************************************************* *
 * class TUAbortReqComposer implementation
 * ************************************************************************* */
TDlgRequestComposerAC::SerializationResult_e
TUAbortReqComposer::serialize2UDT(SUAUnitdataReq & use_udt,
                                  const SCCPAddress& src_addr,
                                  const SCCPAddress& dst_addr) const
{
  TDlgRequestComposerAC::SerializationResult_e
    rval = initUDT(use_udt, _tReq, src_addr, dst_addr);

  if (rval != TDlgRequestComposerAC::srlzOk)
    return rval;

  TDialogueServiceDataRegistry::registry_element_ref_t tDlgSvcData =
      TDialogueServiceDataRegistry::getInstance().getTDialogueServiceData(_tReq.getDialogueId());

  switch(_tReq.getAbortKind()) {
  case TC_UAbort_Req::uabrtAssociation:
    if ( !isDialogueResponse() )
      throw utilx::SerializationException("TUAbortReqComposer::serialize2UDT::: isDialogueResponse() is not true");

    return formAARE_APdu(_tReq, tDlgSvcData->getTransactionId(), use_udt);
  case TC_UAbort_Req::uabrtDialogueUI:
    if ( isDialogueResponse() )
      throw utilx::SerializationException("TUAbortReqComposer::serialize2UDT::: invalid abortKind=uabrtDialogueUI in response to dialogue initiation");

    return formABRT_APdu(_tReq, tDlgSvcData->getTransactionId(), use_udt);
  case TC_UAbort_Req::uabrtDialogueEXT:
    return formEXTERNAL_APdu(_tReq, tDlgSvcData->getTransactionId(), use_udt);
  default:
    throw utilx::SerializationException("TUAbortReqComposer::serialize2UDT::: invalid abortKind value=[%d]", _tReq.getAbortKind());
  }
  return TDlgRequestComposerAC::srlzError;
}

TDlgRequestComposerAC::SerializationResult_e
TUAbortReqComposer::formAARE_APdu(const TC_UAbort_Req& t_req,
                                  const proto::TransactionId& transaction_id,
                                  SUAUnitdataReq& used_udt) const
{
  proto::enc::AARE_APdu aareAPdu(proto::enc::AARE_APdu::protoVersion1);
  aareAPdu.rejectByUser(t_req.getRejectCause());
  aareAPdu.setAppCtxName(t_req.getAppCtx());
  aareAPdu.setUserInfo(t_req.getUserInfo());
  proto::enc::AbortMessage tcapMsg(transaction_id, &aareAPdu);
  return encodeMessage(tcapMsg, used_udt, "TUAbortReqComposer::formAARE_APdu");
}

TDlgRequestComposerAC::SerializationResult_e
TUAbortReqComposer::formABRT_APdu(const TC_UAbort_Req& t_req,
                                  const proto::TransactionId& transaction_id,
                                  SUAUnitdataReq& used_udt) const
{
  proto::enc::ABRT_APdu abrtAPdu(t_req.isAbortFromTCUser() ? proto::enc::ABRT_APdu::dlgServiceUser
                                                           : proto::enc::ABRT_APdu::dlgServiceProvider);
  abrtAPdu.setUserInfo(t_req.getUserInfo());
  proto::enc::AbortMessage tcapMsg(transaction_id, &abrtAPdu);
  return encodeMessage(tcapMsg, used_udt, "TUAbortReqComposer::formABRT_APdu");
}

TDlgRequestComposerAC::SerializationResult_e
TUAbortReqComposer::formEXTERNAL_APdu(const TC_UAbort_Req& t_req,
                                      const proto::TransactionId& transaction_id,
                                      SUAUnitdataReq& used_udt) const
{
//  proto::enc::EXTERNAL_APdu externalAPdu(t_req.getUserInfo());
//
//  proto::enc::AbortMessage tcapMsg(transaction_id, &externalAPdu);
//  return encodeMessage(tcapMsg, used_udt, "TUAbortReqComposer::formEXTERNAL_APdu");
  return srlzOk;
}
   
} //provd
} //tcap
} //eyeline

