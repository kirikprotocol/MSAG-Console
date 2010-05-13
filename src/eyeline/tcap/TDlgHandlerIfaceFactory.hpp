#ifndef __EYELINE_TCAP_TDLGHANDLERIFACEFACTORY_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_TDLGHANDLERIFACEFACTORY_HPP__

# include <eyeline/tcap/TDlgHandlerIface.hpp>

namespace eyeline {
namespace tcap {

class TDlgHandlerIfaceFactory {
public:
  virtual TDlgHandlerIface* createTDlgHandlerIface() = 0;
  virtual void releaseHandler(TDlgHandlerIface* handler) = 0;
};

}}

#endif
