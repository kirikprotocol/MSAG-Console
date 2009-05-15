#ifndef __EYELINE_TCAP_TDLGHANDLERIFACE_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_TDLGHANDLERIFACE_HPP__

# include "eyeline/tcap/TDialogueIndicationPrimitives.hpp"
# include "eyeline/tcap/TComponentIndicationPrimitives.hpp"

namespace eyeline {
namespace tcap {

class TDlgHandlerIface {
public:
  virtual bool updateDialogue(TC_Begin_Ind & use_tmsg) = 0;
  virtual bool updateDialogue(TC_Cont_Ind & use_tmsg) = 0;
  virtual bool updateDialogue(TC_End_Ind & use_tmsg) = 0;
  virtual bool updateDialogue(TC_PAbort_Ind & use_tmsg) = 0;
  virtual bool updateDialogue(TC_UAbort_Ind & use_tmsg) = 0;
  virtual bool updateDialogue(TC_Notice_Ind & use_tmsg) = 0;

  virtual bool updateDialogue(TC_L_Cancel_Ind & indPrimitive) = 0;
  virtual bool updateDialogue(TC_L_Reject_Ind & indPrimitive) = 0;

};

} //tcap
} //eyeline

#endif /* __EYELINE_TCAP_TDLGHANDLERIFACE_HPP__ */

