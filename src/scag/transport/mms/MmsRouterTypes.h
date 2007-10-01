#ifndef __SCAG_TRANSPORT_MMS_ROUTER_TYPES_H__
#define __SCAG_TRANSPORT_MMS_ROUTER_TYPES_H__

#include <string>
#include "core/buffers/Hash.hpp"
#include "logger/Logger.h"

namespace scag { namespace transport { namespace mms {

using smsc::core::buffers::Hash;
using smsc::logger::Logger;
using std::string;

struct MmsRouteInfo {
  string srcId;
  string destId;
  string srcMask;
  string destMask;
  uint32_t id;
  uint32_t service_id;
  bool enabled;
  string name;

  MmsRouteInfo() : id(0), service_id(0), enabled(true) {
  }

  void print(Logger* logger) {
    smsc_log_debug(logger, "Route name=\'%s\' source=\'%s\' dest=\'%s\' srcMask=\'%s\' dstMask=\'%s\' id=%d serviceId=%d enabled=%d",
                   name.c_str(), srcId.c_str(), destId.c_str(), srcMask.c_str(),
                   destMask.c_str(), id, service_id, enabled);
  }
};

typedef Hash<MmsRouteInfo> RouteHash;

}//mms
}//transport
}//scag
#endif
