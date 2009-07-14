#ifndef __EYELINE_TCAP_TCOMPONENTINDICATIONPRIMITIVES_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_TCOMPONENTINDICATIONPRIMITIVES_HPP__

# include "eyeline/utilx/Exception.hpp"
# include "eyeline/tcap/TDialogueId.hpp"
# include "eyeline/tcap/TComponentDefs.hpp"

namespace eyeline {
namespace tcap {

using eyeline::ros::RejectProblem;

class TComponentIndicationPrimitive {
public:
  TComponentIndicationPrimitive()
    : _isSetDlgId(false), _isSetInvokeId(false) {}

  TDialogueId getDialogueId(void) const {
    if ( _isSetDlgId )
      return _dlgId;

    throw utilx::FieldNotSetException("TComponentIndicationPrimitive::getDialogueId::: dialogueId is not set");
  }

  InvokeId getInvokeId(void) const {
    if ( _isSetInvokeId )
      return _invokeId;

    throw utilx::FieldNotSetException("TComponentIndicationPrimitive::getInvokeId::: invokeId is not set");
  }

protected:
  TDialogueId _dlgId;
  InvokeId _invokeId;
  bool _isSetDlgId, _isSetInvokeId;
};


class TC_L_Cancel_Ind : public TComponentIndicationPrimitive {
};

class TC_L_Reject_Ind : public TComponentIndicationPrimitive {
protected:
  RejectProblem   _rejProblem;

public:
  TC_L_Reject_Ind()
  { }

  const RejectProblem & getProblem(void) const { return _rejProblem; }
};

}}

#endif /* __EYELINE_TCAP_TCOMPONENTINDICATIONPRIMITIVES_HPP__ */

