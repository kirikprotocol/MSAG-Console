#ifndef __EYELINE_TCAP_TDLGHANDLERIFACE_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_TDLGHANDLERIFACE_HPP__

# include "eyeline/tcap/TDlgIndications.hpp"

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
};

}}

#endif
