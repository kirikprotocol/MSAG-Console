#ifndef __EYELINE_TCAP_PROVD_TCAPINDICATIONSPROCESSOR_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROVD_TCAPINDICATIONSPROCESSOR_HPP__

# include "eyeline/sccp/SCCPAddress.hpp"
# include "eyeline/tcap/TDialogueHandlingPrimitive.hpp"
# include "eyeline/tcap/TDialogueIndicationPrimitives.hpp"
# include "eyeline/tcap/proto/TransactionId.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

class TCAPIndicationsProcessor {
public:
  TCAPIndicationsProcessor();
  virtual ~TCAPIndicationsProcessor() {}
  virtual bool updateDialogue(TC_Begin_Ind& tcBeginInd, unsigned int srcLinkNum);
  virtual bool updateDialogue(TC_Cont_Ind& tcContInd, unsigned int srcLinkNum);
  virtual bool updateDialogue(TC_End_Ind& tcEndInd, unsigned int srcLinkNum);
  virtual bool updateDialogue(TC_PAbort_Ind& tcPAbortInd, unsigned int srcLinkNum);
  virtual bool updateDialogue(TC_UAbort_Ind& tcUAbortInd, unsigned int srcLinkNum);

  virtual bool updateDialogue(TC_Notice_Ind& tcNoticeInd, unsigned int srcLinkNum);
private:
  sccp::SCCPAddress _ownAddress;
};

}}}

#endif
