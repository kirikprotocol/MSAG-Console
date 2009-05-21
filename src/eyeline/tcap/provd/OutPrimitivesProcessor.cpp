#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/Exception.hpp"
#include "eyeline/tcap/provd/OutPrimitivesProcessor.hpp"
#include "eyeline/tcap/provd/TimeoutMonitor.hpp"


namespace eyeline {
namespace tcap {
namespace provd {

void
OutPrimitivesProcessor::updateDialogue(TC_Begin_Req* beginReqPrimitive)
{
  const TDialogueId& tDialogueId = beginReqPrimitive->getDialogueId();

  TDialogueServiceDataRegistry::registry_element_ref_t tDlgSvcData = TDialogueServiceDataRegistry::getInstance().getTDialogueServiceData(tDialogueId);
  tDlgSvcData->updateDialogueDataByRequest(beginReqPrimitive);

  TBeginReqSender tPrimitiveSender(*beginReqPrimitive);

  TDlgRequestSenderAC::SerializationResult_e srlzResult =
    tPrimitiveSender.serialize2UDT(tDlgSvcData->getSrcAddr(), tDlgSvcData->getDstAddr());
  //TODO: analyze srlzResult and generate appropriate indication

  TReqSendResult callResult = tPrimitiveSender.sendMessage(_suaApi);
  if ( callResult.resultCode == TReqSendResult::SEND_OK ) {
    tDlgSvcData->activateDialogueTimer();

    activateTimers(tDlgSvcData, beginReqPrimitive->CompList());

    tDlgSvcData->setLinkNum(callResult.linkNum);
  } else if ( beginReqPrimitive->getReturnOnError() )
    noticeTCUser(tDlgSvcData, tDialogueId, callResult.resultCode);
}

void
OutPrimitivesProcessor::updateDialogue(TC_Cont_Req* contReqPrimitive)
{
  const TDialogueId& tDialogueId = contReqPrimitive->getDialogueId();

  TDialogueServiceDataRegistry::registry_element_ref_t tDlgSvcData = TDialogueServiceDataRegistry::getInstance().getTDialogueServiceData(tDialogueId);
  tDlgSvcData->updateDialogueDataByRequest(contReqPrimitive);

  TContReqSender tPrimitiveSender(*contReqPrimitive);

  TDlgRequestSenderAC::SerializationResult_e srlzResult =
    tPrimitiveSender.serialize2UDT(tDlgSvcData->getSrcAddr(), tDlgSvcData->getDstAddr());
  //TODO: analyze srlzResult and generate appropriate indication

  TReqSendResult callResult = tPrimitiveSender.sendMessage(_suaApi, tDlgSvcData->getLinkNum());
  if ( callResult.resultCode != TReqSendResult::SEND_OK &&
       contReqPrimitive->getReturnOnError() )
    noticeTCUser(tDlgSvcData, tDialogueId, callResult.resultCode);
  else
    activateTimers(tDlgSvcData, contReqPrimitive->CompList());
}

void
OutPrimitivesProcessor::updateDialogue(TC_End_Req* endReqPrimitive)
{
  const TDialogueId& tDialogueId = endReqPrimitive->getDialogueId();

  TDialogueServiceDataRegistry::registry_element_ref_t tDlgSvcData = TDialogueServiceDataRegistry::getInstance().getTDialogueServiceData(tDialogueId);
  tDlgSvcData->updateDialogueDataByRequest(endReqPrimitive);

  tDlgSvcData->cancelDialogueTimers();

  if ( endReqPrimitive->getTermination() == TC_End_Req::BASIC_END ) {
    TEndReqSender tPrimitiveSender(*endReqPrimitive);

    TDlgRequestSenderAC::SerializationResult_e srlzResult =
      tPrimitiveSender.serialize2UDT(tDlgSvcData->getSrcAddr(), tDlgSvcData->getDstAddr());
    //TODO: analyze srlzResult and generate appropriate indication

    TReqSendResult callResult = tPrimitiveSender.sendMessage(_suaApi, tDlgSvcData->getLinkNum());
    if ( callResult.resultCode != TReqSendResult::SEND_OK &&
         endReqPrimitive->getReturnOnError())
      noticeTCUser(tDlgSvcData, tDialogueId, callResult.resultCode);
    else
      activateTimers(tDlgSvcData, endReqPrimitive->CompList());
  }
}

void
OutPrimitivesProcessor::updateDialogue(TC_UAbort_Req* uAbortReqPrimitive)
{
  const TDialogueId& tDialogueId = uAbortReqPrimitive->getDialogueId();

  TDialogueServiceDataRegistry::registry_element_ref_t tDlgSvcData = TDialogueServiceDataRegistry::getInstance().getTDialogueServiceData(tDialogueId);
  tDlgSvcData->updateDialogueDataByRequest(uAbortReqPrimitive);

  tDlgSvcData->cancelDialogueTimers();

  TUAbortReqSender tPrimitiveSender(*uAbortReqPrimitive);

  TDlgRequestSenderAC::SerializationResult_e srlzResult =
    tPrimitiveSender.serialize2UDT(tDlgSvcData->getSrcAddr(), tDlgSvcData->getDstAddr());
  //TODO: analyze srlzResult and generate appropriate indication

  TReqSendResult callResult = tPrimitiveSender.sendMessage(_suaApi, tDlgSvcData->getLinkNum());
  if ( callResult.resultCode != TReqSendResult::SEND_OK &&
       uAbortReqPrimitive->getReturnOnError() )
    noticeTCUser(tDlgSvcData, tDialogueId, callResult.resultCode);
}

void
OutPrimitivesProcessor::updateDialogue(TC_PAbort_Req* pAbortReqPrimitive)
{
  const TDialogueId& tDialogueId = pAbortReqPrimitive->getDialogueId();

  TDialogueServiceDataRegistry::registry_element_ref_t tDlgSvcData = TDialogueServiceDataRegistry::getInstance().getTDialogueServiceData(tDialogueId);
  tDlgSvcData->updateDialogueDataByRequest(pAbortReqPrimitive);

  tDlgSvcData->cancelDialogueTimers();

  TPAbortReqSender tPrimitiveSender(*pAbortReqPrimitive);

  TDlgRequestSenderAC::SerializationResult_e srlzResult =
    tPrimitiveSender.serialize2UDT(tDlgSvcData->getSrcAddr(), tDlgSvcData->getDstAddr());
  //TODO: analyze srlzResult and generate appropriate indication

  tPrimitiveSender.sendMessage(_suaApi, tDlgSvcData->getLinkNum());
}

void
OutPrimitivesProcessor::sendPrimitive(TC_PAbort_Req* p_abort_req_primitive, unsigned int link_num,
                                      const SCCPAddress& src_addr, const SCCPAddress& dst_addr)
{
  TPAbortReqSender tPrimitiveSender(*p_abort_req_primitive);

  TDlgRequestSenderAC::SerializationResult_e srlzResult =
    tPrimitiveSender.serialize2UDT(src_addr, dst_addr);
  //TODO: analyze srlzResult and generate appropriate indication

  tPrimitiveSender.sendMessage(_suaApi, link_num);
}

void
OutPrimitivesProcessor::sendPrimitive(TC_UAbort_Req* u_abort_req_primitive, unsigned int link_num,
                                      const SCCPAddress& src_addr, const SCCPAddress& dst_addr)
{
  TUAbortReqSender tPrimitiveSender(*u_abort_req_primitive);

  TDlgRequestSenderAC::SerializationResult_e srlzResult =
    tPrimitiveSender.serialize2UDT(src_addr, dst_addr);
  //TODO: analyze srlzResult and generate appropriate indication

  tPrimitiveSender.sendMessage(_suaApi, link_num);
}

void
OutPrimitivesProcessor::noticeTCUser(TDialogueServiceDataRegistry::registry_element_ref_t& tDlgSvcData,
                                     const TDialogueId& tDialogueId,
                                     TReqSendResult::ResultCode_e resultCode)
{
  TC_Notice_Ind tcNoticeIndication;

  tcNoticeIndication.setDialogueId(tDialogueId);
  if ( resultCode == TReqSendResult::INVALID_DATA )
    tcNoticeIndication.setReportCause(TC_Notice_Ind::INVALID_TCUSER_DATA);
  else
    tcNoticeIndication.setReportCause(TC_Notice_Ind::SCCP_ERROR);

  tDlgSvcData->updateDialogueDataByIndication(&tcNoticeIndication);
}

void
OutPrimitivesProcessor::activateTimers(TDialogueServiceDataRegistry::registry_element_ref_t& tDlgSvcData,
                                       const ROSComponentsList * compList)
{
  if (!compList || compList->empty())
    return;

  for (ros::ROSComponentsList::const_iterator it = compList->begin(); it != compList->end(); ++it) {
    ros::ROSComponentPrimitive* rosCompPrimitive = it->get();
    if ( rosCompPrimitive && rosCompPrimitive->rosKind() == ros::ROSComponentPrimitive::rosInvoke ) {
      const ros::ROSInvoke* invoke = static_cast<const ros::ROSInvoke*>(rosCompPrimitive);
      if (invoke->getOperationClass() != ros::ROSInvoke::Class_4)
        tDlgSvcData->activateInvocationTimer(invoke->getTimeout(), invoke->getInvokeId());
    }
  }
}

}}}
