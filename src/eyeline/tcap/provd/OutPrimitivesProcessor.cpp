#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/Exception.hpp"
#include "eyeline/tcap/TComponentIndicationPrimitives.hpp"
#include "eyeline/tcap/provd/OutPrimitivesProcessor.hpp"
#include "eyeline/tcap/provd/TimeoutMonitor.hpp"
#include "eyeline/tcap/provd/TDlgPrimitivesUtils.hpp"
#include "TCompIndComposers.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

void
OutPrimitivesProcessor::updateDialogue(const TC_Begin_Req & begin_req_primitive)
{
  const TDialogueId& tDialogueId = begin_req_primitive.getDialogueId();

  TDialogueServiceDataRegistry::registry_element_ref_t 
  tDlgSvcData = TDialogueServiceDataRegistry::getInstance().getTDialogueServiceData(tDialogueId);

  TBeginReqComposer tReqComposer(begin_req_primitive);
  tDlgSvcData->updateDialogueDataByRequest(tReqComposer); //sets TransactionId

  //TODO: get SCCP standard from config and initialize SUAUnitdataReq accordingly.
  //For now just use SCCPStandard::itut93
  SUAUnitdataReq  reqUDT(SCCPStandard::itut93);
  TDlgRequestComposerAC::SerializationResult_e srlzResult =
    tReqComposer.serialize2UDT(reqUDT, tDlgSvcData->getSrcAddr(), tDlgSvcData->getDstAddr());

  if ( srlzResult == TDlgRequestComposerAC::srlzOk ) {
    SuaApi::CallResult rc = sendMessage(reqUDT);
    if ( rc.operationResult == SuaApi::OK ) {
      tDlgSvcData->activateDialogueTimer();

      activateTimers(tDlgSvcData.Get(), &begin_req_primitive.CompList());

      tDlgSvcData->setLinkNum(rc.suaConnectNum);
    } else {
      smsc_log_error(_logger, "OutPrimitivesProcessor::updateDialogue::: TC_Begin_Req primitive sending error=[%d]", rc.operationResult);
      if ( begin_req_primitive.getReturnOnError() )
        noticeTCUser(tDlgSvcData.Get(), tDialogueId, convertSuaApiError2TNoticeCause(rc.operationResult));
    }
  } else {
    smsc_log_error(_logger, "OutPrimitivesProcessor::updateDialogue::: TC_Begin_Req primitive serialization error=[%d]", srlzResult);
    analyzeFailureCauseAndNotifyTCUser(srlzResult, tDialogueId, tDlgSvcData.Get(), begin_req_primitive.getReturnOnError());
  }
}

void
OutPrimitivesProcessor::updateDialogue(const TC_Cont_Req & cont_req_primitive)
{
  const TDialogueId& tDialogueId = cont_req_primitive.getDialogueId();

  TDialogueServiceDataRegistry::registry_element_ref_t tDlgSvcData = TDialogueServiceDataRegistry::getInstance().getTDialogueServiceData(tDialogueId);

  TContReqComposer tReqComposer(cont_req_primitive);
  tDlgSvcData->updateDialogueDataByRequest(tReqComposer); //sets TransactionId

  //TODO: get SCCP standard from config and initialize SUAUnitdataReq accordingly.
  //For now just use SCCPStandard::itut93
  SUAUnitdataReq  reqUDT(SCCPStandard::itut93);
  TDlgRequestComposerAC::SerializationResult_e srlzResult =
    tReqComposer.serialize2UDT(reqUDT, tDlgSvcData->getSrcAddr(), tDlgSvcData->getDstAddr());

  if ( srlzResult == TDlgRequestComposerAC::srlzOk ) {
    SuaApi::CallResult rc = sendMessage(reqUDT, tDlgSvcData->getLinkNum());
    if ( rc.operationResult == SuaApi::OK ) {
      tDlgSvcData->activateDialogueTimer();

      activateTimers(tDlgSvcData.Get(), &cont_req_primitive.CompList());

      tDlgSvcData->setLinkNum(rc.suaConnectNum);
    } else {
      smsc_log_error(_logger, "OutPrimitivesProcessor::updateDialogue::: TC_Cont_Req primitive sending error=[%d]", rc.operationResult);
      if ( cont_req_primitive.getReturnOnError() )
        noticeTCUser(tDlgSvcData.Get(), tDialogueId, convertSuaApiError2TNoticeCause(rc.operationResult));
    }
  } else {
    smsc_log_error(_logger, "OutPrimitivesProcessor::updateDialogue::: TC_Cont_Req primitive serialization error=[%d]", srlzResult);
    analyzeFailureCauseAndNotifyTCUser(srlzResult, tDialogueId, tDlgSvcData.Get(), cont_req_primitive.getReturnOnError());
  }
}

