/* ************************************************************************** *
 * TC Provider: interface of factory of TCAP dialogue handlers.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_TDLGHANDLERIFACEFACTORY_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif /* __GNUC__ */
#define __EYELINE_TCAP_TDLGHANDLERIFACEFACTORY_HPP__

#include "eyeline/tcap/TDlgIndHandlerIface.hpp"

namespace eyeline {
namespace tcap {

class TDlgHandlerFactoryIface { //bound with specific ApplicationContext
protected:
  virtual ~TDlgHandlerFactoryIface() { }

public:
  //NOTE: init_opcode is mandatory only in case of _ac_contextless_ops !!!
  virtual TDlgIndHandlerIface* createTDlgHandler(ros::LocalOpCode init_opcode = 0) = 0;
  virtual void releaseTDlgHandler(TDlgIndHandlerIface * dlg_hdl) = 0;
};

}}

#endif /* __EYELINE_TCAP_TDLGHANDLERIFACEFACTORY_HPP__ */

