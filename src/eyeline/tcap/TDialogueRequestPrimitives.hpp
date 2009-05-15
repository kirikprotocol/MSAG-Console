/* ************************************************************************** *
 * Classes implementing structured TCAP dialogue requests.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_PROVD_TDLGREQUESTPRIMITIVES_HPP__
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROVD_TDLGREQUESTPRIMITIVES_HPP__

#include "eyeline/asn1/EncodedOID.hpp"
#include "eyeline/tcap/TDialogueHandlingPrimitive.hpp"
#include "eyeline/tcap/proto/TCAssociateDiagnostic.hpp"

namespace eyeline {
namespace tcap {

using eyeline::asn1::EncodedOID;

class TDialogueRequestPrimitive : public TDialogueHandlingPrimitive {
protected:
  TCAPMessage _msgTC;

public:
  TDialogueRequestPrimitive(TCAPMessage::TKind_e use_tkind = TCAPMessage::t_none)
    : TDialogueHandlingPrimitive(_msgTC), _msgTC(use_tkind)
  { }
  virtual ~TDialogueRequestPrimitive()
  { }

  void setAppCtx(const EncodedOID & use_acid)
  { /*TODO: _msgTC.setAppCtx() */ }

  //These methods only for 1st response to T_Begin_Ind
  void acceptDlgByUser(void)
  { /*TODO: */ }
  void rejectDlgByUser(proto::AssociateSourceDiagnostic::DlgSrvUser_e use_cause =
                       proto::AssociateSourceDiagnostic::dsu_null)
  { /*TODO: */  }

  void setReturnOnError();
  bool getReturnOnError() const;

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

  typedef enum { PREARRANGED_END, BASIC_END } end_transaction_facility_t;

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

#endif /* __EYELINE_TCAP_PROVD_TDLGREQUESTPRIMITIVES_HPP__ */

