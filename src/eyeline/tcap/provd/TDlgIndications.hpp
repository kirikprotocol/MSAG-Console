/* ************************************************************************** *
 * Classes implementing structured TCAP dialogue indications.
 * ************************************************************************** */
#ifndef __ELC_TCAP_DIALOGUE_INDICATIONS_HPP
#ident "@(#)$Id$"
#define __ELC_TCAP_DIALOGUE_INDICATIONS_HPP

#include "eyeline/tcap/provd/TDlgHandling.hpp"
#include "eyeline/tcap/provd/SUAIndications.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

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

  void bindSUAInd(const SUAUnitdataInd & sua_ind)
  {
    //TODO:
  }
  //TODO: getters
};
//
class TC_Cont_Ind : public TDialogueIndicationPrimitive {
public:
  TC_Cont_Ind(TCAPMessage & use_tmsg)
    : TDialogueIndicationPrimitive(use_tmsg)
  { }
  
  void bindSUAInd(const SUAUnitdataInd & sua_ind)
  {
    //TODO:
  }
  //TODO: getters
};
//
class TC_End_Ind : public TDialogueIndicationPrimitive {
public:
  TC_End_Ind(TCAPMessage & use_tmsg)
    : TDialogueIndicationPrimitive(use_tmsg)
  { }

  void bindSUAInd(const SUAUnitdataInd & sua_ind)
  {
    //TODO:
  }
  //TODO: getters
};
//
class TC_UAbort_Ind : public TDialogueIndicationPrimitive {
public:
  TC_UAbort_Ind(TCAPMessage & use_tmsg)
    : TDialogueIndicationPrimitive(use_tmsg)
  { }

  void bindSUAInd(const SUAUnitdataInd & sua_ind)
  {
    //TODO:
  }
  //TODO: getters for TCAbrtPDU or TCExternal
};
//
class TC_PAbort_Ind : public TDialogueIndicationPrimitive {
public:
  TC_PAbort_Ind(TCAPMessage & use_tmsg)
    : TDialogueIndicationPrimitive(use_tmsg)
  { }

  void bindSUAInd(const SUAUnitdataInd & sua_ind)
  {
    //TODO:
  }
  //TODO: getters for P_AbortCause
};

//
class TC_Notice_Ind : public TDialogueIndicationPrimitive {
public:
  TC_Notice_Ind(TCAPMessage & use_tmsg)
    : TDialogueIndicationPrimitive(use_tmsg)
  { }

  void bindSUAInd(const SUANoticeInd & sua_ind)
  {
    //TODO:
  }
  //TODO: getters for returnCause
};

class TDlgHandlerIface {
public:
  virtual bool updateDialogue(TC_Begin_Ind & use_tmsg) = 0;
  virtual bool updateDialogue(TC_Cont_Ind & use_tmsg) = 0;
  virtual bool updateDialogue(TC_End_Ind & use_tmsg) = 0;
  virtual bool updateDialogue(TC_PAbort_Ind & use_tmsg) = 0;
  virtual bool updateDialogue(TC_UAbort_Ind & use_tmsg) = 0;
  virtual bool updateDialogue(TC_Notice_Ind & use_tmsg) = 0;
};


} //provd
} //tcap
} //eyeline

#endif /* __ELC_TCAP_DIALOGUE_INDICATIONS_HPP */

