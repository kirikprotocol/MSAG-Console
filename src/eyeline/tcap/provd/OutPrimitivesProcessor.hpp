#ifndef __EYELINE_TCAP_PROVD_OUTPRIMITIVESPROCESSOR_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROVD_OUTPRIMITIVESPROCESSOR_HPP__

# include "eyeline/sua/libsua/SuaUser.hpp"
# include "eyeline/tcap/TDialogueId.hpp"
# include "eyeline/tcap/TDialogueRequestPrimitives.hpp"
# include "eyeline/tcap/provd/TDlgReqSerializer.hpp"
# include "eyeline/tcap/provd/TDialogueServiceDataRegistry.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

class OutPrimitivesProcessor {
public:
  explicit OutPrimitivesProcessor(sua::libsua::SuaApi* suaApi)
    : _suaApi(suaApi)
  {}

  void updateDialogue(TC_Begin_Req* beginReqPrimitive);
  void updateDialogue(TC_Cont_Req* contReqPrimitive);
  void updateDialogue(TC_End_Req* endReqPrimitive);
  void updateDialogue(TC_UAbort_Req* uAbortReqPrimitive);
  void updateDialogue(TC_PAbort_Req* pAbortReqPrimitive);

  void sendPrimitive(TC_PAbort_Req* pAbortReqPrimitive, unsigned int linkNum);
protected:
  void noticeTCUser(TDialogueServiceDataRegistry::registry_element_ref_t& tDlgSvcData,
                    const TDialogueId& tDialogueId,
                    TReqSendResult::ResultCode_e resultCode);

  void activateTimers(TDialogueServiceDataRegistry::registry_element_ref_t& tDlgSvcData,
                      const ROSComponentsList& compList);
private:
  sua::libsua::SuaApi* _suaApi;
};

}}}

#endif
