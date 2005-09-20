/*
        $Id$
*/

#if !defined __SCAG_Cpp_Header__router_types_h__
#define __SCAG_Cpp_Header__router_types_h__

#include <string>
#include "sms/sms.h"
#include "core/buffers/FixedLengthString.hpp"
#include "util/int.h"

namespace scag {
namespace transport {
namespace smpp {
namespace router {

static const int MAX_SUBJECTID_LENGTH=32;

namespace buf=smsc::core::buffers;

typedef char* SmeIndex;

struct RouteInfo
{
  RouteInfo()
  {
    routeId[0]=0;
    smeSystemId[0]=0;
    srcSmeSystemId[0]=0;
  }

  buf::FixedLengthString<smsc::sms::MAX_ROUTE_ID_TYPE_LENGTH> routeId;

  bool enabled;
  bool archived;

  buf::FixedLengthString<smsc::sms::MAX_SMESYSID_TYPE_LENGTH> smeSystemId;
  buf::FixedLengthString<smsc::sms::MAX_SMESYSID_TYPE_LENGTH> srcSmeSystemId;

  buf::FixedLengthString<MAX_SUBJECTID_LENGTH> srcSubj;
  buf::FixedLengthString<MAX_SUBJECTID_LENGTH> dstSubj;

  smsc::sms::Address source;
  smsc::sms::Address dest;

  int32_t providerId;
  int32_t categoryId;
  int32_t ruleId;
};

}//namespace scag
}//namespace transport
}//namespace smpp
}//namespace router

#endif
