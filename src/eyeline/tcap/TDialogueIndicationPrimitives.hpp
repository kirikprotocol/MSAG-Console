/* ************************************************************************** *
 * Classes implementing structured TCAP dialogue indications.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_TDIALOGUEINDICATIONPRIMITIVE_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_TDIALOGUEINDICATIONPRIMITIVE_HPP__

# include "eyeline/tcap/TDialogueHandlingPrimitive.hpp"
# include "eyeline/sccp/SCCPAddress.hpp"

namespace eyeline {
namespace tcap {

using eyeline::sccp::SCCPAddress;

class TDialogueIndicationPrimitive : public TDialogueHandlingPrimitive {
public:
  TDialogueIndicationPrimitive();
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

  const SCCPAddress & getOrigAddress(void) const;
  const SCCPAddress & getDestAddress(void) const;
  //TODO: getters
};
//
class TC_Cont_Ind : public TDialogueIndicationPrimitive {
public:
  TC_Cont_Ind(TCAPMessage & use_tmsg)
    : TDialogueIndicationPrimitive(use_tmsg)
  { }

  // TODO: original and destination addresses must be filled from corresponding fields of CLDT message
  const SCCPAddress & getOrigAddress(void) const;
  const SCCPAddress & getDestAddress(void) const;
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
  TCAPMessage _fake_msg;

  TC_PAbort_Ind()
    : TDialogueIndicationPrimitive(_fake_msg)
  { }

  TC_PAbort_Ind(TCAPMessage & use_tmsg)
    : TDialogueIndicationPrimitive(use_tmsg)
  { }

  void setPAbortCause(PAbortCause_e cause);
  //TODO: getters for P_AbortCause
};

//
class TC_Notice_Ind : public TDialogueIndicationPrimitive {
public:
  TCAPMessage _fake_msg;

  TC_Notice_Ind()
    : TDialogueIndicationPrimitive(_fake_msg)
  { }

  TC_Notice_Ind(TCAPMessage & use_tmsg)
    : TDialogueIndicationPrimitive(use_tmsg)
  { }

  enum ReportCause_e { INVALID_TCUSER_DATA, SCCP_ERROR };

  void setReportCause(ReportCause_e cause);
  ReportCause_e getReportCause();
  //TODO: getters for returnCause
};

} //tcap
} //eyeline

#endif /* __EYELINE_TCAP_TDIALOGUEINDICATIONPRIMITIVE_HPP__ */

