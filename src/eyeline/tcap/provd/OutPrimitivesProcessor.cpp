#include "OutPrimitivesProcessor.hpp"
#include "TimeoutMonitor.hpp"
#include "eyeline/tcap/Exception.hpp"
#include "eyeline/tcap/provd/TDlgReqSerializer.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

void
OutPrimitivesProcessor::updateDialogue(TC_Begin_Req* beginReqPrimitive)
{
  const TDialogueId& tDialogueId = beginReqPrimitive->getDialogueId();

  TDialogueServiceDataRegistry::registry_element_ref_t tDlgSvcData = TDialogueServiceDataRegistry::getInstance().getTDialogueServiceData(tDialogueId);
  tDlgSvcData->updateDialogueDataByRequest(beginReqPrimitive);

  TDlgRequestSerializerT<TC_Begin_Req> tPrimitiveSerializer(*beginReqPrimitive);

  TReqSendResult callResult = tPrimitiveSerializer.sendMessage(_suaApi);
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

  TDlgRequestSerializerT<TC_Cont_Req> tPrimitiveSerializer(*contReqPrimitive);

  TReqSendResult callResult = tPrimitiveSerializer.sendMessage(_suaApi, tDlgSvcData->getLinkNum());
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
    TDlgRequestSerializerT<TC_End_Req> tPrimitiveSerializer(*endReqPrimitive);

    TReqSendResult callResult = tPrimitiveSerializer.sendMessage(_suaApi, tDlgSvcData->getLinkNum());
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

  TDlgRequestSerializerT<TC_UAbort_Req> tPrimitiveSerializer(*uAbortReqPrimitive);

  TReqSendResult callResult = tPrimitiveSerializer.sendMessage(_suaApi, tDlgSvcData->getLinkNum());
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

  TDlgRequestSerializerT<TC_PAbort_Req> tPrimitiveSerializer(*pAbortReqPrimitive);

  tPrimitiveSerializer.sendMessage(_suaApi, tDlgSvcData->getLinkNum());
}

void
OutPrimitivesProcessor::sendPrimitive(TC_PAbort_Req* pAbortReqPrimitive, unsigned int linkNum)
{
  TDlgRequestSerializerT<TC_PAbort_Req> tPrimitiveSerializer(*pAbortReqPrimitive);

  tPrimitiveSerializer.sendMessage(_suaApi, linkNum);
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
                                       const ROSComponentsList& compList)
{
  for (ros::ROSComponentsList::const_iterator it = compList.begin(); it != compList.end(); ++it) {
    ros::ROSComponentPrimitive* rosCompPrimitive = it->get();
    if ( rosCompPrimitive && rosCompPrimitive->rosKind() == ros::ROSComponentPrimitive::rosInvoke ) {
      const ros::ROSInvoke* invoke = static_cast<const ros::ROSInvoke*>(rosCompPrimitive);
      if (invoke->getOperationClass() != ros::ROSInvoke::Class_4)
        tDlgSvcData->activateInvocationTimer(invoke->getTimeout(), invoke->getInvokeId());
    }
  }
}

}}}
