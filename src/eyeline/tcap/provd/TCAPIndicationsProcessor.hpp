#ifndef __EYELINE_TCAP_PROVD_TCAPINDICATIONSPROCESSOR_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROVD_TCAPINDICATIONSPROCESSOR_HPP__

# include "logger/Logger.h"
# include "eyeline/sccp/SCCPAddress.hpp"
# include "eyeline/tcap/provd/TDlgIndComposers.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

class TCAPIndicationsProcessor {
public:
  TCAPIndicationsProcessor();
  virtual ~TCAPIndicationsProcessor() {}

  virtual bool updateDialogue(TBeginIndComposer & tc_begin_ind, unsigned int src_link_num);
  virtual bool updateDialogue(TContIndComposer & tc_cont_ind, unsigned int src_link_num);
  virtual bool updateDialogue(TEndIndComposer & tc_end_ind, unsigned int src_link_num);
  virtual bool updateDialogue(TPAbortIndComposer & tc_pAbort_ind, unsigned int src_link_num);
  virtual bool updateDialogue(TUAbortIndComposer & tc_uAbort_ind, unsigned int src_link_num);
  virtual bool updateDialogue(TNoticeIndComposer & tc_notice_ind, unsigned int src_link_num);

private:
  smsc::logger::Logger* _logger;
  sccp::SCCPAddress _ownAddress;
};

}}}

#endif /* __EYELINE_TCAP_PROVD_TCAPINDICATIONSPROCESSOR_HPP__ */

