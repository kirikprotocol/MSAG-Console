#ifndef __EYELINE_TCAP_PROVD_OUTPRIMITIVESPROCESSOR_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROVD_OUTPRIMITIVESPROCESSOR_HPP__

# include "logger/Logger.h"
# include "eyeline/ros/ROSRejectProblem.hpp"
# include "eyeline/sua/libsua/SuaApi.hpp"
# include "eyeline/tcap/provd/TDlgReqComposers.hpp"
# include "eyeline/tcap/provd/TDialogueServiceDataRegistry.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

using sua::libsua::SuaApi;
using eyeline::ros::RejectProblem;

class OutPrimitivesProcessor {
public:
  explicit OutPrimitivesProcessor(SuaApi* sua_api)
    : _logger(smsc::logger::Logger::getInstance("tcap.provd")), _suaApi(sua_api)
  {}

  void updateDialogue(const TC_Begin_Req & begin_req_primitive);
  void updateDialogue(const TC_Cont_Req & cont_req_primitive);
  void updateDialogue(const TC_End_Req & end_req_primitive);
  void updateDialogue(const TC_UAbort_Req & u_abort_req_primitive);
  void updateDialogue(const TC_PAbort_Req & p_abort_req_primitive);

  void sendPrimitive(const TPAbortReqComposer & p_abort_req, unsigned int link_num,
                     const SCCPAddress& src_addr, const SCCPAddress& dst_addr) const;
  void sendPrimitive(const TUAbortReqComposer & u_abort_req, unsigned int link_num,
                     const SCCPAddress& src_addr, const SCCPAddress& dst_addr) const;
protected:
  void noticeTCUser(TDialogueServiceData* t_dlg_svc_data,
                    const TDialogueId& t_dialogue_id,
                    TC_Notice_Ind::ReportCause_e r_cause);

  void activateTimers(TDialogueServiceData* t_dlg_svc_data,
                      const ROSComponentsList * comp_list);

  void analyzeFailureCauseAndNotifyTCUser(TDlgRequestComposerAC::SerializationResult_e res_status,
                                          const TDialogueId& t_dialogue_id,
                                          TDialogueServiceData* t_dlg_svc_data,
                                          bool return_on_error);

  void rejectComponent(TDialogueServiceData* t_dlg_svc_data,
                       const TDialogueId& t_dialogue_id,
                       const RejectProblem & rej_problem);

  SuaApi::CallResult sendMessage(const SUAUnitdataReq & use_udt) const
  {
    return _suaApi->unitdata_req(use_udt.userData(), use_udt.userDataLen(),
                                 use_udt.calledAddr(), use_udt.calledAddrLen(),
                                 use_udt.callingAddr(), use_udt.callingAddrLen(),
                                 use_udt);
  }

  SuaApi::CallResult sendMessage(const SUAUnitdataReq & use_udt, unsigned int link_num) const
  {
    return _suaApi->unitdata_req(use_udt.userData(), use_udt.userDataLen(),
                                 use_udt.calledAddr(), use_udt.calledAddrLen(),
                                 use_udt.callingAddr(), use_udt.callingAddrLen(),
                                 use_udt, link_num);
  }

private:
  smsc::logger::Logger* _logger;
  SuaApi * _suaApi;
};

}}}

#endif /* __EYELINE_TCAP_PROVD_OUTPRIMITIVESPROCESSOR_HPP__ */

