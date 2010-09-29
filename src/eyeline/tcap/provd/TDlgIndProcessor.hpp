/* ************************************************************************** *
 * TCAP dialogue indications processor interface definition.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_PROVD_TDLG_INDICATIONS_PROCESSOR_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROVD_TDLG_INDICATIONS_PROCESSOR_HPP

#include "eyeline/tcap/provd/TDlgIndComposers.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

//Locates required TDialogueFSM and updates it with indication.
class TDlgIndProcessorIface {
protected:
  virtual ~TDlgIndProcessorIface();

public:
  virtual void updateDlgByIndication(TBeginIndComposer & tc_begin_ind)
    /* throw(std::exception) */ = 0;
  virtual void updateDlgByIndication(TContIndComposer & tc_cont_ind)
    /* throw(std::exception) */ = 0;
  virtual void updateDlgByIndication(TEndIndComposer & tc_end_ind)
    /* throw(std::exception) */ = 0;
  virtual void updateDlgByIndication(TUAbortIndComposer & tc_uAbort_ind)
    /* throw(std::exception) */ = 0;
  virtual void updateDlgByIndication(TPAbortIndComposer & tc_pAbort_ind)
    /* throw(std::exception) */ = 0;
  virtual void updateDlgByIndication(TNoticeIndComposer & tc_notice_ind)
    /* throw(std::exception) */ = 0;
};

}}}

#endif /* __EYELINE_TCAP_PROVD_TDLG_INDICATIONS_PROCESSOR_HPP */

