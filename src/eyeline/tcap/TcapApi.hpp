/* ************************************************************************** *
 * TCAP API: structured TCAP dialogue API.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_TDLG_API_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_TDLG_API_HPP

#include "eyeline/tcap/TDlgHandlerIface.hpp"
#include "eyeline/tcap/TDlgRequestPrimitives.hpp"
#include "eyeline/tcap/TComponentRequestPrimitives.hpp"

namespace eyeline {
namespace tcap {

struct TDlgHandle {
  TDialogueId         dlgId;  //unique dialogue Id.
  TDlgHandlerIface *  indHdl; //dialogue indications handler

  TDlgHandle() : dlgId(0), indHdl(0)
  { }
  TDlgHandle(TDialogueId use_id, TDlgHandlerIface * use_hdl)
    : dlgId(use_id), indHdl(use_hdl)
  { }

  bool empty(void) const { return (!indHdl && !dlgId); }
};

class TcapApi {
protected:
  virtual ~TcapApi() {}

public:
  enum ErrorCode_e {
      dlgOk = 0
    , errSCCP               //dialogue primitive processing failed due to SCCP error
    , errTCAP               //dialogue primitive processing failed due to unexpected TCAP error
    , errSrcAddress         //illegal source SCCP address
    , errDstAddress         //illegal destination SCCP address
    , errDialogueId         //unknown/illegal dialogueId (transaction Id)
    , errDialoguePortion    //dialogue UserInfo serialization error or
                            //UserInfo data is too large
    , errComponentPortion   //Components serialization error,
    , errTooMuchComponents  //Too much components in T_Begin|T_End
    , errInvokeId           //unknown/illegal invoke id
  };

  //Returns string representation of ErrorCode_e value
  static const char * nmErrorCode(ErrorCode_e use_val);

  //Creates TCAP dialogue maintaining structures and dialogue indications
  //handler associated with given Appication Context.
  //Returns empty dialogue handle in case of application context with
  //given OID isn't registered by TCProvider.
  //Throws if TCAP dialog cann't be created due to unexpected reason.
  //
  //NOTE.1: 'init_opcode' is mandatory only in case of _ac_contextless_ops
  // 
  //NOTE.2: 'Dialogue expiration timeout' shouldn't be less than
  //        Remote Operations timeout implied by application context used.
  // 
  //NOTE.3: If 'Dialogue expiration timeout' isn't set, the default value
  //        specified in TCProvider configuration is used.
  virtual TDlgHandle openDialogue(uint8_t ssn_id, const asn1::EncodedOID & ac_oid,
                                   ros::LocalOpCode init_opcode = 0,
                                   TDlgTimeout dlg_exp_tmo = 0) /*throw(std::exception)*/= 0;
  //Returns false if 'dlg_id' is unknown
  virtual bool closeDialogue(TDialogueId dlg_id) /*throw()*/= 0;

  // -- Dialogue(Component contained) handling primitives
  virtual ErrorCode_e updateDialogue(const TC_Begin_Req & tc_req) /*throw()*/= 0;
  virtual ErrorCode_e updateDialogue(const TC_Cont_Req & tc_req) /*throw()*/= 0;
  virtual ErrorCode_e updateDialogue(const TC_End_Req & tc_req) /*throw()*/= 0;
  virtual ErrorCode_e updateDialogue(const TC_UAbort_Req & tc_req) /*throw()*/= 0;

  // -- Component only handling primitives
  virtual ErrorCode_e updateDialogue(const TC_U_Reject_Req & tc_req) /*throw()*/= 0;
  virtual ErrorCode_e updateDialogue(const TC_Timer_Reset_Req & tc_req) /*throw()*/= 0;
};

}}

#endif /* __EYELINE_TCAP_TDLG_API_HPP */

