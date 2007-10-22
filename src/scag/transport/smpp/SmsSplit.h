#ifndef __SCAG_SMPP_SMS_SPLIT__
#define __SCAG_SMPP_SMS_SPLIT__

#include "sms/sms.h"

namespace scag { namespace transport { namespace smpp
{

using namespace smsc::sms;

bool isSliced(SMS& sms);

uint32_t getPartsCount(SMS& orgSms);

uint32_t getNextSmsPart(SMS& orgSms, SMS& partSms, uint32_t refNum, uint32_t seq, uint8_t udh = 0);

}}}

#endif
