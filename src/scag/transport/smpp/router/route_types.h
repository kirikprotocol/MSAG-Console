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

using namespace smsc::core::buffers;

typedef const char* SmeIndex;

namespace SlicingType{
    const uint8_t NONE = 0, SAR = 1, UDH8 = 2, UDH16 = 3;
}

namespace SlicingRespPolicy{
    const uint8_t ALL = 0, ANY = 1;
}

struct RouteInfo
{
  RouteInfo()
  {
    routeId.str[0]=0;
    smeSystemId.str[0]=0;
    srcSmeSystemId.str[0]=0;
    slicing = SlicingType::NONE;
    statistics = true;
    hideMessage = false;
  }

  FixedLengthString<smsc::sms::MAX_ROUTE_ID_TYPE_LENGTH> routeId;

  bool enabled;
  // bool archived;
  bool transit;
  bool statistics;
  bool hideMessage;
  uint8_t slicing, slicingRespPolicy;

  FixedLengthString<smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1> smeSystemId;
  FixedLengthString<smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1> srcSmeSystemId;

  FixedLengthString<MAX_SUBJECTID_LENGTH> srcSubj;
  FixedLengthString<MAX_SUBJECTID_LENGTH> dstSubj;

  smsc::sms::Address source;
  smsc::sms::Address dest;

  int32_t serviceId;
};

}//namespace scag
}//namespace transport
}//namespace smpp
}//namespace router

namespace scag2 {
namespace transport {
namespace smpp {
namespace router = scag::transport::smpp::router;
}
}
}

#endif
