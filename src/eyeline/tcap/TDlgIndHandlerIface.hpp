/* ************************************************************************** *
 * TCAP API: structured TCAP dialogue indications handler interface.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_TDLG_IND_HANDLER_IFACE_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif /* __GNUC__ */
#define __EYELINE_TCAP_TDLG_IND_HANDLER_IFACE_HPP

#include "eyeline/tcap/TDlgIndicationPrimitives.hpp"
#include "eyeline/tcap/TComponentIndicationPrimitives.hpp"

namespace eyeline {
namespace tcap {

class TDlgIndHandlerIface {
protected:
  virtual ~TDlgIndHandlerIface() { }

public:
  // -- Dialogue(Component contained) handling indication primitives
  virtual void updateDialogue(TR_Begin_Ind & tc_ind) /*throw(std::exception)*/ = 0;
  virtual void updateDialogue(TR_Cont_Ind & tc_ind) /*throw(std::exception)*/ = 0;
  virtual void updateDialogue(TR_End_Ind & tc_ind) /*throw(std::exception)*/ = 0;
  virtual void updateDialogue(TR_UAbort_Ind & tc_ind) /*throw(std::exception)*/ = 0;
  // --
  virtual void updateDialogue(TR_PAbort_Ind & tc_ind) /*throw(std::exception)*/ = 0;
  virtual void updateDialogue(TR_Notice_Ind & tc_ind) /*throw(std::exception)*/ = 0;

  // -- Component only handling indication primitives
  virtual void updateDialogue(TC_L_Cancel_Ind & tc_ind) /*throw(std::exception)*/ = 0;
  virtual void updateDialogue(TC_L_Reject_Ind & tc_ind) /*throw(std::exception)*/ = 0;
};

} //tcap
} //eyeline

#endif /* __EYELINE_TCAP_TDLG_IND_HANDLER_IFACE_HPP */

