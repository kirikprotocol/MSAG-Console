/* ************************************************************************** *
 * Classes implementing structured TCAP dialogue requests.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_PROVD_TDLGREQUESTPRIMITIVES_HPP__
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROVD_TDLGREQUESTPRIMITIVES_HPP__

#include "eyeline/tcap/TDialogueHandlingPrimitive.hpp"

namespace eyeline {
namespace tcap {

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

  end_transaction_facility_t getTermination() const;
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

} //tcap
} //eyeline

#endif