void
OutPrimitivesProcessor::updateDialogue(const TC_End_Req & end_req_primitive)
{
  const TDialogueId& tDialogueId = end_req_primitive.getDialogueId();

  TDialogueServiceDataRegistry::registry_element_ref_t tDlgSvcData = TDialogueServiceDataRegistry::getInstance().getTDialogueServiceData(tDialogueId);

  TEndReqComposer tReqComposer(end_req_primitive);
  tDlgSvcData->updateDialogueDataByRequest(tReqComposer); //sets TransactionId
  tDlgSvcData->cancelDialogueTimers();

  if ( end_req_primitive.getTermination() == TC_End_Req::endBASIC ) {
    //TODO: get SCCP standard from config and initialize SUAUnitdataReq accordingly.
    //For now just use SCCPStandard::itut93
    SUAUnitdataReq  reqUDT(SCCPStandard::itut93);
    TDlgRequestComposerAC::SerializationResult_e srlzResult =
      tReqComposer.serialize2UDT(reqUDT, tDlgSvcData->getSrcAddr(), tDlgSvcData->getDstAddr());

    if ( srlzResult == TDlgRequestComposerAC::srlzOk ) {
      SuaApi::CallResult rc = sendMessage(reqUDT, tDlgSvcData->getLinkNum());
      if ( rc.operationResult != SuaApi::OK ) {
        smsc_log_error(_logger, "OutPrimitivesProcessor::updateDialogue::: TC_End_Req primitive sending error=[%d]", rc.operationResult);
      }
    } else {
      smsc_log_error(_logger, "OutPrimitivesProcessor::updateDialogue::: TC_End_Req primitive serialization error=[%d]", srlzResult);
      analyzeFailureCauseAndNotifyTCUser(srlzResult, tDialogueId, tDlgSvcData.Get(), end_req_primitive.getReturnOnError());
    }
  }
}

void
OutPrimitivesProcessor::updateDialogue(const TC_UAbort_Req & u_abort_req_primitive)
{
  const TDialogueId& tDialogueId = u_abort_req_primitive.getDialogueId();

  TDialogueServiceDataRegistry::registry_element_ref_t tDlgSvcData = TDialogueServiceDataRegistry::getInstance().getTDialogueServiceData(tDialogueId);

  TUAbortReqComposer tReqComposer(u_abort_req_primitive);
  tDlgSvcData->updateDialogueDataByRequest(tReqComposer); //sets TransactionId
  tDlgSvcData->cancelDialogueTimers();

  //TODO: get SCCP standard from config and initialize SUAUnitdataReq accordingly.
  //For now just use SCCPStandard::itut93
  SUAUnitdataReq  reqUDT(SCCPStandard::itut93);
  TDlgRequestComposerAC::SerializationResult_e srlzResult =
    tReqComposer.serialize2UDT(reqUDT, tDlgSvcData->getSrcAddr(), tDlgSvcData->getDstAddr());

  if ( srlzResult == TDlgRequestComposerAC::srlzOk ) {
    SuaApi::CallResult rc = sendMessage(reqUDT, tDlgSvcData->getLinkNum());
    if ( rc.operationResult != SuaApi::OK ) {
      smsc_log_error(_logger, "OutPrimitivesProcessor::updateDialogue::: TC_UAbort_Req primitive sending error=[%d]", rc.operationResult);
    }
  } else {
    smsc_log_error(_logger, "OutPrimitivesProcessor::updateDialogue::: TC_UAbort_Req primitive serialization error=[%d]", srlzResult);
    analyzeFailureCauseAndNotifyTCUser(srlzResult, tDialogueId, tDlgSvcData.Get(), u_abort_req_primitive.getReturnOnError());
  }
}

void
OutPrimitivesProcessor::updateDialogue(const TC_PAbort_Req & p_abort_req_primitive)
{
  const TDialogueId& tDialogueId = p_abort_req_primitive.getDialogueId();

  TDialogueServiceDataRegistry::registry_element_ref_t tDlgSvcData = TDialogueServiceDataRegistry::getInstance().getTDialogueServiceData(tDialogueId);

  TPAbortReqComposer tReqComposer(p_abort_req_primitive);
  tDlgSvcData->updateDialogueDataByRequest(tReqComposer); //sets TransactionId
  tDlgSvcData->cancelDialogueTimers();

  //TODO: get SCCP standard from config and initialize SUAUnitdataReq accordingly.
  //For now just use SCCPStandard::itut93
  SUAUnitdataReq  reqUDT(SCCPStandard::itut93);
  TDlgRequestComposerAC::SerializationResult_e srlzResult =
    tReqComposer.serialize2UDT(reqUDT, tDlgSvcData->getSrcAddr(), tDlgSvcData->getDstAddr());

  if ( srlzResult == TDlgRequestComposerAC::srlzOk ) {
    SuaApi::CallResult rc = sendMessage(reqUDT, tDlgSvcData->getLinkNum());
    if ( rc.operationResult != SuaApi::OK )
      smsc_log_error(_logger, "OutPrimitivesProcessor::updateDialogue::: TC_PAbort_Req primitive sending error=[%d]", rc.operationResult);
  } else {
    smsc_log_error(_logger, "OutPrimitivesProcessor::updateDialogue::: TC_PAbort_Req primitive serialization error=[%d]", srlzResult);
    analyzeFailureCauseAndNotifyTCUser(srlzResult, tDialogueId, tDlgSvcData.Get(), p_abort_req_primitive.getReturnOnError());
  }
}

