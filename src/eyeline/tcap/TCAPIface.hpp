#ifndef __EYELINE_TCAP_TCAPIFACE_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_TCAPIFACE_HPP__

# include <sys/time.h>

# include "eyeline/tcap/TDialogueId.hpp"
# include "eyeline/tcap/TDlgHandlerIface.hpp"
# include "eyeline/tcap/TDialogueRequestPrimitives.hpp"

namespace eyeline {
namespace tcap {

class TCAPIface {
public:
  virtual ~TCAPIface() {}

  virtual TDialogueId openDialogue(TDlgHandlerIface* tIndicationHandlers,
                                   unsigned int dialogueTimeout) = 0;

  virtual void updateDialogue(TC_Begin_Req* requestPrimitive) = 0;
  virtual void updateDialogue(TC_Cont_Req* requestPrimitive) = 0;
  virtual void updateDialogue(TC_End_Req* requestPrimitive) = 0;
  virtual void updateDialogue(TC_UAbort_Req* requestPrimitive) = 0;

  virtual void closeDialogue(const TDialogueId& tDialogueId) = 0;
};

}}

#endif

