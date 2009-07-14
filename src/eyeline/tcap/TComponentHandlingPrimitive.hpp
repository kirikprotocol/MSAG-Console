#ifndef __EYELINE_TCAP_TCOMPONENTHANDLINGPRIMITIVE_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_TCOMPONENTHANDLINGPRIMITIVE_HPP__

# include <sys/types.h>

# include "eyeline/utilx/Exception.hpp"
# include "eyeline/tcap/TDialogueId.hpp"
# include "eyeline/tcap/TComponentDefs.hpp"

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

    throw utilx::FieldNotSetException("TComponentHandlingPrimitive::getDialogueId::: dialogueId is not set");
  }

  void setInvokeId(InvokeId invokeId) { _invokeId = invokeId; _isSetInvokeId = true; }
  InvokeId getInvokeId(void) const {
    if ( _isSetInvokeId )
      return _invokeId;

    throw utilx::FieldNotSetException("TComponentHandlingPrimitive::getInvokeId::: invokeId is not set");
  }

private:
  TDialogueId _dlgId;
  InvokeId _invokeId;
  bool _isSetDlgId, _isSetInvokeId;
};

}}

#endif
