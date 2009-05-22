#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/sua/libsua/SuaApi.hpp"
#include "eyeline/tcap/Exception.hpp"
#include "eyeline/tcap/TComponentIndicationPrimitives.hpp"
#include "OutPrimitivesProcessor.hpp"
#include "TimeoutMonitor.hpp"
#include "TDlgIndDispatcher.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

void
OutPrimitivesProcessor::updateDialogue(TC_Begin_Req* begin_req_primitive)
{
  const TDialogueId& tDialogueId = begin_req_primitive->getDialogueId();

  TDialogueServiceDataRegistry::registry_element_ref_t tDlgSvcData = TDialogueServiceDataRegistry::getInstance().getTDialogueServiceData(tDialogueId);
  tDlgSvcData->updateDialogueDataByRequest(begin_req_primitive);

  TBeginReqSender tPrimitiveSender(*begin_req_primitive);

  TDlgRequestSenderAC::SerializationResult_e srlzResult =
    tPrimitiveSender.serialize2UDT(tDlgSvcData->getSrcAddr(), tDlgSvcData->getDstAddr());

  if ( srlzResult == TDlgRequestSenderAC::srlzOk ) {
    sua::libsua::SuaApi::CallResult rc = tPrimitiveSender.sendMessage(_suaApi);
    if ( rc.operationResult == sua::libsua::SuaApi::OK ) {
      tDlgSvcData->activateDialogueTimer();

      activateTimers(tDlgSvcData.Get(), begin_req_primitive->CompList());

      tDlgSvcData->setLinkNum(rc.suaConnectNum);
    } else if ( begin_req_primitive->getReturnOnError() )
      noticeTCUser(tDlgSvcData.Get(), tDialogueId, convertSuaApiError2TNoticeCause(rc.operationResult));
  } else
    analyzeFailureCauseAndNotifyTCUser(srlzResult, tDialogueId, tDlgSvcData.Get(), begin_req_primitive->getReturnOnError());
}

void
OutPrimitivesProcessor::updateDialogue(TC_Cont_Req* cont_req_primitive)
{
  const TDialogueId& tDialogueId = cont_req_primitive->getDialogueId();

  TDialogueServiceDataRegistry::registry_element_ref_t tDlgSvcData = TDialogueServiceDataRegistry::getInstance().getTDialogueServiceData(tDialogueId);
  tDlgSvcData->updateDialogueDataByRequest(cont_req_primitive);

  TContReqSender tPrimitiveSender(*cont_req_primitive);

  TDlgRequestSenderAC::SerializationResult_e srlzResult =
    tPrimitiveSender.serialize2UDT(tDlgSvcData->getSrcAddr(), tDlgSvcData->getDstAddr());
  //TODO: analyze srlzResult and generate appropriate indication

  sua::libsua::SuaApi::CallResult rc = tPrimitiveSender.sendMessage(_suaApi, tDlgSvcData->getLinkNum());
  if ( rc.operationResult != sua::libsua::SuaApi::OK &&
       cont_req_primitive->getReturnOnError() )
    noticeTCUser(tDlgSvcData.Get(), tDialogueId, convertSuaApiError2TNoticeCause(rc.operationResult));
  else
    activateTimers(tDlgSvcData.Get(), cont_req_primitive->CompList());
}

void
OutPrimitivesProcessor::updateDialogue(TC_End_Req* end_req_primitive)
{
  const TDialogueId& tDialogueId = end_req_primitive->getDialogueId();

  TDialogueServiceDataRegistry::registry_element_ref_t tDlgSvcData = TDialogueServiceDataRegistry::getInstance().getTDialogueServiceData(tDialogueId);
  tDlgSvcData->updateDialogueDataByRequest(end_req_primitive);

  tDlgSvcData->cancelDialogueTimers();

  if ( end_req_primitive->getTermination() == TC_End_Req::BASIC_END ) {
    TEndReqSender tPrimitiveSender(*end_req_primitive);

    TDlgRequestSenderAC::SerializationResult_e srlzResult =
      tPrimitiveSender.serialize2UDT(tDlgSvcData->getSrcAddr(), tDlgSvcData->getDstAddr());
    //TODO: analyze srlzResult and generate appropriate indication

    sua::libsua::SuaApi::CallResult rc = tPrimitiveSender.sendMessage(_suaApi, tDlgSvcData->getLinkNum());
    if ( rc.operationResult != sua::libsua::SuaApi::OK &&
         end_req_primitive->getReturnOnError())
      noticeTCUser(tDlgSvcData.Get(), tDialogueId, convertSuaApiError2TNoticeCause(rc.operationResult));
    else
      activateTimers(tDlgSvcData.Get(), end_req_primitive->CompList());
  }
}

