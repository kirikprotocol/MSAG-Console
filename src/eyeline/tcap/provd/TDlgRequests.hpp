/* ************************************************************************** *
 * Classes implementing structured TCAP dialogue requests.
 * ************************************************************************** */
#ifndef __ELC_TCAP_DIALOGUE_REQUESTS_HPP
#ident "@(#)$Id$"
#define __ELC_TCAP_DIALOGUE_REQUESTS_HPP

#include "eyeline/tcap/provd/TDlgHandling.hpp"
#include "eyeline/tcap/provd/SUARequests.hpp"

namespace eyeline {
namespace tcap {
namespace provd {


class TDialogueRequestPrimitive : public TDialogueHandlingPrimitive {
protected:
  TCAPMessage _msgTC;

public:
  TDialogueRequestPrimitive(TCAPMessage::TKind_e use_tkind = TCAPMessage::t_none)
    : TDialogueHandlingPrimitive(_msgTC), _msgTC(use_tkind)
  { }
  virtual ~TDialogueRequestPrimitive()
  { }

  virtual bool transform(SUAUnitdataReq & use_udt) const = 0;
  //TODO: request specific stuff
};

//
class TC_Begin_Req : public TDialogueRequestPrimitive {
public:
  TC_Begin_Req()
    : TDialogueRequestPrimitive(TCAPMessage::t_begin)
  { }
  //TODO: setters
};
//
class TC_Cont_Req : public TDialogueRequestPrimitive {
public:
  TC_Cont_Req()
    : TDialogueRequestPrimitive(TCAPMessage::t_continue)
  { }
  //TODO: setters
};
//
class TC_End_Req : public TDialogueRequestPrimitive {
public:
  TC_End_Req()
    : TDialogueRequestPrimitive(TCAPMessage::t_end)
  { }
  //TODO: setters
};
//
class TC_UAbort_Req : public TDialogueRequestPrimitive {
public:
  TC_UAbort_Req()
    : TDialogueRequestPrimitive(TCAPMessage::t_abort)
  { }

  //TODO: setters for TCAbrtPDU or TCExternal
};

} //provd
} //tcap
} //eyeline

#endif /* __ELC_TCAP_DIALOGUE_REQUESTS_HPP */

