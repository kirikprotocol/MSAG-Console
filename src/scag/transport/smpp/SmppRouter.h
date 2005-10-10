#ifndef __SCAG_TRANSPORT_SMPP_SMPPROUTER_H__
#define __SCAG_TRANSPORT_SMPP_SMPPROUTER_H__

#include "sms/sms.h"
#include "router/route_types.h"
#include "SmppEntity.h"

namespace scag{
namespace transport{
namespace smpp{

class SmppRouter{
public:
  virtual SmppEntity* RouteSms(router::SmeIndex srcidx,
    const smsc::sms::Address& source,
    const smsc::sms::Address& dest, router::RouteInfo& info)=0;
};

}//smpp
}//transport
}//scag


#endif
