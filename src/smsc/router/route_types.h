/*
        $Id$
*/

#if !defined __Cpp_Header__router_types_h__
#define __Cpp_Header__router_types_h__

#include <string>

#include "smsc/smeman/smetypes.h"
#include "smsc/acls/AclManager.hpp"
#include "sms/sms.h"

namespace smsc {
namespace router {

typedef int RoutePriority;
static const RoutePriority RoutePriorityMax = 32000;
static const RoutePriority RoutePriorityMin = 0;

typedef enum {ReplyPathPass, ReplyPathForce, ReplyPathSuppress} ReplyPath;

typedef std::string RouteId;

enum TrafficMode{
  tmNone,
  tmSmsOnly,
  tmUssdOnly,
  tmAll
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
  ReplyPath replyPath;
  int serviceId;
  smsc::acl::AclIdent aclId;
  RouteId routeId;
  smsc::smeman::SmeSystemId destSmeSystemId;
  smsc::smeman::SmeIndex destSmeIndex;
  smsc::smeman::SmeSystemId srcSmeSystemId;
  uint8_t deliveryMode;
  std::string forwardTo;
  std::string backupSme;
  bool allowBlocked;
  int32_t providerId;
  int32_t categoryId;
};

} // namespace router
} // namespcase smsc

#endif
