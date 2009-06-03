/*
 * TCompIndComposers.hpp
 *
 *  Created on: Jun 1, 2009
 *      Author: shulga
 */

#ifndef __EYELINE_TCAP_PROVD_TCOMPINDCOMPOSERS_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROVD_TCOMPINDCOMPOSERS_HPP__

# include "eyeline/tcap/TComponentIndicationPrimitives.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

template <class T_COMP_IND_Arg /* pubic: TComponentIndicationPrimitive */>
class TCompIndicationComposerT : public T_COMP_IND_Arg {
public:
  void setDialogueId(TDialogueId dlgId) { _dlgId = dlgId; _isSetDlgId = true; }

  void setInvokeId(InvokeId invokeId) { _invokeId = invokeId; _isSetInvokeId = true; }
};

class TC_L_Cancel_Ind_Composer : public TCompIndicationComposerT<TC_L_Cancel_Ind> {
};

class TC_L_Reject_Ind_Composer : public TCompIndicationComposerT<TC_L_Reject_Ind> {
public:
  void setProblemCode(problem_code_t problemCode) {
    _problemCode = problemCode; _isSetProblemCode = true;
  }
};

}}}

#endif
