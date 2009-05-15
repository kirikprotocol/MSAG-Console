#ifndef __EYELINE_TCAP_TCOMPONENTHANDLINGPRIMITIVE_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_TCOMPONENTHANDLINGPRIMITIVE_HPP__

# include "eyeline/utilx/Exception.hpp"

namespace eyeline {
namespace tcap {

class TComponentHandlingPrimitive {
public:
  TComponentHandlingPrimitive()
    : _isSetDlgId(false), _isSetInvokeId(false) {}

  void setDialogueId(TDialogueId dlgId) { _dlgId = dlgId; _isSetDlgId = true; }
  TDialogueId getDialogueId(void) const {
    if ( _isSetDlgId )
      return _dlgId;
    else
      throw utilx::FieldNotSetException("TComponentHandlingPrimitive::getDialogueId::: dialogueId is not set");
  }

  typedef uint8_t InvokeId;

  void setInvokeId(InvokeId invokeId) { _invokeId = invokeId; _isSetInvokeId = true; }
  InvokeId getInvokeId(void) const {
    if ( _isSetInvokeId )
      return _invokeId;
    else
      throw utilx::FieldNotSetException("TComponentHandlingPrimitive::getInvokeId::: invokeId is not set");
  }

private:
  TDialogueId _dlgId;
  InvokeId _invokeId;
  bool _isSetDlgId, _isSetInvokeId;
};

}}

#endif
