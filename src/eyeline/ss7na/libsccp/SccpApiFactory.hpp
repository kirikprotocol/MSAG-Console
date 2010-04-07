#ifndef __EYELINE_SS7NA_LIBSCCP_SCCPAPIFACTORY_HPP__
# define __EYELINE_SS7NA_LIBSCCP_SCCPAPIFACTORY_HPP__

# include "eyeline/ss7na/libsccp/SccpApi.hpp"

namespace eyeline {
namespace ss7na {
namespace libsccp {

class SccpApiFactory {
public:
  static void init();
  static SccpApi& getSccpApiIface();
private:
  static SccpApi* _instance;
};

}}}

#endif
