/* ************************************************************************** *
 * TCAP API: interface of factory of TCAP dialogue handlers.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_TDLGHANDLERIFACEFACTORY_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_TDLGHANDLERIFACEFACTORY_HPP__

# include "eyeline/tcap/TDlgHandlerIface.hpp"

namespace eyeline {
namespace tcap {

class TDlgHandlerIfaceFactory { //bound with specific ApplicationContext
protected:
  virtual ~TDlgHandlerIfaceFactory() { }

public:
  //NOTE: init_opcode is mandatory only in case of _ac_contextless_ops !!!
  virtual TDlgHandlerIface* createTDlgHandlerIface(ros::LocalOpCode init_opcode = 0) = 0;
  virtual void releaseTDlgHandlerIface(TDlgHandlerIface* dlg_hdl) = 0;
};

}}

#endif /* __EYELINE_TCAP_TDLGHANDLERIFACEFACTORY_HPP__ */

