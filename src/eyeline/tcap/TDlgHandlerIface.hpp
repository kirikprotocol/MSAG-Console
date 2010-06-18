/* ************************************************************************** *
 * TCAP API: TCAP dialogue indications handler interface.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_TDLGHANDLERIFACE_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_TDLGHANDLERIFACE_HPP__

# include "eyeline/tcap/TDlgIndicationPrimitives.hpp"
# include "eyeline/tcap/TComponentIndicationPrimitives.hpp"

namespace eyeline {
namespace tcap {

class TDlgHandlerIface {
protected:
  virtual ~TDlgHandlerIface() { }

public:
  // -- Dialogue(Component contained) handling primitives
  virtual void updateDialogue(TC_Begin_Ind & tc_ind) = 0;
  virtual void updateDialogue(TC_Cont_Ind & tc_ind) = 0;
  virtual void updateDialogue(TC_End_Ind & tc_ind) = 0;
  virtual void updateDialogue(TC_PAbort_Ind & tc_ind) = 0;
  virtual void updateDialogue(TC_UAbort_Ind & tc_ind) = 0;
  virtual void updateDialogue(TC_Notice_Ind & tc_ind) = 0;

  // -- Component only handling primitives
  virtual void updateDialogue(TC_L_Cancel_Ind & tc_ind) = 0;
  virtual void updateDialogue(TC_L_Reject_Ind & tc_ind) = 0;
  virtual void updateDialogue(TC_R_Reject_Ind & tc_ind) = 0;
  virtual void updateDialogue(TC_U_Reject_Ind & tc_ind) = 0;
};

} //tcap
} //eyeline

#endif /* __EYELINE_TCAP_TDLGHANDLERIFACE_HPP__ */

