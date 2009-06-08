#ifndef __SCAG_SMPP_SMS_SPLIT__
#define __SCAG_SMPP_SMS_SPLIT__

#include "sms/sms.h"

namespace scag { namespace transport { namespace smpp
{

using namespace smsc::sms;

/// returns (slicingType*0x10000 + slicingRefNum) or 0.
bool isSliced(SMS& sms);

uint32_t getPartsCount(SMS& orgSms);

uint32_t getNextSmsPart(SMS& orgSms, SMS& partSms, uint32_t refNum, uint32_t seq, uint8_t udh = 0, bool usePayload = false);

}}}

namespace scag2 {
namespace transport {
namespace smpp {
using scag::transport::smpp::isSliced;
using scag::transport::smpp::getPartsCount;
using scag::transport::smpp::getNextSmsPart;
}
}
}

#endif