void
OutPrimitivesProcessor::sendPrimitive(const TPAbortReqComposer & p_abort_req, unsigned int link_num,
                                      const SCCPAddress& src_addr, const SCCPAddress& dst_addr) const
{
  //TODO: get SCCP standard from config and initialize SUAUnitdataReq accordingly.
  //For now just use SCCPStandard::itut93
  SUAUnitdataReq  reqUDT(SCCPStandard::itut93);
  TDlgRequestComposerAC::SerializationResult_e srlzResult =
      p_abort_req.serialize2UDT(reqUDT, src_addr, dst_addr);

  if ( srlzResult == TDlgRequestComposerAC::srlzOk ) {
    SuaApi::CallResult rc = sendMessage(reqUDT, link_num);
    if ( rc.operationResult != SuaApi::OK )
      smsc_log_error(_logger, "OutPrimitivesProcessor::sendPrimitive::: TC_PAbort_Req primitive sending error=[%d]", rc.operationResult);
  } else
    smsc_log_error(_logger, "OutPrimitivesProcessor::sendPrimitive::: TC_PAbort_Req primitive serialization error=[%d]", srlzResult);
}

void
OutPrimitivesProcessor::sendPrimitive(const TUAbortReqComposer & u_abort_req, unsigned int link_num,
                                      const SCCPAddress& src_addr, const SCCPAddress& dst_addr) const
{
  //TODO: get SCCP standard from config and initialize SUAUnitdataReq accordingly.
  //For now just use SCCPStandard::itut93
  SUAUnitdataReq  reqUDT(SCCPStandard::itut93);
  TDlgRequestComposerAC::SerializationResult_e srlzResult =
      u_abort_req.serialize2UDT(reqUDT, src_addr, dst_addr);

  if ( srlzResult == TDlgRequestComposerAC::srlzOk ) {
    SuaApi::CallResult rc = sendMessage(reqUDT, link_num);
    if ( rc.operationResult != SuaApi::OK )
      smsc_log_error(_logger, "OutPrimitivesProcessor::sendPrimitive::: TC_UAbort_Req primitive sending error=[%d]", rc.operationResult);
  } else
    smsc_log_error(_logger, "OutPrimitivesProcessor::sendPrimitive::: TC_UAbort_Req primitive serialization error=[%d]", srlzResult);
}

void
OutPrimitivesProcessor::noticeTCUser(TDialogueServiceData* t_dlg_svc_data,
                                     const TDialogueId& t_dialogue_id,
                                     TC_Notice_Ind::ReportCause_e r_cause)
{
  TNoticeIndComposer tcNoticeIndication;

  tcNoticeIndication.setDialogueId(t_dialogue_id);
  tcNoticeIndication.setReportCause(r_cause);

  t_dlg_svc_data->updateDialogueDataByIndication(tcNoticeIndication);
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
OutPrimitivesProcessor::analyzeFailureCauseAndNotifyTCUser(TDlgRequestComposerAC::SerializationResult_e res_status,
                                                           const TDialogueId& t_dialogue_id,
                                                           TDialogueServiceData* t_dlg_svc_data,
                                                           bool return_on_error)
{
  if ( res_status == TDlgRequestComposerAC::srlzBadComponentPortion )
    rejectComponent(t_dlg_svc_data, t_dialogue_id, RejectProblem(RejectProblem::rjg_mistypedPDU));
  else if ( res_status == TDlgRequestComposerAC::srlzTooMuchComponents ) {
    rejectComponent(t_dlg_svc_data, t_dialogue_id, RejectProblem(RejectProblem::rjg_badlyStructuredPDU));
  } else {
    if ( !return_on_error )
      return;

    TNoticeIndComposer tcNoticeIndication;

    tcNoticeIndication.setDialogueId(t_dialogue_id);
    if ( res_status == TDlgRequestComposerAC::srlzBadSrcAddr ||
         res_status == TDlgRequestComposerAC::srlzBadDstAddr )
      tcNoticeIndication.setReportCause(TC_Notice_Ind::errNoAdrTranslation);
    else if ( res_status == TDlgRequestComposerAC::srlzBadTransactionPortion )
      tcNoticeIndication.setReportCause(TC_Notice_Ind::errTCAPFailure);
    else if ( res_status == TDlgRequestComposerAC::srlzBadDialoguePortion )
      tcNoticeIndication.setReportCause(TC_Notice_Ind::errDialoguePortion);
    else
      tcNoticeIndication.setReportCause(TC_Notice_Ind::errTCAPFailure);

    t_dlg_svc_data->updateDialogueDataByIndication(tcNoticeIndication);
  }
}

void
OutPrimitivesProcessor::rejectComponent(TDialogueServiceData* t_dlg_svc_data,
                                        const TDialogueId& t_dialogue_id,
                                        const RejectProblem & rej_problem)
{
  TC_L_Reject_Ind_Composer tcLRejectIndication;
  tcLRejectIndication.setDialogueId(t_dialogue_id);
  tcLRejectIndication.setProblem(rej_problem);

  t_dlg_svc_data->notifyTCUserLocally(tcLRejectIndication);
}

}}}
