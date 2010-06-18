/* ************************************************************************** *
 * TComponent sublayer: component handling requests.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_TCOMPONENT_REQ_PRIMITIVES_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_TCOMPONENT_REQ_PRIMITIVES_HPP

#include "eyeline/tcap/TDialogueDefs.hpp"
#include "eyeline/tcap/TComponentDefs.hpp"

namespace eyeline {
namespace tcap {

using eyeline::ros::RejectProblem;
using eyeline::ros::RejectProblem_t;

class TComponentRequestPrimitive {
protected:
  TDialogueId   _dlgId;
  InvokeId      _invokeId;

public:
  TComponentRequestPrimitive(TDialogueId use_dlg = 0, InvokeId use_inv = 0)
    : _dlgId(use_dlg), _invokeId(use_inv)
  { }
  ~TComponentRequestPrimitive()
  { }
  //
  TDialogueId getDialogueId(void) const { return _dlgId; }
  //
  InvokeId getInvokeId(void) const { return _invokeId; }


  void setDialogueId(TDialogueId use_dlg) { _dlgId = use_dlg; }
  void setInvokeId(InvokeId inv_id) { _invokeId = inv_id; }
};


class TC_U_Reject_Req : public TComponentRequestPrimitive {
protected:
  RejectProblem   _rejProblem;

public:
  TC_U_Reject_Req(TDialogueId use_dlg = 0, InvokeId use_inv = 0)
    : TComponentRequestPrimitive(use_dlg, use_inv)
  { }
  ~TC_U_Reject_Req()
  { }

  const RejectProblem & getProblem(void) const { return _rejProblem; }

  void setProblem(RejectProblem::ProblemKind_e use_kind, RejectProblem_t use_val)
  {
    _rejProblem.setProblem(use_kind, use_val);
  }
};


class TC_Timer_Reset_Req : public TComponentRequestPrimitive {
public:
  TC_Timer_Reset_Req(TDialogueId use_dlg = 0, InvokeId use_inv = 0)
    : TComponentRequestPrimitive(use_dlg, use_inv)
  { }
  ~TC_Timer_Reset_Req()
  { }
};

}}

#endif /* __EYELINE_TCAP_TCOMPONENT_REQ_PRIMITIVES_HPP */

