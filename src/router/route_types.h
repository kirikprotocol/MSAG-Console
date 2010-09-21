/*
        $Id$
*/

#if !defined __Cpp_Header__router_types_h__
#define __Cpp_Header__router_types_h__

#include "smeman/smetypes.h"
#include "acls/interfaces.h"
#include <string>
#include "sms/sms.h"

namespace smsc {

namespace smeman{
class SmeProxy;
}

namespace router {

typedef int RoutePriority;
static const RoutePriority RoutePriorityMax = 32000;
static const RoutePriority RoutePriorityMin = 0;

typedef enum {ReplyPathPass, ReplyPathForce, ReplyPathSuppress} ReplyPath;

enum TrafficMode{
  tmNone,
  tmSmsOnly,
  tmUssdOnly,
  tmAll
};

typedef std::string RouteId;
typedef std::string BillingId;

struct RoutePoint{
  smsc::sms::Address source;
  smsc::sms::Address dest;
};

struct RouteInfo
{
  RoutePriority priority;
  TrafficMode trafMode;
  uint8_t billing;
  bool archived;
  bool suppressDeliveryReports;
  bool hide;
  bool forceDelivery;
  bool transit;
  bool allowBlocked;
  ReplyPath replyPath;
  int serviceId;
  smsc::acls::AclIdent aclId;
  RouteId routeId;
  smsc::smeman::SmeSystemId smeSystemId;
  smsc::smeman::SmeSystemId srcSmeSystemId;
  uint8_t deliveryMode;
  std::string forwardTo;
  std::string srcSubj; // for duplucate route debugging purposes only
  std::string dstSubj;
  std::string backupSme;
  int32_t providerId;
  BillingId billingId;
  int32_t categoryId;
};

struct RouteResult{
  int destSmeIdx;
  smsc::smeman::SmeProxy* destProxy;
  RouteInfo info;
  RoutePoint rp;
  bool found;
};

} // namespace router
} // namespcase smsc

#endif
