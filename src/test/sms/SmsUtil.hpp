#ifndef TEST_SMS_SMS_UTIL
#define TEST_SMS_SMS_UTIL

#include "sms/sms.h"
#include <vector>
#include <ostream>
#include <string>

namespace smsc {
namespace test {
namespace sms  {

using std::string;
using std::ostream;
using std::vector;
using smsc::sms::Address;
using smsc::sms::Descriptor;
using smsc::sms::Body;
using smsc::sms::SMS;

const int MAX_ADDRESS_LENGTH = 20;
const int MAX_SM_LENGTH = 254;
const int MAX_PAYLOAD_LENGTH = 65535;
const int MAX_SERVICE_TYPE_LENGTH = 5;
const int MAX_MSG_ID_LENGTH = 64;

typedef enum
{
	COMPARE_ALL = 0x0,
	IGNORE_STATE = 0x1,
	IGNORE_ORIGINATING_DESCRIPTOR = 0x2,
	IGNORE_DESTINATION_DESCRIPTOR = 0x4,
	IGNORE_LAST_TIME = 0x8,
	IGNORE_NEXT_TIME = 0x10,
	IGNORE_ARCHIVATION_REQUESTED = 0x20,
	IGNORE_FAILURE_CAUSE = 0x40,
	IGNORE_ATTEMPTS_COUNT = 0x80
} SmsCompareFlag;

class SmsUtil
{
public:
	//static bool compareAddresses(const Address& a1, const Address& a2);

	//static bool compareDescriptors(const Descriptor& d1, const Descriptor& d2);

	//Использую вместо vector<int> operator==(const Body&, const Body&)
	//для возможности добавления параметра compareFlag
	static vector<int> compareMessageBodies(const Body& b1, const Body& b2);

	static vector<int> compareMessages(const SMS& sms1, const SMS& sms2,
		SmsCompareFlag flag = COMPARE_ALL);

	static void setupRandomCorrectAddress(Address* addr, bool check = true);
	
	static void setupRandomCorrectAddress(Address* addr, int minLen, int maxLen, bool check = true);
	
	static void setupRandomCorrectDescriptor(Descriptor* desc, bool check = true);
	
	static void setupRandomCorrectBody(Body* body,
		uint64_t mask = 0xffffffffffffffff, bool check = true);
	
	static void setupRandomCorrectSms(SMS* sms,
		uint64_t mask = 0xffffffffffffffff, bool check = true);

	static void clearSms(SMS* sms);

	static const string configString(const Address& addr);
};

struct ltAddress
{
	bool operator() (const Address& a1, const Address& a2) const;
};

ostream& operator<< (ostream& os, const Address& a);
bool operator== (const Address& a1, const Address& a2);
bool operator!= (const Address& a1, const Address& a2);
bool operator< (const Address& a1, const Address& a2);
const string str(const Address& a);

ostream& operator<< (ostream& os, const Descriptor& d);
bool operator==(const Descriptor& d1, const Descriptor& d2);
bool operator!=(const Descriptor& d1, const Descriptor& d2);

ostream& operator<< (ostream& os, SMS& sms);
const string str(SMS& sms);

}
}
}

#endif /* TEST_SMS_SMS_UTIL */