void
OutPrimitivesProcessor::updateDialogue(TC_UAbort_Req* u_abort_req_primitive)
{
  const TDialogueId& tDialogueId = u_abort_req_primitive->getDialogueId();

  TDialogueServiceDataRegistry::registry_element_ref_t tDlgSvcData = TDialogueServiceDataRegistry::getInstance().getTDialogueServiceData(tDialogueId);
  tDlgSvcData->updateDialogueDataByRequest(u_abort_req_primitive);

  tDlgSvcData->cancelDialogueTimers();

  TUAbortReqSender tPrimitiveSender(*u_abort_req_primitive);

  TDlgRequestSenderAC::SerializationResult_e srlzResult =
    tPrimitiveSender.serialize2UDT(tDlgSvcData->getSrcAddr(), tDlgSvcData->getDstAddr());
  //TODO: analyze srlzResult and generate appropriate indication

  sua::libsua::SuaApi::CallResult rc = tPrimitiveSender.sendMessage(_suaApi, tDlgSvcData->getLinkNum());
  if ( rc.operationResult != sua::libsua::SuaApi::OK &&
       u_abort_req_primitive->getReturnOnError() )
    noticeTCUser(tDlgSvcData.Get(), tDialogueId, convertSuaApiError2TNoticeCause(rc.operationResult));
}

void
OutPrimitivesProcessor::updateDialogue(TC_PAbort_Req* p_abort_req_primitive)
{
  const TDialogueId& tDialogueId = p_abort_req_primitive->getDialogueId();

  TDialogueServiceDataRegistry::registry_element_ref_t tDlgSvcData = TDialogueServiceDataRegistry::getInstance().getTDialogueServiceData(tDialogueId);
  tDlgSvcData->updateDialogueDataByRequest(p_abort_req_primitive);

  tDlgSvcData->cancelDialogueTimers();

  TPAbortReqSender tPrimitiveSender(*p_abort_req_primitive);

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
OutPrimitivesProcessor::noticeTCUser(TDialogueServiceData* t_dlg_svc_data,
                                     const TDialogueId& t_dialogue_id,
                                     TC_Notice_Ind::ReportCause_e r_cause)
{
  TC_Notice_Ind tcNoticeIndication;

  tcNoticeIndication.setDialogueId(t_dialogue_id);
  tcNoticeIndication.setReportCause(r_cause);

  t_dlg_svc_data->updateDialogueDataByIndication(&tcNoticeIndication);
}

void
OutPrimitivesProcessor::activateTimers(TDialogueServiceData* t_dlg_svc_data,
                                       const ROSComponentsList * comp_list)
{
  if (!comp_list || comp_list->empty())
    return;

  for (ros::ROSComponentsList::const_iterator it = comp_list->begin(); it != comp_list->end(); ++it) {
    ros::ROSComponentPrimitive* rosCompPrimitive = it->get();
    if ( rosCompPrimitive && rosCompPrimitive->rosKind() == ros::ROSComponentPrimitive::rosInvoke ) {
      const ros::ROSInvoke* invoke = static_cast<const ros::ROSInvoke*>(rosCompPrimitive);
      if (invoke->getOperationClass() != ros::ROSInvoke::Class_4)
        t_dlg_svc_data->activateInvocationTimer(invoke->getTimeout(), invoke->getInvokeId());
    }
  }
}

void
OutPrimitivesProcessor::analyzeFailureCauseAndNotifyTCUser(TDlgRequestSenderAC::SerializationResult_e res_status,
                                                           const TDialogueId& t_dialogue_id,
                                                           TDialogueServiceData* t_dlg_svc_data,
                                                           bool return_on_error)
{
  if ( res_status == TDlgRequestSenderAC::srlzBadComponentPortion )
    rejectComponent(t_dlg_svc_data, t_dialogue_id, TC_L_Reject_Ind::BAD_COMPONENT_PROTION);
  else {
    if ( !return_on_error )
      return;

    TC_Notice_Ind tcNoticeIndication;

    tcNoticeIndication.setDialogueId(t_dialogue_id);
    if ( res_status == TDlgRequestSenderAC::srlzBadSrcAddr ||
         res_status == TDlgRequestSenderAC::srlzBadDstAddr )
      tcNoticeIndication.setReportCause(TC_Notice_Ind::errNoAdrTranslation);
    else if ( res_status == TDlgRequestSenderAC::srlzBadTransactionPortion )
      tcNoticeIndication.setReportCause(TC_Notice_Ind::errTCAPFailure);
    else if ( res_status == TDlgRequestSenderAC::srlzBadDialoguePortion )
      tcNoticeIndication.setReportCause(TC_Notice_Ind::errDialoguePortion);
    else
      tcNoticeIndication.setReportCause(TC_Notice_Ind::errTCAPFailure);

    t_dlg_svc_data->updateDialogueDataByIndication(&tcNoticeIndication);
  }
}

void
OutPrimitivesProcessor::rejectComponent(TDialogueServiceData* t_dlg_svc_data,
                                        const TDialogueId& t_dialogue_id,
                                        TC_L_Reject_Ind::problem_code_e problem_code)
{
  TC_L_Reject_Ind tcLRejectIndication;

  tcLRejectIndication.setDialogueId(t_dialogue_id);
  tcLRejectIndication.setProblemCode(problem_code);

  //tDlgSvcData->updateDialogueDataByIndication(&tcLRejectIndication);
}

}}}
