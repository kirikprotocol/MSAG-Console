#include "TestConfig.hpp"

namespace smsc {
namespace test {
namespace conf {

TestConfig TestConfig::cfg = TestConfig();

TestConfig::TestConfig()
{
	//конфигурационный параметр core/reschedule_table
	vector<int> rescheduleTimes;
	rescheduleTimes.push_back(30);
	rescheduleTimes.push_back(20);
	intArrMap["rescheduleTimes"] = rescheduleTimes;
	//различные времена
	intMap["timeCheckAccuracy"] = 5; //5 сек
	intMap["sequentialPduInterval"] = 10; //10 сек
	intMap["maxValidPeriod"] = 150; //150 сек
	intMap["maxWaitTime"] = 60; //max(waitTime)
	intMap["maxDeliveryPeriod"] = 120; //max(validTime - waitTime)
	//адреса
	addrMap["smscAddr"] = ".3.5.123abc";
	//profiler
	addrMap["profilerAddr"] = "+38435435";
	addrMap["profilerAlias"] = ".3.3.profilerAlias123";
	strMap["profilerSystemId"] = "profiler";
	//db sme
	addrMap["dbSmeAddr"] = "+1597534682";
	addrMap["dbSmeAlias"] = ".5.5.dbSmeAlias123";
	strMap["dbSmeSystemId"] = "dbSme";
	strMap["dbSmeServiceType"] = "dBsmE";
	intMap["dbSmeProtocolId"] = 23;
	//ресурсы
	strMap["cmdRespReportNone"] = 
		"Now you will receive only requested delivery reports";
	strMap["cmdRespReportFull"] =
		"Now you will receive auxiliary delivery reports";
	strMap["cmdRespDataCodingDefault"] =
		"Now you are a default language user";
	strMap["cmdRespDataCodingUcs2"] =
		"Now you will be able to receive ucs2-encoded messages";
	strMap["cmdRespInvalidCmdText"] =
		"Invalid command";
}

int TestConfig::getIntParam(const char* name)
{
	IntMap::const_iterator it = cfg.intMap.find(name);
	__require__(it != cfg.intMap.end());
	return it->second;
}

const vector<int>& TestConfig::getIntArrParam(const char* name)
{
	IntArrMap::const_iterator it = cfg.intArrMap.find(name);
	__require__(it != cfg.intArrMap.end());
	return it->second;
}

const string& TestConfig::getStrParam(const char* name)
{
	StrMap::const_iterator it = cfg.strMap.find(name);
	__require__(it != cfg.strMap.end());
	return it->second;
}

const Address& TestConfig::getAddrParam(const char* name)
{
	AddrMap::const_iterator it = cfg.addrMap.find(name);
	__require__(it != cfg.addrMap.end());
	return it->second;
}

}
}
}

