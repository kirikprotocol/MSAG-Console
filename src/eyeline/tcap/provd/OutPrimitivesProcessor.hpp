#ifndef __EYELINE_TCAP_PROVD_OUTPRIMITIVESPROCESSOR_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROVD_OUTPRIMITIVESPROCESSOR_HPP__

# include "eyeline/sua/libsua/SuaUser.hpp"
# include "eyeline/tcap/TDialogueId.hpp"
# include "eyeline/tcap/TDialogueRequestPrimitives.hpp"
# include "eyeline/tcap/provd/TDlgReqSender.hpp"
# include "eyeline/tcap/provd/TDialogueServiceDataRegistry.hpp"
# include "eyeline/tcap/provd/TDlgReqSender.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

class OutPrimitivesProcessor {
public:
  explicit OutPrimitivesProcessor(sua::libsua::SuaApi* sua_api)
    : _suaApi(sua_api)
  {}

  void updateDialogue(TC_Begin_Req* begin_req_primitive);
  void updateDialogue(TC_Cont_Req* cont_req_primitive);
  void updateDialogue(TC_End_Req* end_req_primitive);
  void updateDialogue(TC_UAbort_Req* u_abort_req_primitive);
  void updateDialogue(TC_PAbort_Req* p_abort_req_primitive);

  void sendPrimitive(TC_PAbort_Req* p_abort_req_primitive, unsigned int link_num,
                     const SCCPAddress& src_addr, const SCCPAddress& dst_addr);
  void sendPrimitive(TC_UAbort_Req* u_abort_req_primitive, unsigned int link_num,
                     const SCCPAddress& src_addr, const SCCPAddress& dst_addr);
protected:
  void noticeTCUser(TDialogueServiceData* t_dlg_svc_data,
                    const TDialogueId& t_dialogue_id,
                    TC_Notice_Ind::ReportCause_e r_cause);

  void activateTimers(TDialogueServiceData* t_dlg_svc_data,
                      const ROSComponentsList * comp_list);

  void analyzeFailureCauseAndNotifyTCUser(TDlgRequestSenderAC::SerializationResult_e res_status,
                                          const TDialogueId& t_dialogue_id,
                                          TDialogueServiceData* t_dlg_svc_data,
                                          bool return_on_error);

  void rejectComponent(TDialogueServiceData* t_dlg_svc_data,
                       const TDialogueId& t_dialogue_id,
                       TC_L_Reject_Ind::problem_code_e problem_code);

private:
  sua::libsua::SuaApi* _suaApi;
};

}}}

#endif
