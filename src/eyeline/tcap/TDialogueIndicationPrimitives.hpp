/* ************************************************************************** *
 * Classes implementing structured TCAP dialogue indications.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_TDIALOGUEINDICATIONPRIMITIVE_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_TDIALOGUEINDICATIONPRIMITIVE_HPP__

# include "eyeline/tcap/TDialogueHandlingPrimitive.hpp"

//   bindSUAInd was removed from TC_*_Ind classes
//   void bindSUAInd(const SUANoticeInd & sua_ind)
//   {
//     //TODO:
//   }

namespace eyeline {
namespace tcap {

class TDialogueIndicationPrimitive : public TDialogueHandlingPrimitive {
public:
  TDialogueIndicationPrimitive(TCAPMessage & use_tmsg)
    : TDialogueHandlingPrimitive(use_tmsg)
  { }
  virtual ~TDialogueIndicationPrimitive()
  { }
  //TODO: indications specific stuff
};

//
class TC_Begin_Ind : public TDialogueIndicationPrimitive {
public:
  TC_Begin_Ind(TCAPMessage & use_tmsg)
    : TDialogueIndicationPrimitive(use_tmsg)
  { }

  //TODO: getters
};
//
class TC_Cont_Ind : public TDialogueIndicationPrimitive {
public:
  TC_Cont_Ind(TCAPMessage & use_tmsg)
    : TDialogueIndicationPrimitive(use_tmsg)
  { }
  
  //TODO: getters
};
//
class TC_End_Ind : public TDialogueIndicationPrimitive {
public:
  TC_End_Ind(TCAPMessage & use_tmsg)
    : TDialogueIndicationPrimitive(use_tmsg)
  { }

  //TODO: getters
};
//
class TC_UAbort_Ind : public TDialogueIndicationPrimitive {
public:
  TC_UAbort_Ind(TCAPMessage & use_tmsg)
    : TDialogueIndicationPrimitive(use_tmsg)
  { }

  //TODO: getters for TCAbrtPDU or TCExternal
};
//
class TC_PAbort_Ind : public TDialogueIndicationPrimitive {
public:
  TC_PAbort_Ind(TCAPMessage & use_tmsg)
    : TDialogueIndicationPrimitive(use_tmsg)
  { }

  //TODO: getters for P_AbortCause
};

//
class TC_Notice_Ind : public TDialogueIndicationPrimitive {
public:
  TC_Notice_Ind(TCAPMessage & use_tmsg)
    : TDialogueIndicationPrimitive(use_tmsg)
  { }

  //TODO: getters for returnCause
};

} //tcap
} //eyeline

#endif
