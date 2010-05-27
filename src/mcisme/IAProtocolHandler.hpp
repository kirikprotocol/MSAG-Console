#ifndef __SMSC_MCISME_IAPROTOCOLHANDLER_HPP__
# define __SMSC_MCISME_IAPROTOCOLHANDLER_HPP__

# include "mcisme/mcaia/BusyResponse.hpp"

namespace smsc {
namespace mcisme {

class IAProtocolHandler {
public:
  virtual ~IAProtocolHandler() {}

  virtual void handle(const mcaia::BusyResponse& msg) = 0;
};

}}

#endif
