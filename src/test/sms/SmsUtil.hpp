#ifndef TEST_SMS_SMS_UTIL
#define TEST_SMS_SMS_UTIL

#include "sms/sms.h"
#include <vector>
#include <ostream>

namespace smsc {
namespace test {
namespace sms  {

using smsc::sms::Address;
using smsc::sms::Descriptor;
using smsc::sms::Body;
using smsc::sms::SMS;
using std::vector;
using std::auto_ptr;

class SmsUtil
{
public:
	static bool compareAddresses(const Address& a1, const Address& a2);

	static bool compareDescriptors(const Descriptor& d1, const Descriptor& d2);

	static bool compareMessageBodies(const Body& b1, const Body& b2);

	static vector<int> compareMessages(const SMS& sms1, const SMS& sms2);

	static void setupRandomCorrectAddress(Address* addr);
	
	static void setupRandomCorrectDescriptor(Descriptor* desc);
	
	static void setupRandomCorrectBody(Body* body);
	
	static void setupRandomCorrectSms(SMS* sms);

	static void clearSms(SMS* sms);
};

}
}
}

#endif /* TEST_SMS_SMS_UTIL */
