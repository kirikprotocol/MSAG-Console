#ifndef TEST_STORE_SM_UTIL
#define TEST_STORE_SM_UTIL

#include "sms/sms.h"

namespace smsc  {
namespace test  {
namespace store {

class SMUtil
{
public:
	bool compareAddresses(smsc::sms::Address& a1, smsc::sms::Address& a2);
	bool compareMessageBody(smsc::sms::Body& b1, smsc::sms::Body& b2);
};

}
}
}

#endif /* TEST_STORE_SM_UTIL */
