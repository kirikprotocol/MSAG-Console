#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/sccp/SCCPDefs.hpp"
#include "eyeline/tcap/provd/TDlgReqComposers.hpp"

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
  if (!use_udt.setCalledAddr(src_addr))
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

  //TODO: serialize TEndReq to use_udt.dataBuf() taking in account 'maxDataSz'
  //If serialized data is too long determine the portion of TCAPMessage that is
  //too large (either UserInfo or ComponentsList) and return associated value
  //of SerializationResult_e
  return TDlgRequestComposerAC::srlzBadTransactionPortion;
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

  uint16_t maxDataSz = use_udt.dataBuf().getMaxSize();

  //TODO: serialize TPAbortReq to use_udt.dataBuf(), perfoming a paranoic check
  //for 'maxDataSz'.
  return TDlgRequestComposerAC::srlzBadTransactionPortion;
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

  uint16_t maxDataSz = use_udt.dataBuf().getMaxSize();

  //TODO: serialize TUAbortReq to use_udt.dataBuf() taking in account 'maxDataSz'
  //If serialized data is too long determine the portion of TCAPMessage that is
  //too large (actually it may be only a UserInfo) and return associated value
  //of SerializationResult_e
  return TDlgRequestComposerAC::srlzBadTransactionPortion;
}


} //provd
} //tcap
} //eyeline

