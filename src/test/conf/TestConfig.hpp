#ifndef TEST_CONF_TEST_CONFIG
#define TEST_CONF_TEST_CONFIG

#include "sms/sms.h"
#include <map>
#include <string>
#include <vector>

#define __cfg_int__(param) \
	static const int param = TestConfig::getIntParam(#param)
	
#define __cfg_int_arr__(param) \
	static const vector<int>& param = TestConfig::getIntArrParam(#param)

#define __cfg_str__(param) \
	static const string& param = TestConfig::getStrParam(#param)

#define __cfg_addr__(param) \
	static const Address& param = TestConfig::getAddrParam(#param)

namespace smsc {
namespace test {
namespace conf {

using std::map;
using std::string;
using std::vector;
using smsc::sms::Address;

class TestConfig
{
	typedef map<string, int> IntMap;
	typedef map<string, vector<int> > IntArrMap;
	typedef map<string, string> StrMap;
	typedef map<string, Address> AddrMap;

	IntMap intMap;
	IntArrMap intArrMap;
	StrMap strMap;
	AddrMap addrMap;
	static TestConfig cfg;

public:
	TestConfig();
	static int getIntParam(const char* name);
	static const vector<int>& getIntArrParam(const char* name);
	static const string& getStrParam(const char* name);
	static const Address& getAddrParam(const char* name);
};

}
}
}

#endif /* TEST_CONF_TEST_CONFIG */

