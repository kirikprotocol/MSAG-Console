/* ************************************************************************** *
 * TComponent sublayer: component handling requests.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_TCOMPONENT_REQ_PRIMITIVES_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif /* __GNUC__ */
#define __EYELINE_TCAP_TCOMPONENT_REQ_PRIMITIVES_HPP

#include "eyeline/tcap/TDialogueDefs.hpp"
#include "eyeline/tcap/TComponentDefs.hpp"

namespace eyeline {
namespace tcap {

using eyeline::ros::RejectProblem;
using eyeline::ros::RejectProblem_t;

class TComponentRequestPrimitive {
public:
  enum RKind_e {
    reqNone = 0, reqTimerReset, reqUCancel
  };

  virtual ~TComponentRequestPrimitive()
  { }

  static const char * getIdent(RKind_e use_kind);
  //
  const char * getIdent(void) const { return getIdent(_rKind); }
  //

  TDialogueId getDialogueId(void) const { return _dlgId; }
  //
  InvokeId getInvokeId(void) const { return _invokeId; }


  void setDialogueId(TDialogueId use_dlg) { _dlgId = use_dlg; }
  void setInvokeId(InvokeId inv_id) { _invokeId = inv_id; }

protected:
  const RKind_e _rKind;
  TDialogueId   _dlgId;
  InvokeId      _invokeId;

  TComponentRequestPrimitive(RKind_e req_kind, TDialogueId use_dlg = 0, InvokeId use_inv = 0)
    : _rKind(req_kind), _dlgId(use_dlg), _invokeId(use_inv)
  { }
};

// ---------------------------------------------------
// -- Requests, which have only local meaning:
// ---------------------------------------------------

class TC_Timer_Reset_Req : public TComponentRequestPrimitive {
public:
  TC_Timer_Reset_Req(TDialogueId use_dlg = 0, InvokeId use_inv = 0)
    : TComponentRequestPrimitive(reqTimerReset, use_dlg, use_inv)
  { }
  ~TC_Timer_Reset_Req()
  { }
};


class TC_U_Cancel_Req : public TComponentRequestPrimitive {
public:
  TC_U_Cancel_Req(TDialogueId use_dlg = 0, InvokeId use_inv = 0)
    : TComponentRequestPrimitive(reqUCancel, use_dlg, use_inv)
  { }
  ~TC_U_Cancel_Req()
  { }
};

}}

#endif /* __EYELINE_TCAP_TCOMPONENT_REQ_PRIMITIVES_HPP */

