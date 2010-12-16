#ifndef __SCAG_TRANSPORT_SMPP_SMPPROUTER2_H__
#define __SCAG_TRANSPORT_SMPP_SMPPROUTER2_H__

#include "sms/sms.h"
#include "router/route_types.h"
#include "SmppEntity2.h"

namespace scag2 {
namespace transport {
namespace smpp {

    namespace router = scag::transport::smpp::router;

class SmppRouter {
public:
    virtual SmppEntity* RouteSms( router::SmeIndex srcidx,
                                  const smsc::sms::Address& source,
                                  const smsc::sms::Address& dest,
                                  router::RouteInfo& info,
                                  std::vector< std::string >* traceit )=0;
    virtual SmppEntity* getSmppEntity(const char* sysId, bool* isEnabled = 0) const=0;
};

}//smpp
}//transport
}//scag


#endif
