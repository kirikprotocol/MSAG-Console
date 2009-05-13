#ifndef __EYELINE_TCAP_PROVD_TCAPINDICATIONSPROCESSOR_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROVD_TCAPINDICATIONSPROCESSOR_HPP__

# include "eyeline/tcap/TDialogueHandlingPrimitive.hpp"
# include "eyeline/tcap/proto/TransactionId.hpp"
# include "eyeline/tcap/provd/TDlgIndDispatcher.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

class TCAPIndicationsProcessor/* : public TDlgManagerIface*/ {
public:
  virtual ~TCAPIndicationsProcessor() {}
  virtual bool updateDialogue(TC_Begin_Ind& tcBeginInd, unsigned int srcLinkNum);
  virtual bool updateDialogue(TC_Cont_Ind& tcContInd, unsigned int srcLinkNum);
  virtual bool updateDialogue(TC_End_Ind& tcEndInd, unsigned int srcLinkNum);
  virtual bool updateDialogue(TC_PAbort_Ind& tcPAbortInd, unsigned int srcLinkNum);
  virtual bool updateDialogue(TC_UAbort_Ind& tcUAbortInd, unsigned int srcLinkNum);
  //  virtual bool updateDialogue(TC_Notice_Ind& tcNoticeInd);
  virtual bool updateDialogue(TC_L_Cancel_Ind& lCancelInd, unsigned int srcLinkNum);
  virtual bool updateDialogue(TC_L_Reject_Ind& lRejectInd, unsigned int srcLinkNum);
protected:
  void formPAbortRequest(const proto::TransactionId& trnId,
                         TDialogueHandlingPrimitive::PAbortCause_e pAbortCause);
};

}}}

#endif
