#ifndef __EYELINE_TCAP_TCOMPONENTINDICATIONPRIMITIVES_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_TCOMPONENTINDICATIONPRIMITIVES_HPP__

# include "eyeline/utilx/Exception.hpp"
# include "eyeline/tcap/TDialogueId.hpp"
# include "eyeline/tcap/TComponentDefs.hpp"

namespace eyeline {
namespace tcap {

class TComponentIndicationPrimitive {
public:
  TComponentIndicationPrimitive()
    : _isSetDlgId(false), _isSetInvokeId(false) {}

  TDialogueId getDialogueId(void) const {
    if ( _isSetDlgId )
      return _dlgId;
    else
      throw utilx::FieldNotSetException("TComponentIndicationPrimitive::getDialogueId::: dialogueId is not set");
  }

  InvokeId getInvokeId(void) const {
    if ( _isSetInvokeId )
      return _invokeId;
    else
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
public:
  TC_L_Reject_Ind()
    : _isSetProblemCode(false) {}

  problem_code_t getProblemCode() const {
    if ( _isSetProblemCode )
      return _problemCode;
    else
      throw utilx::FieldNotSetException("TC_L_Reject_Ind::getProblemCode::: problemCode is not set");
  }

protected:
  problem_code_t _problemCode;
  bool _isSetProblemCode;
};

}}

#endif
